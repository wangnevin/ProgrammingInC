#define LISPIMPL "Linked"

struct lisp {
    struct lisp* cdr;  // point right
    struct lisp* car;  // point down
    atomtype val;
};
