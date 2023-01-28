#include "lisp.h"
#include "specific.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>

#define STRLEN 10
#define ASC '0'
#define TOKEN_NUM 100
#define TOKEN_LEN 50

#define L_PAREN '('
#define R_PAREN ')'
#define SPACE ' '
#define MINUS '-'

typedef struct Node {
    lisp* cons;
    struct Node* next;
} Node;

typedef struct Stack {
    Node* top;
    int size;
} Stack;

/* --- Stack related functions --- */

Node* _make_node(lisp* cons);
Stack* _make_stack(void);
void _push(Stack* s, Node* n);
Node* _pop(Stack* s);
void _free_stack(Stack* s);

/* ---helper functions for lisp_tostring and lisp_fromstring --- */

void _to_string(const lisp* l, char* str);
lisp* _from_string(char tokens[TOKEN_NUM][TOKEN_LEN], Stack* s, int* i);
void _incre(int* i);

/* --- functions for parseing strings --- */

// divide every element in str in to an arr of str
// eg: "(1 23 (-4 5))" --> {"(", "1", "23", "(", "-4", "5", ")", ")"}
void _parse(char* str, char tokens[TOKEN_NUM][TOKEN_LEN]);

// delete the spaces at the start and end of str
void _del_space_ends(char* str, char* p_str);

// split a str by space
// eg: "(1 23 (-4 5))" --> {"(1", "23", "(-4", "5))"}
void _split(char* str, char words[TOKEN_NUM][TOKEN_LEN]);

// split str into digits and paren
// eg: "(-4" --> {"(", "-4"}
void _parse_word(char* word, char tokens[TOKEN_NUM][TOKEN_LEN]);

// return the number of tokens in an array
// eg: len of {"(", "1", "23", "(", "-4", "5", ")", ")"} is 8
int _token_len(char tokens[TOKEN_NUM][TOKEN_LEN]);

// return true if a str can be converted to number
bool _is_digit(char* str);
// convert a str into a number
int _get_digit(char* str);


lisp* lisp_atom(const atomtype a) {
    lisp* l = (lisp*)ncalloc(1, sizeof(lisp));
    l->car = NULL;
    l->cdr = NULL;
    l->val = a;
    return l;
}

lisp* lisp_cons(const lisp* l1,  const lisp* l2) {
    lisp* l = (lisp*)ncalloc(1, sizeof(lisp));
    l->car = (lisp*)l1;
    l->cdr = (lisp*)l2;
    return l;
}

lisp* lisp_car(const lisp* l) {
    return (lisp*)l->car;
}

lisp* lisp_cdr(const lisp* l) {
    return (lisp*)l->cdr;
}

atomtype lisp_getval(const lisp* l) {
    return (atomtype)l->val;
}

bool lisp_isatomic(const lisp* l) {
    if (l == NULL) {
        return false;
    }
    if (l->car == NULL && l->cdr == NULL) {
        return true;
    }
    return false;
}

lisp* lisp_copy(const lisp* l) {
    if (l == NULL) {
        return NULL;
    }
    lisp* nl = (lisp*)ncalloc(1, sizeof(lisp)); // create head of copied lisp
    lisp* current = (lisp*)l;
    lisp* cur_dest = nl;
    while (current != NULL) {
        if (lisp_isatomic(current->car)) {
            lisp* car = (lisp*)ncalloc(1, sizeof(lisp));
            car->val = current->car->val;
            cur_dest->car = car;
        } else {  // recursively copy sub lisp
            cur_dest->car = lisp_copy(current->car);
        }
        if (current->cdr != NULL) { // assign space for next cons
            lisp* cdr = (lisp*)ncalloc(1, sizeof(lisp));
            cur_dest->cdr = cdr;
        }
        current = current->cdr;
        cur_dest = cur_dest->cdr;
    }
    return nl;
}

int lisp_length(const lisp* l) {
    if (l == NULL) {
        return 0;
    }
    if (lisp_isatomic(l)) {
        return 0;
    }
    const lisp* current = l;
    int len = 0;
    while (current != NULL) {
        len++;
        current = current->cdr;
    }
    return len;
}

void lisp_tostring(const lisp* l, char* str) {
    if (l == NULL) {
        strcpy(str, "()");
        return;
    }
    if (lisp_isatomic(l)) {
        char content[STRLEN];
        snprintf(content, STRLEN, "%i", l->val);
        strcpy(str, content);
        return;
    }
    str[0] = '\0';
    _to_string(l, str);
    int len = strlen(str);
    if (len > 2) {
        str[len-1] = '\0';
    }
}

