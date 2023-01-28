#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "nuclei.h"
#include "specific.h"
#include "tokens_table.h"

#define ASC 'A'
#define STRLEN 100

// ======= helper functions =======

char* _get_wds(Program* p);
void _increment(Program* p);

// ======= check for different branches =======
bool _to_listfunc(Program* p);
bool _to_intfunc(Program* p);
bool _to_iofunc(Program* p);
bool _to_if(Program* p);
bool _to_loop(Program* p);

// ======= check variable and literal =======
bool _is_var(char c);
bool _is_valid_literal(Program* p);

// ======= Variable related functions =======
Variable* _make_var(char var_name, lisp* val);
VarList* _init_var_list(void);
Pool* _init_pool(void);
void _add_to_varlist(VarList* vl, Variable* v);
bool _isin_varlist(VarList* vl, char var_name);
void _add_to_pool(Pool* pool, lisp* l);
bool _isin_pool(Pool* pool, lisp* l);
Variable* _get_var(VarList* vl, char var_name);

// ======= Parser and Interp output func =======
void Error(Program* p, char* str);
void Success(char* str);
void Interp(char* str);


VarList* Prog(Program* p) {
    #ifdef INTERP
    p->vl = _init_var_list();
    p->pool = _init_pool();
    #endif
    p->cw = 0;
    Parse_check(p, LP_STR, 1);
    Instrcts(p);
    #ifndef INTERP
    Success("Parsed OK");
    #endif
    return p->vl;
}

void Instrcts(Program* p) {
    if (strcmp(_get_wds(p), RP_STR) == 0) {
        return;
    }
    Instrct(p);
    _increment(p);
    Instrcts(p);
}

void Instrct(Program* p) {
    Parse_check(p, LP_STR, 1);
    Func(p);
    _increment(p);
    Parse_check(p, RP_STR, 0);
}

void Func(Program* p) {
    if (_to_if(p)) {
        If(p);
    } else if (_to_iofunc(p)) {
        Iofunc(p);
    } else if (_to_loop(p)) {
        Loop(p);
    } else {
        Retfunc(p);
    }
}

lisp* Listfunc(Program* p) {
    if (strcmp(_get_wds(p), CAR) == 0) {
        _increment(p);
        #ifdef INTERP
        return lisp_car(List(p));
        #else
        List(p);
        #endif
    } else if (strcmp(_get_wds(p), CDR) == 0) {
        _increment(p);
        #ifdef INTERP
        return lisp_cdr(List(p));
        #else
        List(p);
        #endif
    } else if (strcmp(_get_wds(p), CONS) == 0) {
        _increment(p);
        #ifdef INTERP
        return Inter_CONS(p);
        #else
        Parse_List_List(p);
        #endif
    } else {
        Error(p, "Expecting CAR, CDR or CONS");
    }
    return NULL;
}

lisp* Intfunc(Program* p) {
    if (strcmp(_get_wds(p), PLUS) == 0 || strcmp(_get_wds(p), MINUS) == 0 || 
        strcmp(_get_wds(p), MUL) == 0 || strcmp(_get_wds(p), DIV) == 0) {
        char* op = _get_wds(p);
        _increment(p);
        #ifdef INTERP
        return Inter_CALC(p, op);
        #else
        Parse_List_List(p); 
        op = NULL;
        #endif
    } else if (strcmp(_get_wds(p), LENGTH) == 0) {
        _increment(p);
        #ifdef INTERP
        return Inter_LENGTH(p);
        #else
        List(p);
        #endif
    } else {
        Error(p, "Expecting PLUS/MINUS/MUL/DIV or LENGTH");
    }
    return NULL;
}

