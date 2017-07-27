#include "array.h"
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>



typedef struct expr_parser expr_parser;

typedef struct _expr_value {
	int valueType;  // 0- 数字 1-字符串
	union {
		int64_t nValue;
		char *pValue;
	};
} expr_value;

typedef int (*expr_value_getter)(char * varname, char ** value);

expr_parser * expr_parser_new();
void expr_parser_delete(expr_parser *parser);
void expr_parser_reset(expr_parser *parser);
void expr_parser_parse(expr_parser * parser, char *exp_str);
int expr_parser_execute(expr_parser *parser, int *result,  expr_value_getter getter);

//void _output_node(expr_node_t * node);
