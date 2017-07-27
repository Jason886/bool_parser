#include "array.h"
#include "expr_parser.h"
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <ctype.h>

/*
* 运算符枚举
*/
static const int _OPER_EQ		= 0;	// ==	, 数字相等
static const int _OPER_NE		= 1;	// !=	, 数字不等
static const int _OPER_LT		= 2;	// <	, 数字小于
static const int _OPER_LE		= 3;	// >=	, 数字小于等于
static const int _OPER_GT		= 4;	// >	, 数字大于
static const int _OPER_GE		= 5;	// >=	, 数字大于等于
static const int _OPER_SE		= 6;	// -se	, 字符串相等
static const int _OPER_SNE		= 7;	// -sne	, 字符串不等
static const int _OPER_CE		= 8;	// -ce	, 字符串相等（忽略大小写）
static const int _OPER_CNE		= 9;	// -cne	, 字符串不等（忽略大小写）
static const int _OPER_AND		= 10;	// &&	, 逻辑与
static const int _OPER_OR		= 11;	// ||	, 逻辑或
static const int _OPER_NOT		= 12;	// !	, 逻辑非
static const int _OPER_BRK_L	= 13;	// (	, 左括号
static const int _OPER_BRK_R	= 14;	// )	, 右括号

/*
* 运算符
*/
static const char * _TEXT_EQ	= "==";
static const char * _TEXT_NE	= "!=";
static const char * _TEXT_LT	= "<";
static const char * _TEXT_LE	= "<=";
static const char * _TEXT_GT	= ">";
static const char * _TEXT_GE	= ">=";
static const char * _TEXT_SE	= "-se";
static const char * _TEXT_SNE	= "-sne";
static const char * _TEXT_CE	= "-ce";
static const char * _TEXT_CNE	= "-cne";
static const char * _TEXT_AND	= "&&";
static const char * _TEXT_OR	= "||";
static const char * _TEXT_NOT	= "!";
static const char * _TEXT_BRK_L	= "(";
static const char * _TEXT_BRK_R	= ")";


/*
* 运算符配置
*/
typedef struct _opercfg_t {
	int oper;			// 运算符枚举
	char * text;		// 运算符文本
	int need_left;		// 需要左参数
	int need_right;		// 需要右参数
	int priority_l;		// 在左边时优先级
	int priority_r;		// 在右边时优先级
} opercfg_t ;


ARRAY_DEFINE(opercfg_t, opercfg)
ARRAY_DEFINE(node_t *, node)

static array_t _opercfgs;

static opercfg_t _new_opercfg(int oper, char * text, int need_left, int need_right, int priority_l, int priority_r) {
	opercfg_t cfg = {oper, text, need_left, need_right, priority_l, priority_r};
	return cfg;
}

static opercfg_t * _opercfg_of(int oper) {
	size_t size = array_size(&_opercfgs);
	size_t i = 0;
	for( ; i<size; ++i) {
		opercfg_t *cfg = 0;
		array_ref_at(&_opercfgs, i, (void **)&cfg);
		if(oper == cfg->oper) {
			return cfg;
		}
	}
	return NULL;
}

static int _cmp_oper_len(const void *a, const void *b) {
	const opercfg_t * aa = (const opercfg_t *)a;
	const opercfg_t * bb = (const opercfg_t *)b;
	int alen = aa->text != 0 ? strlen(aa->text) : 0;
	int blen = bb->text != 0 ? strlen(bb->text) : 0;
	return alen > blen ? -1 : (alen < blen ? 1 : 0);
}

static node_t * _new_node(int type) 
{
	node_t * node = (node_t *) malloc(sizeof(node_t));
	memset(node, 0x00, sizeof(node_t));
	node->type = type;
	return node;
}

static void _free_node(node_t *node) 
{
	if(NULL != node) 
	{
		if(_TYPE_DATA == node->type) 
		{
			if(NULL != node->data) 
			{
				free(node->data);
			}
		}
		if(NULL != node->left) 
		{
			_free_node(node->left);
		}
		if(NULL != node->right) 
		{
			_free_node(node->right);
		}

		free(node);
	}
}