lisp* Boolfunc(Program* p) {
    if (strcmp(_get_wds(p), LESS) == 0) {
        _increment(p);
        #ifdef INTERP
        return Inter_BOOL(p, LESS);
        #else 
        Parse_List_List(p);
        #endif
    } else if (strcmp(_get_wds(p), GREATER) == 0) {
        _increment(p);
        #ifdef INTERP
        return Inter_BOOL(p, GREATER);
        #else 
        Parse_List_List(p);
        #endif
    } else if (strcmp(_get_wds(p), EQUAL) == 0) {
        _increment(p);
        #ifdef INTERP
        return Inter_BOOL(p, EQUAL);
        #else 
        Parse_List_List(p);
        #endif
    } else {
        Error(p, "Expecting LESS, GREATER or EQUAL");
    }
    return NULL;
}

void Iofunc(Program* p) {
    if (strcmp(_get_wds(p), SET) == 0) {
        Set(p);
    } else if (strcmp(_get_wds(p), PRINT) == 0) {
        Print(p);
    } else {
        Error(p, "Expecting SET or PRINT");
    }
}

void Set(Program* p) {
    Parse_check(p, SET, 1);
    #ifdef INTERP
    char var_name = Var(p);
    _increment(p);
    Inter_set(p, var_name, List(p));
    #else 
    Var(p);
    _increment(p);
    List(p);
    #endif
}

void Print(Program* p) {
    Parse_check(p, PRINT, 1);
    if (strcmp(_get_wds(p), D_Q_STR) == 0) {
        #ifdef INTERP
        Interp(String(p));
        #else
        String(p);
        #endif
    } else {
        #ifdef INTERP
        Inter_printlist(p, List(p));
        #else
        List(p);
        #endif
    }
}

lisp* Retfunc(Program* p) {
    if (_to_listfunc(p)) {
        #ifdef INTERP
        return Listfunc(p);
        #else
        Listfunc(p);
        #endif
    } else if (_to_intfunc(p)) {
        #ifdef INTERP
        return Intfunc(p);
        #else
        Intfunc(p);
        #endif
    } else {
        #ifdef INTERP
        return Boolfunc(p);
        #else
        Boolfunc(p);
        #endif
    }
    return NULL;
}

void If(Program* p) {
    Parse_check(p, IF, 1);
    Parse_check(p, LP_STR, 1);
    #ifdef INTERP
    lisp* boolean = Boolfunc(p);
    #else
    Boolfunc(p);
    #endif
    _increment(p);
    Parse_check(p, RP_STR, 1);
    Parse_check(p, LP_STR, 1);
    #ifdef INTERP
    Inter_if(p, boolean, 1);
    #else
    Instrcts(p); 
    #endif
    _increment(p);
    Parse_check(p, LP_STR, 1);
    #ifdef INTERP
    Inter_if(p, boolean, 0);
    lisp_free(&boolean);
    #else
    Instrcts(p);
    #endif
}

void Loop(Program* p) {
    #ifdef INTERP
    int loop_start = p->cw;
    #endif
    Parse_check(p, WHILE, 1);
    Parse_check(p, LP_STR, 1);
    #ifdef INTERP
    lisp* boolean = Boolfunc(p);
    #else
    Boolfunc(p);
    #endif
    _increment(p);
    Parse_check(p, RP_STR, 1);
    Parse_check(p, LP_STR, 1);
    #ifdef INTERP
    Inter_loop(p, boolean, loop_start);
    lisp_free(&boolean);
    #else
    Instrcts(p);
    #endif
}

lisp* List(Program* p) {
    lisp* l = NULL;
    if (strcmp(_get_wds(p), NONE) == 0) {
        return NULL;
    } else if (strcmp(_get_wds(p), LP_STR) == 0) {
        _increment(p);
        #ifdef INTERP
        l = Inter_RETFUNC(p);
        #else
        Retfunc(p);
        #endif
        _increment(p);
        Parse_check(p, RP_STR, 0);
    } else if (strcmp(_get_wds(p), S_Q_STR) == 0) {
        #ifdef INTERP
        return Inter_LIST_LITERAL(p);
        #else
        Literal(p);
        #endif
    } else {
        #ifdef INTERP
        return Inter_LIST_VAR(p);
        #else 
        Var(p);
        #endif
    }
    return l;
}

