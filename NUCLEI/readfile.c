#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "readfile.h"
#include "lisp.h"
#include "specific.h"
#include "tokens_table.h"

#define STRLEN 100


void read_file(char* fname_ncl, Program* p) {
    FILE* fp = nfopen(fname_ncl, "r");
    char line[STRLEN] = {'\0'};
    int word_cnt = 0;
    while (fgets(line, STRLEN, fp)) {
        char line_words[MAXNUMTOKENS][MAXTOKENSIZE] = {{'\0'}};
        int len = strlen(line);
        if (line[len-1] == NL_CH || line[len-1] == RE_CH) {
            line[len-1] = '\0';
        }
        if (len-2>=0 && (line[len-2] == NL_CH || line[len-2] == RE_CH)) {
            line[len-2] = '\0';
        }
        if (line[0] != COMM_CH) {
            process_line(line, line_words);
            word_cnt = append(p->wds, line_words, word_cnt);
        }
    }
    fclose(fp);
}

void find_string(char* str, char words[MAXNUMTOKENS][MAXTOKENSIZE]) {
    int i=0;
    int len = strlen(str);
    int token_cnt = 0;
    while (i < len) {
        char tmp[2] = {'\0'};
        if (str[i] != D_Q_CH) {
            tmp[0] = str[i];
            strcat(words[token_cnt], tmp);
            i++;
        // found <STRING>, store the whole string in a new cell in words
        } else {
            token_cnt++;
            strcat(words[token_cnt], D_Q_STR);
            i++;
            while (i < len && str[i] != D_Q_CH) {
                tmp[0] = str[i];
                strcat(words[token_cnt], tmp);
                i++;
            }
            if (i < len) {
                strcat(words[token_cnt], D_Q_STR);
            }
            token_cnt++;
            i++;
        }
    }
}

void split_string(char* str, char words[MAXNUMTOKENS][MAXTOKENSIZE]) {
    int i = 0; 
    int word_cnt = -1;
    int len = strlen(str);
    while (i < len) {
        // store " in a separate cell in words
        if (str[i] == D_Q_CH && word_cnt >= -1) {
            word_cnt++;
            strcpy(words[word_cnt], D_Q_STR);
            word_cnt++;
        // store the content of string in a cell in words
        } else {
            char tmp[2] = {str[i], '\0'};
            strcat(words[word_cnt], tmp);
        }
        i++;
    }
}

void find_literal(char* str, char words[MAXNUMTOKENS][MAXTOKENSIZE]) {
    int i=0;
    int len = strlen(str);
    int token_cnt = 0;
    while (i < len) {
        char tmp[2] = {'\0'};
        if (str[i] != S_Q_CH) {
            tmp[0] = str[i]; tmp[1] = '\0';
            strcat(words[token_cnt], tmp);
            i++;
        // found <LITERAL>, store the whole literal in a new cell in words
        } else {
            token_cnt++;
            tmp[0] = str[i]; tmp[1] = '\0';
            strcat(words[token_cnt], tmp);
            i++;
            while (i < len && str[i] != S_Q_CH) {
                tmp[0] = str[i]; tmp[1] = '\0';
                strcat(words[token_cnt], tmp);
                i++;
            }
            if (i < len) {
                tmp[0] = str[i]; tmp[1] = '\0';
                strcat(words[token_cnt], tmp);
            }
            token_cnt++;
            i++;
        }
    }
}

void split_literal(char* str, char words[MAXNUMTOKENS][MAXTOKENSIZE]) {
    int i = 0; 
    int word_cnt = -1;
    int len = strlen(str);
    while (i < len) {
        // store ' in a separate cell in words
        if (str[i] == S_Q_CH && word_cnt >= -1) {
            word_cnt++;
            strcpy(words[word_cnt], S_Q_STR);
            word_cnt++;
        // store the content of literal in a cell in words
        } else {
            char tmp[2] = {str[i], '\0'};
            strcat(words[word_cnt], tmp);
        }
        i++;
    }
}

void process_str_lit(char* str, char words[MAXNUMTOKENS][MAXTOKENSIZE]) {
    char tmp1[MAXNUMTOKENS][MAXTOKENSIZE] = {{'\0'}};
    find_string(str, tmp1);
    int t1 = 0;
    int word_cnt = 0;
    while (tmp1[t1][0] != '\0') {
        char tmp2[MAXNUMTOKENS][MAXTOKENSIZE] = {{'\0'}};
        if (tmp1[t1][0] != D_Q_CH) {
            find_literal(tmp1[t1], tmp2);
            word_cnt = append(words, tmp2, word_cnt);
        } else {
            strcpy(words[word_cnt], tmp1[t1]);
            word_cnt++;
        }
        t1++;
    }
}

