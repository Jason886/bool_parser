#include "array.h"
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

/*
 * 语法树节点类型
 */
static const int EXPR_NODE_OPER		= 0;
static const int EXPR_NODE_DATA		= 1;

typedef struct _expr_node_t {
	int type;			// 0-运算符, 1-数据
	union {
		int oper;
		char * data;
	};
	size_t offset;
	struct _expr_node_t * left;
	struct _expr_node_t * right;
} expr_node_t;

typedef struct _expr_parser {
	struct _expr_node_t * root;
	array_t _stack1;
	array_t _stack2;
} expr_parser;

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

void _output_node(expr_node_t * node);