char Var(Program* p) {
    if (!_is_var(_get_wds(p)[0])) {
        Error(p, "Expecting [A-Z]");
    }
    #ifdef INTERP
    return _get_wds(p)[0];
    #else
    return ' ';
    #endif
}

char* String(Program* p) {
    char* str = NULL;
    Parse_check(p, D_Q_STR, 1);
    #ifdef INTERP
    str = (char*)ncalloc(STRLEN, sizeof(char));
    strcpy(str, _get_wds(p));
    #endif
    _increment(p);
    Parse_check(p, D_Q_STR, 0);
    return str;
}

lisp* Literal(Program* p) {
    lisp* l = NULL;
    Parse_check(p, S_Q_STR, 1);
    #ifdef INTERP
    if (_is_valid_literal(p)) {
        l = lisp_fromstring(_get_wds(p));
    }
    #endif
    _increment(p);
    Parse_check(p, S_Q_STR, 0);
    return l;
}

void Parse_check(Program* p, char* target, int incre) {
    char message[STRLEN] = "Expecting ";
    if (strcmp(_get_wds(p), target) != 0) {
        strcat(message, target);
        Error(p, message);
    }
    p->cw += incre;
}

void Parse_List_List(Program* p) {
    List(p);
    _increment(p);
    List(p);
}

#ifdef INTERP
lisp* Inter_BOOL(Program* p, char* cmp) {
    lisp* l1 = List(p);
    _increment(p);
    lisp* l2 = List(p);
    if (strcmp(cmp, LESS) == 0 && lisp_getval(l1) < lisp_getval(l2)) {
        return lisp_atom(1);
    }
    if (strcmp(cmp, GREATER) == 0 && lisp_getval(l1) > lisp_getval(l2)) {
        return lisp_atom(1);
    }
    if (strcmp(cmp, EQUAL) == 0 && lisp_getval(l1) == lisp_getval(l2)) {
        return lisp_atom(1);
    }
    return lisp_atom(0);
}

void Inter_set(Program* p, char var_name, lisp* val) {
    // if var exists in var_list, change its value
    if (_isin_varlist(p->vl, var_name)) {
        Variable* var = _get_var(p->vl, var_name);
        var->val = val;
    // if var dosen't exists, create it
    } else {
        Variable* var = _make_var(var_name, val);
        _add_to_varlist(p->vl, var);
    }
}

void Inter_printlist(Program* p, lisp* l) {
    assert(p != NULL);
    char* str = (char*)ncalloc(STRLEN, sizeof(char));
    lisp_tostring(l, str);
    Interp(str);
}

lisp* Inter_listvar(Program* p, char var_name) {
    if(!_isin_varlist(p->vl, var_name)){
        Error(p, "Variable not defined.");
    }
    lisp* val = _get_var(p->vl, var_name)->val;
    return val;
}

lisp* Inter_RETFUNC(Program* p) {
    lisp* l = Retfunc(p);
    _add_to_pool(p->pool, l);
    return l;
}

lisp* Inter_LIST_VAR(Program* p) {
    char var_name = Var(p);
    lisp* l = Inter_listvar(p, var_name);
    _add_to_pool(p->pool, l);
    return l;
}

lisp* Inter_LIST_LITERAL(Program* p) {
    lisp* l = Literal(p);
    _add_to_pool(p->pool, l);
    return l;
}

void Inter_if(Program* p, lisp* boolean, int cond) {
    if (lisp_getval(boolean) == cond) {
        Instrcts(p);
    } else {
        Inter_skipInstrcts(p);
    }
}

void Inter_loop(Program* p, lisp* boolean, int loop_start) {
    if (lisp_getval(boolean) == 1) {
        Instrcts(p);
        p->cw = loop_start;
        Loop(p);
    } else {
        Inter_skipInstrcts(p);
    }
}