void lisp_free(lisp** l) {
    if (*l == NULL) {
        return;
    }
    if ((*l)->car != NULL) {
        lisp_free(&((*l)->car));
        (*l)->car = NULL;
    }
    if ((*l)->cdr != NULL) {
        lisp_free(&((*l)->cdr));
        (*l)->cdr = NULL;
    }
    free(*l);
    *l = NULL;
}


lisp* lisp_fromstring(const char* str) {
    if (strcmp(str, "()") == 0) {
        return NULL;
    }
    char tokens[TOKEN_NUM][TOKEN_LEN] = {{'\0'}};
    _parse((char*)str, tokens);
    if (_token_len(tokens) == 1) {
        int n = _get_digit(tokens[0]);
        return lisp_atom(n);
    }
    Stack* s = _make_stack();
    int i=0;  // keep track of index when iterate through str
    lisp* result = _from_string(tokens, s, &i);
    return result;
}


lisp* lisp_list(const int n, ...) {
    lisp* list[TOKEN_NUM] = {NULL};
    va_list argp;
    va_start(argp, n);
    // add every lisp into list
    for (int i=0; i<n; i++) {
        lisp* cons = va_arg(argp, lisp*);
        list[i] = cons;
    }
    // iterate through list backwards and use lisp_cons to build lisp
    list[n-1] = lisp_cons(list[n-1], NULL);
    for (int i=n-1; i>0; i--) {
        list[i-1] = lisp_cons(list[i-1], list[i]);
    }
    return list[0];
}

void lisp_reduce(void (*func)(lisp* l, atomtype* n), lisp* l, atomtype* acc) {
    lisp* current = l;
    while (current != NULL) {
        if (lisp_isatomic(lisp_car(current))) {
            func(lisp_car(current), acc);
        } else {
            lisp_reduce(func, current->car, acc);
        }
        current = current->cdr;
    }
}


/* ===== Self-defined Functions ===== */

void _to_string(const lisp* l, char* str) {
    strcat(str, "(");
    lisp* current = (lisp*)l;
    while (current != NULL) {
        if (lisp_isatomic(current->car)) {
            char content[STRLEN];
            snprintf(content, STRLEN, "%i ", current->car->val);
            strcat(str, content);
        } else {
            _to_string(current->car, str);
        }
        current = current->cdr;
    }
    str[strlen(str)-1] = R_PAREN;
    strcat(str, " ");
}

lisp* _from_string(char tokens[TOKEN_NUM][TOKEN_LEN], Stack* s, int* i) {
    lisp* result = NULL;
    int len = _token_len(tokens);
    while (*i < len) {
        // if it's number, push it into the stack
        if (_is_digit(tokens[*i])) {
            Node* n = _make_node(lisp_cons(lisp_atom(_get_digit(tokens[*i])), NULL));
            _push(s, n);
            _incre(i);
        // if it's ")", pop from stack and use lisp_cons() to build lisp
        } else if (strcmp(tokens[*i], ")") == 0) {
            Node* n = _pop(s);
            lisp* cur_cons = n->cons;
            free(n);
            while (s->size > 0) {
                Node* n  = _pop(s);
                cur_cons = lisp_cons(n->cons->car, cur_cons);
                free(n->cons);
                free(n);
            }
            _incre(i);
            _free_stack(s);
            return cur_cons;
        // if it's "(", recursively build sub lisp  
        } else if (strcmp(tokens[*i], "(") == 0){
            lisp* cons = (lisp*)ncalloc(1, sizeof(lisp));
            Stack* sub_s = _make_stack();
            _incre(i);
            cons->car = _from_string(tokens, sub_s, i);
            result = cons->car;
            Node* n = _make_node(cons);
            _push(s, n);
        } 
    }
    _free_stack(s);
    return result;
}

void _incre(int* i) {
    *i = *i + 1;
    i = &(*i);
}

void _parse(char* str, char tokens[TOKEN_NUM][TOKEN_LEN]) {
    char words[TOKEN_NUM][TOKEN_LEN] = {{'\0'}};
    int token_cnt = 0;
    char p_str[TOKEN_NUM] = {'\0'};
    _del_space_ends(str, p_str);  // delete spaces at front and end of str
    _split(p_str, words);         // split str by space
    int i = 0;
    while (words[i][0] != '\0') {
        char sub_tokens[TOKEN_NUM][TOKEN_LEN] = {{'\0'}};
        _parse_word(words[i], sub_tokens);
        for (int j=0; sub_tokens[j][0] != '\0'; j++) {
            strcpy(tokens[token_cnt], sub_tokens[j]);
            token_cnt++;
        }
        i++;
    }
}