void _output_node(node_t * node)
{
	if(NULL != node->left)
	{
		_output_node(node->left);
	}
	if(NULL != node->right)
	{
		_output_node(node->right);
	}

	if(node->type == _TYPE_OPER)
	{
		opercfg_t * cfg = _opercfg_of(node->oper);
		printf("%s, ", cfg->text);
	}
	else
	{
		printf("%s, ", node->data);
	}
}

static void _clear_stack(array_t * stack)
{
	if(stack) {
		size_t i = 0;
		size_t size = array_size(stack);
		for( ;i<size; ++i) {
			node_t * node = 0;
			array_at(stack, i, &node);
			_free_node(node);
		}
		array_clear(stack);
	}
}

bparser * bparser_new() 
{
	static int _opercfgs_inited = 0;
	if(!_opercfgs_inited) {
		opercfg_array_init(&_opercfgs);
		opercfg_array_push_back(&_opercfgs, _new_opercfg(_OPER_EQ, (char *)_TEXT_EQ, 1, 1, 4, 4));
		opercfg_array_push_back(&_opercfgs, _new_opercfg(_OPER_NE, (char *)_TEXT_NE, 1, 1, 4, 4));
		opercfg_array_push_back(&_opercfgs, _new_opercfg(_OPER_LT, (char *)_TEXT_LT, 1, 1, 4, 4));
		opercfg_array_push_back(&_opercfgs, _new_opercfg(_OPER_LE, (char *)_TEXT_LE, 1, 1, 4, 4));
		opercfg_array_push_back(&_opercfgs, _new_opercfg(_OPER_GT, (char *)_TEXT_GT, 1, 1, 4, 4));
		opercfg_array_push_back(&_opercfgs, _new_opercfg(_OPER_GE, (char *)_TEXT_GE, 1, 1, 4, 4));
		opercfg_array_push_back(&_opercfgs, _new_opercfg(_OPER_SE, (char *)_TEXT_SE, 1, 1, 4, 4));
		opercfg_array_push_back(&_opercfgs, _new_opercfg(_OPER_SNE, (char *)_TEXT_SNE, 1, 1, 4, 4));
		opercfg_array_push_back(&_opercfgs, _new_opercfg(_OPER_CE, (char *)_TEXT_CE, 1, 1, 4, 4));
		opercfg_array_push_back(&_opercfgs, _new_opercfg(_OPER_CNE, (char *)_TEXT_CNE, 1, 1, 4, 4));
		opercfg_array_push_back(&_opercfgs, _new_opercfg(_OPER_AND, (char *)_TEXT_AND, 1, 1, 3, 3));
		opercfg_array_push_back(&_opercfgs, _new_opercfg(_OPER_OR, (char *)_TEXT_OR, 1, 1, 2, 2));
		opercfg_array_push_back(&_opercfgs, _new_opercfg(_OPER_NOT, (char *)_TEXT_NOT, 0, 1, 1, 7));
		opercfg_array_push_back(&_opercfgs, _new_opercfg(_OPER_BRK_L, (char *)_TEXT_BRK_L, 0, 0, 0, 8));
		opercfg_array_push_back(&_opercfgs, _new_opercfg(_OPER_BRK_R, (char *)_TEXT_BRK_R, 0, 0, 8, 0));
		array_shrink(&_opercfgs);

		qsort(_opercfgs._data, _opercfgs._size, _opercfgs._element_size, _cmp_oper_len);
		
		_opercfgs_inited = 1;
	}

	bparser * parser = malloc(sizeof(bparser));
	if(parser) {
		node_array_init(&(parser->_stack1));
		node_array_init(&(parser->_stack2));
	}

	return parser;
}

void bparser_delete(bparser *parser) {
	if(parser) {
		_clear_stack(&(parser->_stack1));
		_clear_stack(&(parser->_stack2));
		
		if(parser->root) {
			_free_node(parser->root);
		}
		free(parser);
	}
}

node_t * _pop_left_oper(array_t *stack1, array_t *stack2)
{
	while(1)
	{
		if(array_size(stack1) == 0)
		{
			return NULL;
		}
		node_t * node;
		array_back(stack1, &node);
		array_pop_back(stack1);
		if(node->type == _TYPE_DATA)
		{
			node_array_push_back(stack2, node);
			continue;
		}

		opercfg_t *cfg = _opercfg_of(node->oper);
		if(!cfg->need_right || NULL != node->right)
		{
			node_array_push_back(stack2, node);
			continue;
		}

		return node;
	}
}