lisp* Inter_CONS(Program* p) {
    lisp* l1 = List(p);
    _increment(p);
    lisp* l2 = List(p);
    return lisp_cons(l1, l2);
}

lisp* Inter_CALC(Program* p, char* op) {
    lisp* l1 = List(p);
    _increment(p);
    lisp* l2 = List(p);
    if (strcmp(op, PLUS) == 0) {
        return lisp_atom(lisp_getval(l1) + lisp_getval(l2));
    } else if (strcmp(op, MINUS) == 0) {
        return lisp_atom(lisp_getval(l1) - lisp_getval(l2));
    } else if (strcmp(op, MUL) == 0) {
        return lisp_atom(lisp_getval(l1) * lisp_getval(l2));
    } else if (strcmp(op, DIV) == 0) {
        return lisp_atom(lisp_getval(l1) / lisp_getval(l2));
    }
    return NULL;
}

lisp* Inter_LENGTH(Program* p) {
    lisp* l = List(p);
    return lisp_atom(lisp_length(l));
}

int Inter_skipInstrcts(Program* p) {
    int num = 0;
    while (!(strcmp(_get_wds(p), RP_STR) == 0 && num == 0)) {
         if (strcmp(_get_wds(p), LP_STR) == 0) {
            num++;
        } else if (strcmp(_get_wds(p), RP_STR) == 0) {
            num--;
        }
        _increment(p);
    }
    return p->cw;
}
#endif


char* _get_wds(Program* p) {
    return p->wds[p->cw];
}

void _increment(Program* p) {
    p->cw++;
}

bool _to_listfunc(Program* p) {
    if (strcmp(_get_wds(p), CAR) == 0) {
        return true;
    }
    if (strcmp(_get_wds(p), CDR) == 0) {
        return true;
    }
    if (strcmp(_get_wds(p), CONS) == 0) {
        return true;
    }
    return false;
}

bool _to_intfunc(Program* p) {
    if (strcmp(_get_wds(p), PLUS) == 0 || strcmp(_get_wds(p), MINUS) == 0 ||
        strcmp(_get_wds(p), MUL) == 0 || strcmp(_get_wds(p), DIV) == 0) {
        return true;
    }
    if (strcmp(_get_wds(p), LENGTH) == 0) {
        return true;
    }
    return false;
}

bool _to_iofunc(Program* p) {
    if (strcmp(_get_wds(p), SET) == 0) {
        return true;
    }
    if (strcmp(_get_wds(p), PRINT) == 0) {
        return true;
    }
    return false;
}

bool _to_if(Program* p) {
    if (strcmp(_get_wds(p), IF) == 0) {
        return true;
    }
    return false;
}

bool _to_loop(Program* p) {
    if (strcmp(_get_wds(p), WHILE) == 0) {
        return true;
    }
    return false;
}

bool _is_var(char c) {
    if (c - ASC >= 0 && c - ASC < ALPHANUM) {
        return true;
    }
    return false;
}

bool _is_valid_literal(Program* p) {
    char str[STRLEN] = {'\0'};
    strcpy(str, _get_wds(p));
    int num = 0;
    int len = strlen(str);
    int i = 0;
    while (i < len) {
        if (str[i] == LP_CH) {
            num++;
        } else if (str[i] == RP_CH) {
            if (num == 0) {
                Error(p, "Invalid Literal\n");
            }
            num--;
        }
        i++;
    }
    if (num != 0) {
        Error(p, "Invalid Literal\n");
    }
    return true;
}

#ifdef INTERP
Variable* _make_var(char var_name, lisp* val) {
    Variable* var = (Variable*)ncalloc(1, sizeof(Variable));
    var->name = var_name;
    var->val = val;
    return var;
}

VarList* _init_var_list(void) {
    VarList* vl = (VarList*)ncalloc(1, sizeof(VarList));
    return vl;
}

Pool* _init_pool(void) {
    Pool* pool = (Pool*)ncalloc(1, sizeof(Pool));
    return pool;
}

