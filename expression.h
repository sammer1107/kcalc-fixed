/* MathEX is a mathematical expression evaluator.
 * It takes string as input and returns floating-point number as a result.
 */

#ifndef EXPRESSION_H_
#define EXPRESSION_H_

#ifdef __KERNEL__
#include <linux/slab.h>
#else
#include <stddef.h>
#include <stdlib.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define vec(T)   \
    struct {     \
        T *buf;  \
        int len; \
        int cap; \
    }
#define vec_init() \
    {              \
        NULL, 0, 0 \
    }
#define vec_len(v) ((v)->len)
#define vec_unpack(v) \
    (char **) &(v)->buf, &(v)->len, &(v)->cap, sizeof(*(v)->buf)
#define vec_push(v, val) \
    vec_expand(vec_unpack(v)) ? -1 : ((v)->buf[(v)->len++] = (val), 0)
#define vec_nth(v, i) (v)->buf[i]
#define vec_peek(v) (v)->buf[(v)->len - 1]
#define vec_pop(v) (v)->buf[--(v)->len]
#ifdef __KERNEL__
#define vec_free(v) (kfree((v)->buf), (v)->buf = NULL, (v)->len = (v)->cap = 0)
#else
#define vec_free(v) (free((v)->buf), (v)->buf = NULL, (v)->len = (v)->cap = 0)
#endif
#define vec_foreach(v, var, iter)                                              \
    if ((v)->len > 0)                                                          \
        for ((iter) = 0; (iter) < (v)->len && (((var) = (v)->buf[(iter)]), 1); \
             ++(iter))

/* Simple expandable vector implementation */
static inline int vec_expand(char **buf, int *length, int *cap, int memsz)
{
    if (*length + 1 > *cap) {
        void *ptr;
        int n = (*cap == 0) ? 1 : *cap << 1;
#ifdef __KERNEL__
        ptr = krealloc(*buf, n * memsz, GFP_KERNEL);
#else
        ptr = realloc(*buf, n * memsz);
#endif
        if (!ptr)
            return -1; /* allocation failed */
        *buf = (char *) ptr;
        *cap = n;
    }
    return 0;
}

/*
 * Expression data types
 */
struct expr_func;
typedef vec(struct expr) vec_expr_t;
typedef void (*exprfn_cleanup_t)(struct expr_func *f, void *context);
typedef uint64_t (*exprfn_t)(struct expr_func *f,
                             vec_expr_t args,
                             void *context);

enum expr_type {
    OP_UNKNOWN,
    OP_UNARY_MINUS,
    OP_UNARY_LOGICAL_NOT,
    OP_UNARY_BITWISE_NOT,

    OP_POWER,
    OP_DIVIDE,
    OP_MULTIPLY,
    OP_REMAINDER,

    OP_PLUS,
    OP_MINUS,

    OP_SHL,
    OP_SHR,

    OP_LT,
    OP_LE,
    OP_GT,
    OP_GE,
    OP_EQ,
    OP_NE,

    OP_BITWISE_AND,
    OP_BITWISE_OR,
    OP_BITWISE_XOR,

    OP_LOGICAL_AND,
    OP_LOGICAL_OR,

    OP_ASSIGN,
    OP_COMMA,

    OP_CONST,
    OP_VAR,
    OP_FUNC,
};

struct expr {
    enum expr_type type;
    union {
        struct {
            uint64_t value;
        } num;
        struct {
            uint64_t *value;
        } var;
        struct {
            vec_expr_t args;
        } op;
        struct {
            struct expr_func *f;
            vec_expr_t args;
            void *context;
        } func;
    } param;
};

struct expr_string {
    const char *s;
    int n;
};
struct expr_arg {
    int oslen;
    int eslen;
    vec_expr_t args;
};

typedef vec(struct expr_string) vec_str_t;
typedef vec(struct expr_arg) vec_arg_t;

/*
 * Functions
 */
struct expr_func {
    const char *name;
    exprfn_t f;
    exprfn_cleanup_t cleanup;
    size_t ctxsz;
};

struct expr_func *expr_func(struct expr_func *funcs, const char *s, size_t len);

/*
 * Variables
 */
struct expr_var {
    uint64_t value;
    struct expr_var *next;
    char name[];
};

struct expr_var_list {
    struct expr_var *head;
};

struct expr_var *expr_var(struct expr_var_list *vars,
                          const char *s,
                          size_t len);

uint64_t expr_eval(struct expr *e);

#define EXPR_TOP (1 << 0)
#define EXPR_TOPEN (1 << 1)
#define EXPR_TCLOSE (1 << 2)
#define EXPR_TNUMBER (1 << 3)
#define EXPR_TWORD (1 << 4)
#define EXPR_TDEFAULT (EXPR_TOPEN | EXPR_TNUMBER | EXPR_TWORD)

#define EXPR_UNARY (1 << 5)
#define EXPR_COMMA (1 << 6)

int expr_next_token(const char *s, size_t len, int *flags);

struct expr *expr_create(const char *s,
                         size_t len,
                         struct expr_var_list *vars,
                         struct expr_func *funcs);

void expr_destroy(struct expr *e, struct expr_var_list *vars);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* EXPRESSION_H_ */
