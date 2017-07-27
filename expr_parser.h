#include "array.h"
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <ctype.h>

/*
 * 语法树节点类型
 */
static const int _TYPE_OPER		= 0;
static const int _TYPE_DATA		= 1;

typedef struct _node_t {
	int type;			// 0-运算符, 1-数据
	union {
		int oper;
		char * data;
	};
	struct _node_t * left;
	struct _node_t * right;
} node_t;

typedef struct _bparser {
	struct _node_t * root;
	array_t _stack1;
	array_t _stack2;
} bparser;

bparser * bparser_new();
void bparser_delete(bparser *parser);
void bparser_reset(bparser *parser);
void bparser_parse(bparser * parser, char *exp_str);

void _output_node(node_t * node);
