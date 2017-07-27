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
typedef int (*expr_value_getter)(char * varname, char ** value);

extern expr_parser * expr_parser_new();
extern void expr_parser_delete(expr_parser *parser);
extern void expr_parser_reset(expr_parser *parser);
extern void expr_parser_parse(expr_parser * parser, char *exp_str);
extern int expr_parser_execute(expr_parser *parser, int *result,  expr_value_getter getter);
extern void expr_parser_print_tree(expr_parser *parser);

#ifdef __cplusplus
}
#endif
