#include "nuclei.h"

void read_file(char* fname_ncl, Program* p);

// separate <STRING> from string
void find_string(char* str, char words[MAXNUMTOKENS][MAXTOKENSIZE]);
// separate <LITERAL> from string
void find_literal(char* str, char words[MAXNUMTOKENS][MAXTOKENSIZE]);

// separate <STRING> and <LITERAL> from string
void process_str_lit(char* str, char words[MAXNUMTOKENS][MAXTOKENSIZE]);
// split string by space and del redundant space (allow <STRING> or <LITERAL> in string)
void process_space(char* str, char words[MAXNUMTOKENS][MAXTOKENSIZE]);
// split a line into a list of tokens: PAREN / KEYWORD / STRING / LITERAL
void process_line(char* str, char words[MAXNUMTOKENS][MAXTOKENSIZE]);

// split string by space and del redundant space (No <STRING> or <LITERAL> in string)
void split_space(char* str, char words[MAXNUMTOKENS][MAXTOKENSIZE]);
// split <STRING> to three parts: " + content + "
void split_string(char* str, char words[MAXNUMTOKENS][MAXTOKENSIZE]);
// split <LITERAL> to three parts: ' + content + '
void split_literal(char* str, char words[MAXNUMTOKENS][MAXTOKENSIZE]);
// separate every paren in string (No <STRING> or <LITERAL> in string)
void split_paren(char* str, char words[MAXNUMTOKENS][MAXTOKENSIZE]);

// append a list of tokens at the end of another tokens
int append(char dest[MAXNUMTOKENS][MAXTOKENSIZE], char sour[MAXNUMTOKENS][MAXTOKENSIZE], int start);
