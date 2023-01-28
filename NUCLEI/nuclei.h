#include "lisp.h"

#define MAXNUMTOKENS 500
#define MAXTOKENSIZE 100
#define ALPHANUM 26
#define POOLSIZE 5000

// struct for every variable set in NUCLEI
typedef struct Variable {
    char name;
    lisp* val;
} Variable;

// struct to store every variable in a list
typedef struct VarList {
    Variable* list[ALPHANUM];
    int num;
} VarList;

// struct to store every lisp generated in interp
typedef struct Pool {
    lisp* p[POOLSIZE];
    int size;
} Pool;

typedef struct Program {
    char wds[MAXNUMTOKENS][MAXTOKENSIZE];
    int cw;
    VarList* vl;
    Pool* pool;
} Program;

// ===== Main functions used during parse and interp =====

VarList* Prog(Program* p);
void Instrcts(Program* p);
void Instrct(Program* p);
void Func(Program* p);
lisp* Listfunc(Program* p);
lisp* Intfunc(Program* p);
lisp* Boolfunc(Program* p);
void Iofunc(Program* p);
void Set(Program* p);
void Print(Program* p);
lisp* List(Program* p);
char Var(Program* p);
char* String(Program* p);
lisp* Literal(Program* p);
lisp* Retfunc(Program* p);
void If(Program* p);
void Loop(Program* p);


// ===== Sub functions used during parse =====

// check if cw is target, if not, exit the program
void Parse_check(Program* p, char* target, int incre);
// check for double list <LIST> <LIST> grammar
void Parse_List_List(Program* p);


// ===== Sub functions used during interp =====
#ifdef INTERP
// *** IO ***
// associate variable name with a lisp, and store this variable in variableList
void Inter_set(Program* p, char var_name, lisp* val);
// used in PRINT
void Inter_printlist(Program* p, lisp* l);

// *** LIST ***
// return the lisp associated with variable name var_name
lisp* Inter_listvar(Program* p, char var_name);
// used in RETFUNC branch in LIST
lisp* Inter_RETFUNC(Program* p);
// used in VAR branch in LIST
lisp* Inter_LIST_VAR(Program* p);
// used in LITERAL branch in LIST
lisp* Inter_LIST_LITERAL(Program* p);

// *** IF & LOOP ***
// if the val in boolean matches cond, then execute next part, otherwise skip it
void Inter_if(Program* p, lisp* boolean, int cond);
// if the val in boolean is 1, then set cw to loop_start, otherwise exit loop
void Inter_loop(Program* p, lisp* boolean, int loop_start);

// *** LISTFUNC ***
lisp* Inter_CONS(Program* p);

// *** INTFUNC ***
// perform plus, minus, multiply and division, based on op
lisp* Inter_CALC(Program* p, char* op);
lisp* Inter_LENGTH(Program* p);

// *** BOOLFUNC ***
lisp* Inter_BOOL(Program* p, char* cmp);

// used in IF and WHILE statement to skip code when condition is false
int Inter_skipInstrcts(Program* p);

void free_var_list(VarList* vl, char* fname);
// dummy functions
void free_pool(Pool* pool, char* fname);
#endif