node_t * _pick_oper(char * exp_str, int * cursor) 
{
	if(exp_str[*cursor] == '\0')
	{
		return NULL;
	}
	size_t i=0;
	for(; i< array_size(&_opercfgs); ++i)
	{
		opercfg_t cfg;
		array_at(&_opercfgs, i, (void *)&cfg); 
		int len = strlen(cfg.text);
		//printf("cfg text %s\n", cfg.text);
		//printf("exp_str %s\n", exp_str + (*cursor));
		//printf("len %d\n", len);
		if(strncmp(exp_str + (*cursor), cfg.text, len) == 0) 
		{
			node_t *node = _new_node(_TYPE_OPER);
			node->oper = cfg.oper;
			node->left=NULL;
			node->right=NULL;
			(*cursor) += len;
			printf("oper: %s\n", cfg.text);
			return node;
		}
	}

	return NULL;
}

node_t * _pick_data(char *exp_str, int *cursor) 
{
	if(exp_str[*cursor] == '\0')
	{
		return NULL;
	}
	int start = -1;
	int end = -1;
	if(exp_str[*cursor] == '\"') 
	{
		start = *cursor +1;
		end = *cursor+1;
		while(exp_str[end] != 0 && (exp_str[end] != '\"' || exp_str[end-1] == '\\'))	{ end++;}
	}
	else if(strncmp(exp_str+*cursor, "id:", 3) == 0) 
	{
		start = *cursor;
		end = *cursor+3;
		while(exp_str[end] != 0 && (isalpha(exp_str[end]) || isdigit(exp_str[end]) || exp_str[end] == '_' || exp_str[end] == '.')) {end++;}
	}
	else if(!isspace(exp_str[*cursor]))
	{
		start = *cursor;
		end = *cursor + 1;
		while(exp_str[end] != 0 && !isspace(exp_str[end])) {end++;}
	}
	else {
		return NULL;
	}

	char *data_tmp = (char *)malloc(end-start+1);
	memset(data_tmp, 0x00, end-start+1);
	memcpy(data_tmp, exp_str+start, end-start);

	node_t * node = _new_node(_TYPE_DATA);
	node->data = data_tmp;
	node->left = NULL;
	node->right = NULL;

	printf("data:%s\n", data_tmp);

	*cursor = end;
	return node;
}

void _slip_space(char *exp_str, int *cursor) 
{
	if(exp_str[*cursor] == '\0')
	{
		return;
	}
	while(isspace(exp_str[*cursor])) 
	{
		(*cursor)++;
	}
}

node_t * _get_next_node(char *exp_str, int *cursor)
{
	_slip_space(exp_str, cursor);
	node_t * node = _pick_oper(exp_str, cursor);
	if(NULL != node)
	{
		return node;
	}
	_slip_space(exp_str, cursor);
	node = _pick_data(exp_str, cursor);
	return node;
}