void split_space(char* str, char words[MAXNUMTOKENS][MAXTOKENSIZE]) {
    int i=0;
    int len = strlen(str);
    char word[MAXTOKENSIZE] = {'\0'};
    int word_cnt = 0;
    bool is_space = true;
    while (i < len) {
        if (str[i] != SPACE_CH) {
            char tmp[2] = {str[i], '\0'};
            strcat(word, tmp);
            is_space = false;
        } else { // word finished, add word to words
            if (!is_space) { // in case of mutiple spaces 
                strcpy(words[word_cnt], word);
                word[0] = '\0';
                word_cnt++;
            }
            is_space = true;
        }
        i++;
    }
    strcpy(words[word_cnt], word);
}

void process_space(char* str, char words[MAXNUMTOKENS][MAXTOKENSIZE]) {
    char tmp[MAXNUMTOKENS][MAXTOKENSIZE] = {{'\0'}};
    int word_cnt = 0;
    process_str_lit(str, tmp);
    for (int i=0; tmp[i][0]; i++) {
        char tmp2[MAXNUMTOKENS][MAXTOKENSIZE] = {{'\0'}};
        if (tmp[i][0] != D_Q_CH && tmp[i][0] != S_Q_CH) {
            split_space(tmp[i], tmp2);
            word_cnt = append(words, tmp2, word_cnt);
        } else {
            strcpy(words[word_cnt], tmp[i]);
            word_cnt++;
        }
    }
}

void split_paren(char* str, char words[MAXNUMTOKENS][MAXTOKENSIZE]) {
    int i = 0;
    int len = strlen(str);
    int word_cnt = 0;
    bool is_paren = true;
    while (i < len) {
        if (str[i] == LP_CH || str[i] == RP_CH) {
            if (!is_paren) {
                word_cnt++;
                is_paren= true;
            }
            char tmp[2] = {str[i], '\0'};
            strcpy(words[word_cnt], tmp);
            word_cnt++;
        } else {
            char tmp[2] = {str[i], '\0'};
            strcat(words[word_cnt], tmp);
            is_paren = false;
        }
        i++;
    } 
}

void process_line(char* str, char words[MAXNUMTOKENS][MAXTOKENSIZE]) {
    char tmp[MAXNUMTOKENS][MAXTOKENSIZE] = {{'\0'}};
    int word_cnt = 0;
    process_space(str, tmp);
    for (int i=0; tmp[i][0]; i++) {
        char tmp2[MAXNUMTOKENS][MAXTOKENSIZE] = {{'\0'}};
        if (tmp[i][0] == D_Q_CH) {
            split_string(tmp[i], tmp2);
            word_cnt = append(words, tmp2, word_cnt);
        } else if (tmp[i][0] == S_Q_CH) {
            split_literal(tmp[i], tmp2);
            word_cnt = append(words, tmp2, word_cnt);
        } else {
            split_paren(tmp[i], tmp2);
            word_cnt = append(words, tmp2, word_cnt);
        }
    }
}



int append(char dest[MAXNUMTOKENS][MAXTOKENSIZE], char sour[MAXNUMTOKENS][MAXTOKENSIZE], int start) {
    int i = 0;
    while (sour[i][0] != '\0') {
        strcpy(dest[start], sour[i]);
        start++;
        i++;
    }
    return start;
}