void _del_space_ends(char* str, char* p_str) {
    int len = strlen(str);
    int p = 0;
    int i = 0, j = len - 1;
    while (str[i] == SPACE) { i++;}
    while (str[j] == SPACE) { j--;}
    for (int k=i; k<j+1; k++) {
        p_str[p] = str[k];
        p++;
    }
}

void _split(char* str, char words[TOKEN_NUM][TOKEN_LEN]) {
    int i=0;
    int len = strlen(str);
    char word[TOKEN_LEN] = {'\0'};
    int word_cnt = 0;
    bool is_space = false;
    while (i < len) {
        if (str[i] != SPACE) {
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

void _parse_word(char* word, char tokens[TOKEN_NUM][TOKEN_LEN]) {
    int i = 0;
    int len = strlen(word);
    int token_cnt = 0;
    while (i < len) {
        if (word[i] == L_PAREN) {
            strcpy(tokens[token_cnt], "(");
            token_cnt++;
        } else if (word[i] == R_PAREN) {
            strcpy(tokens[token_cnt], ")");
            token_cnt++;
        } else { // found a number
            char tmp[2] = {word[i], '\0'};
            strcat(tokens[token_cnt], tmp);
            if (i+1<len && (word[i+1] == L_PAREN || word[i+1] == R_PAREN)) {
                token_cnt++;
            } else if (i+1>=len) {
                token_cnt++;
            }
        }
        i++;
    }
}

int _token_len(char tokens[TOKEN_NUM][TOKEN_LEN]) {
    int i = 0;
    while (tokens[i][0] != '\0') {
        i++;
    }
    return i;
}

bool _is_digit(char* str) {
    int n = 0;
    return sscanf(str, "%i", &n);
}

int _get_digit(char* str) {
    int n = 0;
    assert(sscanf(str, "%i", &n));
    return n;
}


Node* _make_node(lisp* cons) {
    Node* n = (Node*)ncalloc(1, sizeof(Node));
    n->cons = cons;
    return n;
}

Stack* _make_stack() {
    Stack* s = (Stack*)ncalloc(1, sizeof(Stack));
    return s;
}

void _push(Stack* s, Node* n) {
    if (s == NULL) {
        Stack* s = _make_stack();
        s->top = n;
        s->size = 1;
        return;
    }
    if (s->top == NULL) {
        s->top = n;
        s->size++;
        return;
    }
    n->next = s->top;
    s->top = n;
    s->size++;
}

Node* _pop(Stack* s) {
    if (s == NULL) {
        return NULL;
    }
    Node* top = s->top;
    if (top != NULL) {
        s->top = top->next;
        s->size--;        
    }
    return top;
}

void _free_stack(Stack* s) {
    if (s == NULL) {
        return;
    }
    Node* current = NULL;
    while (s->size != 0) {
        current = _pop(s);
        free(current->cons);
        free(current);
    }
    free(s);
}


// void test(void) {
//     int i = 0;
//     _incre(&i);
//     assert(i == 1);
//     _incre(&i);
//     assert(i == 2);

//     Stack* s = _make_stack();
//     lisp* cons1 = lisp_cons(lisp_atom(2), NULL);
//     lisp* cons2 = lisp_cons(lisp_atom(3), NULL);
//     lisp* cons3 = lisp_cons(lisp_atom(4), NULL);
//     Node* node1 = _make_node(cons1);
//     Node* node2 = _make_node(cons2);
//     Node* node3 = _make_node(cons3);
//     _push(s, node1);
//     _push(s, node2);
//     _push(s, node3);
//     assert(lisp_getval(lisp_car(_pop(s)->cons)) == 4);
//     assert(lisp_getval(lisp_car(_pop(s)->cons)) == 3);
//     assert(lisp_getval(lisp_car(_pop(s)->cons)) == 2);

//     lisp_free(&cons1); lisp_free(&cons2); lisp_free(&cons3);
//     free(node1); free(node2); free(node3);
//     _free_stack(s);

//     char p_str[TOKEN_NUM] = {'\0'};
//     _del_space_ends("   ((-1 2) (3 4) (5 (6 7))) ", p_str);
//     assert(strcmp("((-1 2) (3 4) (5 (6 7)))", p_str) == 0);

//     char words[TOKEN_NUM][TOKEN_LEN] = {{'\0'}};
//     _split("((-1 2) (3 4) (5 (6 7)))", words);
//     assert(strcmp(words[0], "((-1") == 0);
//     assert(strcmp(words[1], "2)") == 0);
//     assert(strcmp(words[2], "(3") == 0);
//     assert(strcmp(words[3], "4)") == 0);
//     assert(strcmp(words[4], "(5") == 0);
//     assert(strcmp(words[5], "(6") == 0);
//     assert(strcmp(words[6], "7)))") == 0);
//     char sub_tokens[TOKEN_NUM][TOKEN_LEN] = {{'\0'}};
//     _parse_word(words[0], sub_tokens);
//     assert(strcmp(sub_tokens[0], "(") == 0);
//     assert(strcmp(sub_tokens[1], "(") == 0);
//     assert(strcmp(sub_tokens[2], "-1") == 0);
    
//     char sub_tokens2[TOKEN_NUM][TOKEN_LEN] = {{'\0'}};
//     _parse_word(words[1], sub_tokens2);
//     assert(strcmp(sub_tokens2[0], "2") == 0);
//     assert(strcmp(sub_tokens2[1], ")") == 0);
//     assert(sub_tokens2[2][0] == '\0');

//     char sub_tokens3[TOKEN_NUM][TOKEN_LEN] = {{'\0'}};
//     _parse_word(words[6], sub_tokens3);
//     assert(strcmp(sub_tokens3[0], "7") == 0);
//     assert(strcmp(sub_tokens3[1], ")") == 0);
//     assert(strcmp(sub_tokens3[2], ")") == 0);
//     assert(strcmp(sub_tokens3[3], ")") == 0);
//     assert(sub_tokens3[4][0] == '\0');

//     char tokens[TOKEN_NUM][TOKEN_LEN] = {{'\0'}};
//     _parse("((-1 2) (3 4) (5 (6 7)))", tokens);
//     assert(_token_len(tokens) == 17);
//     assert(strcmp(tokens[0], "(") == 0);
//     assert(strcmp(tokens[1], "(") == 0);
//     assert(strcmp(tokens[2], "-1") == 0);
//     assert(strcmp(tokens[3], "2") == 0);
//     assert(strcmp(tokens[4], ")") == 0);
//     assert(strcmp(tokens[5], "(") == 0);
//     assert(strcmp(tokens[6], "3") == 0);
//     assert(strcmp(tokens[7], "4") == 0);
//     assert(strcmp(tokens[8], ")") == 0);
//     assert(strcmp(tokens[9], "(") == 0);
//     assert(strcmp(tokens[10], "5") == 0);
//     assert(strcmp(tokens[11], "(") == 0);
//     assert(strcmp(tokens[12], "6") == 0);
//     assert(strcmp(tokens[13], "7") == 0);
//     assert(strcmp(tokens[14], ")") == 0);
//     assert(strcmp(tokens[15], ")") == 0);
//     assert(strcmp(tokens[16], ")") == 0);
//     assert(tokens[17][0] == '\0');

//     char tokens2[TOKEN_NUM][TOKEN_LEN] = {{'\0'}};
//     _parse("(1(2(-3(4 5))))", tokens2);
//     assert(_token_len(tokens2) == 13);
//     assert(strcmp(tokens2[0], "(") == 0);
//     assert(strcmp(tokens2[1], "1") == 0);
//     assert(strcmp(tokens2[2], "(") == 0);
//     assert(strcmp(tokens2[3], "2") == 0);
//     assert(strcmp(tokens2[4], "(") == 0);
//     assert(strcmp(tokens2[5], "-3") == 0);

//     char str[1000] = {'\0'};
//     char ans[6][1000] = {"()", "(1)", "(0 (1 -2) 3 4 50)", 
//                          "((-1 2) (3 4) (5 (6 7)))", "(-1 2 -133 4 5)", "(1 (-2) (3) (4 (5 6 7) -8 9) 10)"};
//     char inp[6][1000] = {"()", "( 1)", "(  0 (1 -2)3 4 50)", 
//                          "((-1 2)(  3 4) (5(6 7 ) ) )  ", " ( -1 2 -133 4 5)", "(1(-2)(3)(4(5 6 7)-8 9) 10)"};
//     for(int i=0; i<6; i++){
//         lisp* f1 = lisp_fromstring(inp[i]);
//         lisp_tostring(f1, str);
//         assert(strcmp(str, ans[i])==0);
//         lisp_free(&f1);
//         assert(!f1);
//    }

//     assert(_is_digit("1234"));
//     assert(_is_digit("-1234"));
//     assert(!_is_digit("("));
//     assert(_get_digit("-1") == -1);
//     assert(_get_digit("90") == 90);
//     assert(_get_digit("12") == 12);
// }