node_t * parse(char *exp_str, array_t * stack1, array_t * stack2)
{
	_clear_stack(stack1);
	_clear_stack(stack2);

	int cursor = 0;

	while(1)
	{
		node_t * node = _get_next_node(exp_str, &cursor);
		if(NULL == node)
		{
			if(exp_str[cursor] != '\0')
			{
				// error
				_clear_stack(stack1);
				_clear_stack(stack2);
				return NULL;
			}

			while(1)
			{
				node_t * left_oper = _pop_left_oper(stack1, stack2);
				if(NULL == left_oper)
				{
					while(!array_empty(stack2))
					{
						node_t * node;
						array_back(stack2, &node);
						node_array_push_back(stack1, node);
						array_pop_back(stack2);
					}

					if(array_size(stack1) > 1)
					{
						// error
						_clear_stack(stack1);
						_clear_stack(stack2);
						return NULL;
					}
					
					node_t *ret;
					array_back(stack1, &ret);
					array_pop_back(stack1);

					return ret;
				}

				if(array_empty(stack2))
				{
					// error
					_clear_stack(stack1);
					_clear_stack(stack2);
					_free_node(left_oper);
					return NULL;
				}
				array_back(stack2, &left_oper->right);
				array_pop_back(stack2);
				node_array_push_back(stack2, left_oper);
			}
		}

		if(node->type == _TYPE_DATA)
		{
			node_array_push_back(stack1, node);
			continue;
		}

		if(node->type == _TYPE_OPER)
		{
			if(node->oper == _OPER_BRK_L)
			{
				node_array_push_back(stack1, node);
				continue;
			}

			if(node->oper == _OPER_BRK_R)
			{
				node_t * data;
				array_back(stack1, &data);
				array_pop_back(stack1);

				if(data->type != _TYPE_DATA)
				{
					opercfg_t * cfg = _opercfg_of(data->oper);
					if(cfg->need_right && data->right == NULL)
					{
						// error
						_clear_stack(stack1);
						_clear_stack(stack2);
						_free_node(data);
						_free_node(node);
						return NULL;
					}
				}
				
				node_t *left_bracket;
				array_back(stack1, &left_bracket);
				array_pop_back(stack1);

				if(left_bracket->oper != _OPER_BRK_L)
				{
					// error
					_clear_stack(stack1);
					_clear_stack(stack2);
					_free_node(data);
					_free_node(left_bracket);
					free(node);
					return NULL;
				}

				_free_node(left_bracket);
				_free_node(node);
				node_array_push_back(stack1, data);
				continue;
			}

			while(1)
			{
				//printf("_pop_left_oper\n");
				node_t * left_oper = _pop_left_oper(stack1, stack2);
				if(NULL == left_oper)
				{
					opercfg_t * cfg = _opercfg_of(node->oper);
					if(NULL == cfg)
					{
						//printf("NULL == cfg\n");
					}
					if(cfg->need_left)
					{
						if(array_empty(stack2))
						{
							// error
							_clear_stack(stack1);
							_clear_stack(stack2);
							_free_node(node);
							return NULL;
						}

						while(!array_empty(stack2))
						{
							node_t * node;
							array_back(stack2, &node);
							node_array_push_back(stack1, node);
							array_pop_back(stack2);
						}
						array_back(stack1, &node->left);
						array_pop_back(stack1);
					}
					else
					{
						while(!array_empty(stack2))
						{
							node_t * node;
							array_back(stack2, &node);
							node_array_push_back(stack1, node);
							array_pop_back(stack2);
						}
					}
					node_array_push_back(stack1, node);
					break;
				}

				opercfg_t * cfg = _opercfg_of(node->oper);
				if(!cfg->need_left)
				{
					node_array_push_back(stack1, left_oper);
					while(!array_empty(stack2))
					{
						node_t * node;
						array_back(stack2, &node);
						node_array_push_back(stack1, node);
						array_pop_back(stack2);
					}
					node_array_push_back(stack1, node);
					break;
				}

				opercfg_t * left_cfg = _opercfg_of(left_oper->oper);
				if(left_cfg->priority_r < cfg->priority_l)
				{
					node_array_push_back(stack1, left_oper);
					if(array_empty(stack2))
					{
						// error
						_clear_stack(stack1);
						_clear_stack(stack2);
						_free_node(node);
						return NULL;
					}
					while(!array_empty(stack2))
					{
						node_t * node;
						array_back(stack2, &node);
						node_array_push_back(stack1, node);
						array_pop_back(stack2);
					}
					array_back(stack1, &node->left);
					array_pop_back(stack1);
					node_array_push_back(stack1, node);

					break;
				}

				if(array_empty(stack2))
				{
					// error
					_clear_stack(stack1);
					_clear_stack(stack2);
					_free_node(node);
					_free_node(left_oper);
					return NULL;
				}
				array_back(stack2, &left_oper->right);
				array_pop_back(stack2);
				node_array_push_back(stack2, left_oper);
			}
		}
	}
}

void bparser_reset(bparser *parser) {
	if(parser) {
		_clear_stack(&parser->_stack1);
		_clear_stack(&parser->_stack2);
		if(parser->root) {
			_free_node(parser->root);
			parser->root = 0;
		}
	}
}

void bparser_parse(bparser * parser, char *exp_str) {
	if(parser) {
		bparser_reset(parser);
		parser->root = parse(exp_str, &parser->_stack1, &parser->_stack2);
	}
}