void read_file_test(void) {
    printf("\nRunning read file test: ... \n");
    
    char str1[STRLEN] = "((PRINT \"  HELLO WORLD \") (PRINT \"  test case \"))";
    char words1[MAXNUMTOKENS][MAXTOKENSIZE] = {{'\0'}};
    find_string(str1, words1);
    assert(strcmp(words1[0], "((PRINT ") == 0);
    assert(strcmp(words1[1], "\"  HELLO WORLD \"") == 0);
    assert(strcmp(words1[2], ") (PRINT ") == 0);
    assert(strcmp(words1[3], "\"  test case \"") == 0);
    assert(strcmp(words1[4], "))") == 0);
    assert(words1[5][0] == '\0');

    char str2[STRLEN] = "\"hello world \"";
    char words2[MAXNUMTOKENS][MAXTOKENSIZE] = {{'\0'}};
    split_string(str2, words2);
    assert(strcmp(words2[0], "\"") == 0);
    assert(strcmp(words2[1], "hello world ") == 0);
    assert(strcmp(words2[2], "\"") == 0);
    assert(words2[3][0] == '\0');

    char str3[STRLEN] = "(SET A ' (1 2 3 (4 5))    ')";
    char words3[MAXNUMTOKENS][MAXTOKENSIZE] = {{'\0'}};
    find_literal(str3, words3);
    assert(strcmp(words3[0], "(SET A ") == 0);
    assert(strcmp(words3[1], "' (1 2 3 (4 5))    '") == 0);
    assert(strcmp(words3[2], ")") == 0);
    assert(words3[3][0] == '\0');

    char str4[STRLEN] = "' (1 2 3 (4 5))    '";
    char words4[MAXNUMTOKENS][MAXTOKENSIZE] = {{'\0'}};
    split_literal(str4, words4);
    assert(strcmp(words4[0], "'") == 0);
    assert(strcmp(words4[1], " (1 2 3 (4 5))    ") == 0);
    assert(strcmp(words4[2], "'") == 0);
    assert(words4[3][0] == '\0');

    char str5[STRLEN] = "((PRINT \"Hello World\") (SET A '(1 2 (3 4))') (PRINT \"Finished\"))";
    char words5[MAXNUMTOKENS][MAXTOKENSIZE] = {{'\0'}};
    process_str_lit(str5, words5);
    assert(strcmp(words5[0], "((PRINT ") == 0);
    assert(strcmp(words5[1], "\"Hello World\"") == 0);
    assert(strcmp(words5[2], ") (SET A ") == 0);
    assert(strcmp(words5[3], "'(1 2 (3 4))'") == 0);
    assert(strcmp(words5[4], ") (PRINT ") == 0);
    assert(strcmp(words5[5], "\"Finished\"") == 0);
    assert(strcmp(words5[6], "))") == 0);
    assert(words5[7][0] == '\0');

    char str6[STRLEN] = "   ((SET  A B) (PRINT  A) ) ";
    char words6[MAXNUMTOKENS][MAXTOKENSIZE] = {{'\0'}};
    split_space(str6, words6);
    assert(strcmp(words6[0], "((SET") == 0);
    assert(strcmp(words6[1], "A") == 0);
    assert(strcmp(words6[2], "B)") == 0);
    assert(strcmp(words6[3], "(PRINT") == 0);
    assert(strcmp(words6[4], "A)") == 0);
    assert(strcmp(words6[5], ")") == 0);
    assert(words6[6][0] == '\0');

    char str7[STRLEN] = "((PRINT \"Hello World \")  (SET A '(1 2 (3 4))') (PRINT \"Finished\"))";
    char words7[MAXNUMTOKENS][MAXTOKENSIZE] = {{'\0'}};
    process_space(str7, words7);
    assert(strcmp(words7[0], "((PRINT") == 0);
    assert(strcmp(words7[1], "\"Hello World \"") == 0);
    assert(strcmp(words7[2], ")") == 0);
    assert(strcmp(words7[3], "(SET") == 0);
    assert(strcmp(words7[4], "A") == 0);
    assert(strcmp(words7[5], "'(1 2 (3 4))'") == 0);
    assert(strcmp(words7[6], ")") == 0);
    assert(strcmp(words7[7], "(PRINT") == 0);
    assert(strcmp(words7[8], "\"Finished\"") == 0);
    assert(strcmp(words7[9], "))") == 0);
    assert(words7[10][0] == '\0');

    char str8[STRLEN] = "((PRINT";
    char words8[MAXNUMTOKENS][MAXTOKENSIZE] = {{'\0'}};
    split_paren(str8, words8);
    assert(strcmp(words8[0], "(") == 0);
    assert(strcmp(words8[1], "(") == 0);
    assert(strcmp(words8[2], "PRINT") == 0);
    assert(words8[3][0] == '\0');

    char str9[STRLEN] = "(PRINT)";
    char words9[MAXNUMTOKENS][MAXTOKENSIZE] = {{'\0'}};
    split_paren(str9, words9);
    assert(strcmp(words9[0], "(") == 0);
    assert(strcmp(words9[1], "PRINT") == 0);
    assert(strcmp(words9[2], ")") == 0);
    assert(words9[3][0] == '\0');

    char str10[STRLEN] = "((PRINT \"Hello World \")  (SET A ' (1 2 (3 4))') (PRINT \"Finished\"))";
    char words10[MAXNUMTOKENS][MAXTOKENSIZE] = {{'\0'}};
    process_line(str10, words10);
    assert(strcmp(words10[0], "(") == 0);
    assert(strcmp(words10[3], "\"") == 0);
    assert(strcmp(words10[4], "Hello World ") == 0);
    assert(strcmp(words10[5], "\"") == 0);
    assert(strcmp(words10[10], "'") == 0);
    assert(strcmp(words10[11], " (1 2 (3 4))") == 0);
    assert(strcmp(words10[12], "'") == 0);

    char str11[STRLEN] = "((SET A) (PRINT A))";
    char words11[MAXNUMTOKENS][MAXTOKENSIZE] = {{'\0'}};
    process_line(str11, words11);
    assert(strcmp(words11[0], "(") == 0);
    assert(strcmp(words11[6], "PRINT") == 0);
    assert(words11[10][0] == '\0');

    printf("Read file test ended ...\n\n");
}