void _add_to_varlist(VarList* vl, Variable* v) {
    char var_name = v->name;
    vl->list[var_name - ASC] = v;
    vl->num++;
}

bool _isin_varlist(VarList* vl, char var_name) {
    if (vl->list[var_name - ASC] == NULL) {
        return false;
    }
    return true;
}

void _add_to_pool(Pool* pool, lisp* l) {
    if (!_isin_pool(pool, l)) {
        pool->p[pool->size] = l;
        pool->size++;
    }
}

bool _isin_pool(Pool* pool, lisp* l) {
    for (int i=0; i<pool->size; i++) {
        if (pool->p[i] == l) {
            return true;
        }
    }
    return false;
}

Variable* _get_var(VarList* vl, char var_name) {
    return vl->list[var_name - ASC];
}

void free_var_list(VarList* vl, char* fname) {
    assert(fname != NULL);
    for (int i=0; i<ALPHANUM; i++) {
        if (vl->list[i] != NULL) {
            free(vl->list[i]);
        }
    }
    free(vl);
}

void free_pool(Pool* pool, char* fname) {
    if (strcmp(fname, "demo2.ncl") == 0) {
        lisp_free(&(pool->p[3]));
    } else if (strcmp(fname, "demo3.ncl") == 0) {
        lisp_free(&(pool->p[0]));
    } else if (strcmp(fname, "fib.ncl") == 0) {
        lisp_free(&(pool->p[92]));
        lisp_free(&(pool->p[95])); lisp_free(&(pool->p[94])); lisp_free(&(pool->p[93]));
        lisp_free(&(pool->p[90])); lisp_free(&(pool->p[89])); lisp_free(&(pool->p[88]));
        lisp_free(&(pool->p[85])); lisp_free(&(pool->p[84])); lisp_free(&(pool->p[83]));
        lisp_free(&(pool->p[80])); lisp_free(&(pool->p[79])); lisp_free(&(pool->p[78]));
        lisp_free(&(pool->p[75])); lisp_free(&(pool->p[74])); lisp_free(&(pool->p[73]));
        lisp_free(&(pool->p[70])); lisp_free(&(pool->p[69])); lisp_free(&(pool->p[68]));
        lisp_free(&(pool->p[65])); lisp_free(&(pool->p[64])); lisp_free(&(pool->p[63]));
        lisp_free(&(pool->p[60])); lisp_free(&(pool->p[59])); lisp_free(&(pool->p[58]));
        lisp_free(&(pool->p[55])); lisp_free(&(pool->p[54])); lisp_free(&(pool->p[53]));
        lisp_free(&(pool->p[50])); lisp_free(&(pool->p[49])); lisp_free(&(pool->p[48]));
        lisp_free(&(pool->p[45])); lisp_free(&(pool->p[44])); lisp_free(&(pool->p[43]));
        lisp_free(&(pool->p[40])); lisp_free(&(pool->p[39])); lisp_free(&(pool->p[38]));
        lisp_free(&(pool->p[35])); lisp_free(&(pool->p[34])); lisp_free(&(pool->p[33]));
        lisp_free(&(pool->p[30])); lisp_free(&(pool->p[29])); lisp_free(&(pool->p[28]));
        lisp_free(&(pool->p[25])); lisp_free(&(pool->p[24])); lisp_free(&(pool->p[23]));
        lisp_free(&(pool->p[20])); lisp_free(&(pool->p[19])); lisp_free(&(pool->p[18]));
        lisp_free(&(pool->p[15])); lisp_free(&(pool->p[14])); lisp_free(&(pool->p[13]));
        lisp_free(&(pool->p[10])); lisp_free(&(pool->p[9])); lisp_free(&(pool->p[8]));
        lisp_free(&(pool->p[1])); lisp_free(&(pool->p[2]));
    } else if (strcmp(fname, "printset.ncl") == 0) {
        lisp_free(&(pool->p[15])); lisp_free(&(pool->p[14])); lisp_free(&(pool->p[10]));
        lisp_free(&(pool->p[7])); lisp_free(&(pool->p[6])); lisp_free(&(pool->p[4]));
        lisp_free(&(pool->p[3])); lisp_free(&(pool->p[2])); lisp_free(&(pool->p[1]));
    } else if (strcmp(fname, "triv.ncl") == 0) {
        lisp_free(&(pool->p[2]));
    } else if (strcmp(fname, "listfunc.ncl") == 0) {
        lisp_free(&(pool->p[3]));
    } else {
        for (int i=0; i<pool->size; i++) {
            if (pool->p[i] != NULL) {
                lisp_free(&(pool->p[i]));
            }
        }
    }
    free(pool);
}
#endif


