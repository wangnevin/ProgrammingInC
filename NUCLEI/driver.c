#include <stdio.h>
#include <assert.h>
#include "lisp.h"
#include "specific.h"
#include "readfile.h"

#define DUMMY_NUM 100
#define DUMMY_LEN 100

void main_test(void);
void helper_test(void);
void read_file_test(void);
void test(void);
#ifdef INTERP
void dummy_free(VarList* vl, Pool* pool, char* fname);
#endif

int main(int argc, char* argv[]) {
    Program p;
    assert(argc == 2);
    if (strcmp(argv[1], "test") == 0) {
        test();
        return 0;
    }

    read_file(argv[1], &p);
    
    #ifdef INTERP
    VarList* vl = Prog(&p);
    dummy_free(vl, p.pool, argv[1]);
    #else
    Prog(&p);
    #endif
    return 0;
}

#ifdef INTERP
void dummy_free(VarList* vl, Pool* pool, char* fname) {
    char valid_fname[DUMMY_NUM][DUMMY_LEN] = {
        "demo1.ncl","demo2.ncl","demo3.ncl","fib.ncl","if_parsefail_interppass.ncl",
        "iftest.ncl","inf_loop.ncl","literal_parsepass_interpfail.ncl","printset.ncl",
        "simploop.ncl","testl.ncl","triv.ncl","listfunc.ncl","basicprint.ncl"
    };
    int num = 14;
    for (int i=0; i<num; i++) {
        if (strcmp(fname, valid_fname[i]) == 0) {
            free_var_list(vl, fname);
            free_pool(pool, fname);
            return;
        }
    }
}
#endif

