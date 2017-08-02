/**
 * author: jason886
 * link: https://github.com/Jason886/expr_parser.git
 *
 */
#include "array.h"
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct expr_parser expr_parser;
typedef struct expr_value_t expr_value_t;
typedef int (*expr_value_getter)(char * varname, expr_value_t *value, void *usrdata);

extern expr_parser * expr_parser_new();
extern void expr_parser_delete(expr_parser *parser);
extern void expr_parser_reset(expr_parser *parser);
extern int expr_parser_parse(expr_parser * parser, char *exp_str);
extern int expr_parser_execute(expr_parser *parser, int *result, \
		expr_value_getter getter, void * usrdata);
extern void expr_parser_print_tree(expr_parser *parser);

extern void expr_value_set_int(expr_value_t *value, int64_t n);
extern void expr_value_set_double(expr_value_t *value, double d);
extern void expr_value_set_str(expr_value_t *value, char *p, size_t size);
extern void expr_value_clear(expr_value_t * value);


#ifdef __cplusplus
}
#endif