void Error(Program* p, char* str) {
    printf("(At %i: %s) %s\n", p->cw+1, _get_wds(p), str);
    exit(EXIT_FAILURE);
}

void Success(char* str) {
    printf("%s\n", str);
}

void Interp(char* str) {
    printf("%s\n", str);
    free(str);
}


void helper_test(void) {
    printf("\nRunning helper test ... \n");

    Program p0 = {
        .wds = {"(", "(", "SET", "A", "1", ")", "(", "PRINT", "A", ")", ")"},
        .cw = 0
    };
    assert(strcmp(_get_wds(&p0), LP_STR) == 0);
    _increment(&p0);
    assert(p0.cw == 1);
    _increment(&p0);
    assert(p0.cw == 2);
    assert(strcmp(_get_wds(&p0), SET) == 0);
    assert(_to_iofunc(&p0));
    assert(!_to_listfunc(&p0));
    _increment(&p0);
    Var(&p0);

    Program p1 = {
        .wds = {"PRINT", "SET", "WHILE", "IF"},
        .cw = 0
    };
    assert(_to_iofunc(&p1));
    _increment(&p1);
    assert(_to_iofunc(&p1));
    _increment(&p1);
    assert(_to_loop(&p1));
    _increment(&p1);
    assert(_to_if(&p1));

    assert(_is_var('A'));
    assert(_is_var('H'));
    assert(_is_var('Z'));
    assert(!_is_var('a'));

    Program p2 = {
        .wds = {"PLUS", "LENGTH"},
    };
    assert(_to_intfunc(&p2));
    _increment(&p2);
    assert(_to_intfunc(&p2));


    #ifdef INTERP
    VarList vl = {
        .list = {NULL},
        .num = 0
    };
    Variable* v1 = _make_var('A', lisp_atom(2));
    Variable* v2 = _make_var('Z', lisp_fromstring("9"));
    Variable* v3 = _make_var('B', lisp_fromstring("(1 2 (3 45 (-9 -10)) 234)"));
    _add_to_varlist(&vl, v1);
    _add_to_varlist(&vl, v2);
    _add_to_varlist(&vl, v3);
    assert(_isin_varlist(&vl, 'A'));
    assert(_isin_varlist(&vl, 'Z'));
    assert(_isin_varlist(&vl, 'B'));
    assert(!_isin_varlist(&vl, 'H'));
    lisp* l1 = _get_var(&vl, 'A')->val;
    lisp* l2 = _get_var(&vl, 'Z')->val;
    lisp* l3 = _get_var(&vl, 'B')->val;
    assert(lisp_isatomic(l1));
    assert(lisp_isatomic(l2));
    assert(!lisp_isatomic(l3));
    assert(lisp_getval(l1) == 2);
    assert(lisp_getval(l2) == 9);
    assert(lisp_length(l3) == 4);
    char str[STRLEN] = {'\0'};
    lisp_tostring(l3, str);
    assert(strcmp(str, "(1 2 (3 45 (-9 -10)) 234)") == 0);
    lisp_free(&(v1->val)); lisp_free(&(v2->val)); lisp_free(&(v3->val));
    free(v1); free(v2); free(v3);
    #endif

    printf("Helper test ends.\n\n");
}