void main_test(void) {
    Program demo1 = {
        .wds = {"(", "(", "SET", "A", "'", "1", "'", ")", "(", "PRINT", "A", ")", ")"},
        .cw = 0,
    };

    Program demo2 = {
        .wds = {"(", "(", "PRINT", "(", "CONS", "'", "1", "'", "(", "CONS", "'", "2", "'", "NIL", ")", ")", ")", ")"},
        .cw = 0,
    };

    Program demo3 = {
        .wds = {"(", "(", "SET", "A", "'", "(5 (1 2 3))", "'", ")", "(", "PRINT", "(", "CAR", "A", ")", ")", ")"},
        .cw = 0,
    };

    Program fib = {
        .wds = {
            "(",
            "(","SET","L","'","(1 0)","'",")",
            "(","SET","C","'","2","'",")",
            "(","WHILE","(","LESS","C","'","20","'",")","(",
                "(","SET","N","(","PLUS","(","CAR","L",")","(","CAR","(","CDR","L",")",")",")",")",
                "(","SET","M","(","CONS","N","L",")",")",
                "(","SET","L","M",")",
                "(","SET","B","(","PLUS","'","1","'","C",")",")",
                "(","SET","C","B",")",
            ")",")",
            "(","PRINT","M",")",
            ")"
        },
        .cw = 0,
    };

    Program iftest = {
        .wds = {
            "(",
                "(","SET","A","'","2","'",")",
                "(",
                    "IF","(","EQUAL","A","'","3","'",")","(",
                        "(","PRINT","\"","A is equal to 2","\"",")",
                        "(","PRINT","\"","Boolean is true","\"",")",
                    ")","(",
                        "(","PRINT","\"","A is not equal to 2","\"",")",
                        "(","PRINT","\"","Boolean is false","\"",")",
                    ")",
                ")",
                "(","PRINT","A",")",
            ")"
        },
        .cw = 0,
    };
    
    Program boolean = {
        .wds = {
            "(",
                "(","SET","A","(","EQUAL","'","3","'","'","3","'",")",")",
                "(","SET","B","(","EQUAL","'","3","'","'","2","'",")",")",
                "(","SET","C","(","GREATER","'","3","'","'","2","'",")",")",
                "(","SET","D","(","GREATER","'","1","'","'","2","'",")",")",
                "(","SET","E","(","LESS","'","3","'","'","2","'",")",")",
                "(","SET","F","(","LESS","'","1","'","'","2","'",")",")",
            ")"
        },
        .cw = 0,
    };

    Program intfunc = {
        .wds = {
            "(",
                "(","SET","A","(","PLUS","'","-5","'","'","3","'",")",")",
                "(","SET","B","(","LENGTH","'","(1 -2 (3 -4) (5 6 (10 11)) 7)","'",")",")",
            ")"
        },
        .cw = 0
    };

    Program listfunc = {
        .wds = {
            "(",
                "(","SET","A","'","1","'",")",
                "(","SET","B","'","2","'",")",
                "(","SET","C","(","CONS","A","NIL",")",")",
                "(","SET","D","(","CONS","B","C",")",")",    
            ")"
        },
        .cw = 0
    };

    #ifdef INTERP
    printf("\n========== interp test begin ========== \n");
    Program p1 = {
        .wds = {"(","A",")"}
    };
    VarList* vl = (VarList*)ncalloc(1, sizeof(VarList));
    Pool* pool = (Pool*)ncalloc(1, sizeof(Pool));
    p1.vl = vl;
    p1.pool = pool;
    lisp* l = lisp_fromstring("(1 2 (3 4 5))");
    Inter_set(&p1, 'A', l);
    assert(p1.vl->list[0]->name == 'A');
    char str[100] = {'\0'};
    lisp_tostring(p1.vl->list[0]->val, str);
    assert(strcmp("(1 2 (3 4 5))", str) == 0);

    lisp* l1 = Inter_listvar(&p1, 'A');
    str[0] = '\0';
    lisp_tostring(l1, str);
    assert(strcmp("(1 2 (3 4 5))", str) == 0);

    p1.cw = 1;
    lisp* l2 =  Inter_LIST_VAR(&p1);
    str[0] = '\0';
    lisp_tostring(l2, str);
    assert(strcmp("(1 2 (3 4 5))", str) == 0);
    assert(p1.pool->size == 1);
    str[0] = '\0';
    lisp_tostring(p1.pool->p[0], str);
    assert(strcmp("(1 2 (3 4 5))", str) == 0);

    lisp_free(&l);
    free_var_list(vl, "p1");
    free(pool);

    Program p2 = {
        .wds = {"'","(1 2 (3 4) 5)","'"},
        .cw = 0
    };
    VarList* vl_2 = (VarList*)ncalloc(1, sizeof(VarList));
    Pool* pool_2 = (Pool*)ncalloc(1, sizeof(Pool));
    p2.vl = vl_2;
    p2.pool = pool_2;
    lisp* l3 = Inter_LIST_LITERAL(&p2);
    str[0] = '\0';
    lisp_tostring(l3, str);
    assert(strcmp("(1 2 (3 4) 5)", str) == 0);
    assert(p2.pool->size == 1);
    str[0] = '\0';
    lisp_tostring(p2.pool->p[0], str);
    assert(strcmp("(1 2 (3 4) 5)", str) == 0);

    lisp_free(&l3);
    free_var_list(vl_2, "p2");
    free(pool_2);

    Program p3 = {
        .wds = {"CONS","'","1","'","'","(2)","'"},
        .cw = 1
    };
    Pool* pool_3 = (Pool*)ncalloc(1, sizeof(Pool));
    p3.pool = pool_3;
    lisp* l4 = Inter_CONS(&p3);
    assert(p3.pool->size == 2);
    str[0] = '\0';
    lisp_tostring(l4, str);
    assert(strcmp("(1 2)", str) == 0);

    lisp_free(&l4);
    free(pool_3);

    Program p4 = {
        .wds = {"'","1","'","'","2","'"},
        .cw = 0
    };
    Pool* pool_4 = (Pool*)ncalloc(1, sizeof(Pool));
    p4.pool = pool_4;
    lisp* l5 = Inter_CALC(&p4, "PLUS");
    assert(p4.pool->size == 2);
    str[0] = '\0';
    lisp_tostring(l5, str);
    assert(strcmp("3", str) == 0);
    assert(lisp_getval(l5) == 3);

    lisp_free(&l5);
    lisp_free(&(p4.pool->p[0]));
    lisp_free(&(p4.pool->p[1]));
    free(pool_4);

    Program p5 = {
        .wds = {"'","(1 2 (3 4) 5)","'"},
        .cw = 0
    };
    Pool* pool_5 = (Pool*)ncalloc(1, sizeof(Pool));
    p5.pool = pool_5;
    lisp* l6 = Inter_LENGTH(&p5);
    assert(p5.pool->size == 1);
    str[0] = '\0';
    lisp_tostring(l6, str);
    assert(strcmp("4", str) == 0);
    assert(lisp_getval(l6) == 4);

    lisp_free(&l6);
    lisp_free(&(p5.pool->p[0]));
    free(pool_5);

    Program p6 = {
        .wds = {"'","23","'","'","45","'"},
        .cw = 0
    };
    Pool* pool_6 = (Pool*)ncalloc(1, sizeof(Pool));
    p6.pool = pool_6;
    lisp* l7 = Inter_BOOL(&p6, "LESS");
    assert(p6.pool->size == 2);
    assert(lisp_getval(l7) == 1);
    lisp_free(&l7);
    lisp_free(&(p6.pool->p[0]));
    lisp_free(&(p6.pool->p[1]));
    free(pool_6);

    Pool* pool_7 = (Pool*)ncalloc(1, sizeof(Pool));
    p6.pool = pool_7;
    p6.cw = 0;
    lisp* l8 = Inter_BOOL(&p6, "GREATER");
    assert(p6.pool->size == 2);
    assert(lisp_getval(l8) == 0);
    lisp_free(&l8);
    lisp_free(&(p6.pool->p[0]));
    lisp_free(&(p6.pool->p[1]));
    free(pool_7);

    Pool* pool_8 = (Pool*)ncalloc(1, sizeof(Pool));
    p6.pool = pool_8;
    p6.cw = 0;
    lisp* l9 = Inter_BOOL(&p6, "EQUAL");
    assert(p6.pool->size == 2);
    assert(lisp_getval(l9) == 0);
    lisp_free(&l9);
    lisp_free(&(p6.pool->p[0]));
    lisp_free(&(p6.pool->p[1]));
    free(pool_8);
    
    Pool* pool_9 = (Pool*)ncalloc(1, sizeof(Pool));
    p6.pool = pool_9;
    p6.cw = 0;
    lisp* l10 = Inter_CALC(&p6, "MINUS");
    assert(lisp_getval(l10) == -22);
    lisp_free(&l10);
    lisp_free(&(p6.pool->p[0]));
    lisp_free(&(p6.pool->p[1]));
    free(pool_9);

    Pool* pool_10 = (Pool*)ncalloc(1, sizeof(Pool));
    p6.pool = pool_10;
    p6.cw = 0;
    lisp* l11 = Inter_CALC(&p6, "MUL");
    assert(lisp_getval(l11) == 1035);
    lisp_free(&l11);
    lisp_free(&(p6.pool->p[0]));
    lisp_free(&(p6.pool->p[1]));
    free(pool_10);

    Program p7 = {
        .wds = {"'","3","'","'","2","'"},
        .cw = 0
    };
    Pool* pool_11 = (Pool*)ncalloc(1, sizeof(Pool));
    p7.pool = pool_11;
    p7.cw = 0;
    lisp* l12 = Inter_CALC(&p7, "DIV");
    assert(lisp_getval(l12) == 1);
    lisp_free(&l12);
    lisp_free(&(p7.pool->p[0]));
    lisp_free(&(p7.pool->p[1]));
    free(pool_11);

    Program p8 = {
        .wds = {"MINUS","'","10","'","'","4","'",
                "MUL",  "'","4", "'","'","6","'",
                "DIV",  "'","20","'","'","5","'"},
        .cw = 0
    };
    Pool* pool_12 = (Pool*)ncalloc(1, sizeof(Pool));
    p8.pool = pool_12;
    lisp* l13 = Intfunc(&p8);
    assert(lisp_getval(l13) == 6);
    p8.cw = 7;
    lisp* l14 = Intfunc(&p8);
    assert(lisp_getval(l14) == 24);
    p8.cw = 14;
    lisp* l15 = Intfunc(&p8);
    assert(lisp_getval(l15) == 4);
    lisp_free(&l13); lisp_free(&l14); lisp_free(&l15);
    lisp_free(&(p8.pool->p[0]));
    lisp_free(&(p8.pool->p[1]));
    lisp_free(&(p8.pool->p[2]));
    lisp_free(&(p8.pool->p[3]));
    lisp_free(&(p8.pool->p[4]));
    lisp_free(&(p8.pool->p[5]));
    free(pool_12);

    VarList* vl1 = Prog(&demo1);
    assert(vl1->list[0] != NULL);
    assert(vl1->num == 1);
    assert(lisp_isatomic(vl1->list[0]->val));
    assert(vl1->list[0]->name == 'A');
    assert(lisp_getval(vl1->list[0]->val) == 1);
    printf("demo1 interp pass\n");

    VarList* vl2 = Prog(&demo2);
    assert(demo2.pool->size != 0);
    printf("demo2 interp pass\n");

    VarList* vl3 = Prog(&demo3);
    assert(vl3->list[0] != NULL);
    assert(vl3->num == 1);
    assert(lisp_length(vl3->list[0]->val) == 2);
    char tmp[100] = {'\0'};
    lisp_tostring(vl3->list[0]->val, tmp);
    assert(strcmp(tmp, "(5 (1 2 3))") == 0);
    printf("demo3 interp pass\n");
    
    VarList* vl4 = Prog(&fib);
    assert(lisp_length(vl4->list['M'-'A']->val) == 20);
    tmp[0] = '\0';
    lisp_tostring(vl4->list['M'-'A']->val, tmp);
    assert(strcmp(tmp, "(4181 2584 1597 987 610 377 233 144 89 55 34 21 13 8 5 3 2 1 1 0)") == 0);
    printf("fib interp pass\n");

    VarList* vl5 = Prog(&iftest);
    assert(vl5->list[0] != NULL);
    assert(vl5->num == 1);
    assert(lisp_isatomic(vl5->list[0]->val));
    assert(vl5->list[0]->name == 'A');
    assert(lisp_getval(vl5->list[0]->val) == 2);
    printf("iftest interp pass\n");

    VarList* vl6 = Prog(&boolean);
    assert(lisp_getval(vl6->list[0]->val) == 1);
    assert(lisp_getval(vl6->list[1]->val) == 0);
    assert(lisp_getval(vl6->list[2]->val) == 1);
    assert(lisp_getval(vl6->list[3]->val) == 0);
    assert(lisp_getval(vl6->list[4]->val) == 0);
    assert(lisp_getval(vl6->list[5]->val) == 1);
    printf("boolean interp pass\n");

    VarList* vl7 = Prog(&intfunc);
    assert(lisp_getval(vl7->list[0]->val) == -2);
    assert(lisp_getval(vl7->list[1]->val) == 5);
    printf("intfunc interp pass\n");

    VarList* vl8 = Prog(&listfunc);
    assert(lisp_getval(vl8->list[0]->val) == 1);
    assert(lisp_getval(vl8->list[1]->val) == 2);
    assert(lisp_length(vl8->list[2]->val) == 1);
    assert(lisp_length(vl8->list[3]->val) == 2);
    tmp[0] = '\0';
    lisp_tostring(vl8->list[3]->val, tmp);
    assert(strcmp(tmp, "(2 1)") == 0);
    printf("listfunc interp pass\n");

    free_var_list(vl1, "demo1.ncl");
    free_var_list(vl2, "demo2.ncl");
    free_var_list(vl3, "demo3.ncl");
    free_var_list(vl4, "fib.ncl");
    free_var_list(vl5, "iftest.ncl");
    free_var_list(vl6, "boolean.ncl");
    free_var_list(vl7, "intfunc.ncl");
    free_var_list(vl8, "listfunc.ncl");
    free_pool(demo1.pool, "demo1.ncl");
    free_pool(demo2.pool, "demo2.ncl");
    free_pool(demo3.pool, "demo3.ncl");
    free_pool(fib.pool, "fib.ncl");
    free_pool(iftest.pool, "iftest.ncl");
    free_pool(boolean.pool, "boolean.ncl");
    free_pool(intfunc.pool, "intfunc.ncl");
    free_pool(listfunc.pool, "listfunc.ncl");
    printf("========== interp test ends ========== \n");
    #else
    printf("\n========== parse test begin ========== \n");
    Program p1 = {
        .wds = {"PRINT"},
        .cw = 0
    };
    Parse_check(&p1, "PRINT", 1);
    assert(p1.cw == 1);
    p1.cw = 0;
    Parse_check(&p1, "PRINT", 0);
    assert(p1.cw == 0);

    Program p2 = {
        .wds = {"(","(","LESS","'","1","'","'","2","'",")",")"},
        .cw = 3
    };
    Parse_List_List(&p2);

    Prog(&demo1);
    Prog(&demo2);
    Prog(&demo3);
    Prog(&fib);
    Prog(&iftest);
    Prog(&boolean);
    Prog(&intfunc);
    Prog(&listfunc);
    printf("========== parse test ends ========== \n");
    #endif
}

void test(void) {
    helper_test();
    read_file_test();
    main_test();
}
