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
 * 语法树节点类型
 */
static const int _NODE_OPER		= 0;
static const int _NODE_DATA		= 1;

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

/*
 * 语法树节点
 */
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

/*
 * 解析器
 */
struct expr_parser {
	struct _expr_node_t * root;
	array_t _stack1;
};

ARRAY_DEFINE(opercfg_t, opercfg)
ARRAY_DEFINE(expr_node_t *, node)

static array_t _opercfgs;

static inline opercfg_t _new_opercfg(int oper, char * text, int need_left, int need_right, int priority_l, int priority_r) {
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
	return 0;
}

static int _cmp_oper_len(const void *a, const void *b) {
	const opercfg_t * aa = (const opercfg_t *)a;
	const opercfg_t * bb = (const opercfg_t *)b;
	int alen = aa->text != 0 ? strlen(aa->text) : 0;
	int blen = bb->text != 0 ? strlen(bb->text) : 0;
	return alen > blen ? -1 : (alen < blen ? 1 : 0);
}

static expr_node_t * _new_node(int type) {
	expr_node_t * node = (expr_node_t *) malloc(sizeof(expr_node_t));
	memset(node, 0x00, sizeof(expr_node_t));
	node->type = type;
	return node;
}

static void _free_node(expr_node_t *node) {
	if(0 != node) {
		if(_NODE_DATA == node->type) {
			if(0 != node->data) {
				free(node->data);
			}
		}
		if(0 != node->left) {
			_free_node(node->left);
		}
		if(0 != node->right) {
			_free_node(node->right);
		}

		free(node);
	}
}

static void _output_node(expr_node_t * node) {
	if(0 != node->left) {
		_output_node(node->left);
	}
	if(0 != node->right) {
		_output_node(node->right);
	}

	if(node->type == _NODE_OPER) {
		opercfg_t * cfg = _opercfg_of(node->oper);
		printf("%s, ", cfg->text);
	}
	else {
		printf("%s, ", node->data);
	}
}

static void _clear_stack(array_t * stack) {
	if(stack) {
		size_t i = 0;
		size_t size = array_size(stack);
		for( ;i<size; ++i) {
			expr_node_t * node = 0;
			array_at(stack, i, &node);
			_free_node(node);
		}
		array_clear(stack);
	}
}

expr_parser * expr_parser_new() {
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
		opercfg_array_push_back(&_opercfgs, _new_opercfg(_OPER_BRK_L, (char *)_TEXT_BRK_L, 0, 1, 0, 0));
		opercfg_array_push_back(&_opercfgs, _new_opercfg(_OPER_BRK_R, (char *)_TEXT_BRK_R, 0, 0, 0, 0));
		array_shrink(&_opercfgs);

		qsort(_opercfgs._data, _opercfgs._size, _opercfgs._element_size, _cmp_oper_len);
		
		_opercfgs_inited = 1;
	}

	expr_parser * parser = (expr_parser*) malloc(sizeof(expr_parser));
	if(parser) {
		node_array_init(&(parser->_stack1));
	}

	return parser;
}

void expr_parser_delete(expr_parser *parser) {
	if(parser) {
		_clear_stack(&(parser->_stack1));
		
		if(parser->root) {
			_free_node(parser->root);
		}
		free(parser);
	}
}

expr_node_t * _pick_oper(char * exp_str, int * cursor) {
	if(exp_str[*cursor] == '\0') {
		return 0;
	}
	size_t i=0;
	for(; i< array_size(&_opercfgs); ++i) {
		opercfg_t cfg;
		array_at(&_opercfgs, i, (void *)&cfg); 
		int len = strlen(cfg.text);
		if(strncmp(exp_str + (*cursor), cfg.text, len) == 0) {
			expr_node_t *node = _new_node(_NODE_OPER);
			node->oper = cfg.oper;
			node->left=0;
			node->right=0;
			(*cursor) += len;
			printf("oper: %s end: %d\n", cfg.text, (*cursor));
			return node;
		}
	}

	return 0;
}

static inline  int _is_varname_char(char c) {
	return ( isalpha(c) || isdigit(c) || c == '_' || c == '.' || c == '[' || c == ']');
}

static inline int _is_number_start(char c) {
	return isdigit(c) || c == '.' || c == '-' || c == '+';
}

static inline int _is_number_char(char c) {
	return isdigit(c) || c == '.';
}

static expr_node_t * _pick_data(char *exp_str, int *cursor) 
{
	if(exp_str[*cursor] == '\0') {
		return 0;
	}
	int start = -1;
	int end = -1;
	if(exp_str[*cursor] == '\"') {	/* "字符串开始 */
		start = *cursor;
		end = *cursor+1;
		while(exp_str[end] != 0 && exp_str[end] != '\"')	{ end++;}
		if(exp_str[end] == '\"') end++;
	}
	else if(exp_str[*cursor] == '\'') {	/* '字符串开始 */
		start = *cursor;
		end = *cursor+1;
		while(exp_str[end] != 0 && exp_str[end] != '\'')	{ end++;}
		if(exp_str[end] == '\'') end++;
	}
	else if(strncmp(exp_str+*cursor, "[[", 2) == 0) {	/* [[字符串开始 */
		start = *cursor;
		end = *cursor+2;
		while(exp_str[end] != '\0' && (exp_str[end] != ']' || exp_str[end-1] != ']'))	{ end++;}
		if(exp_str[end] == ']') end++;
	}
	else if(strncmp(exp_str+*cursor, "$", 1) == 0) {	/* $引用变量 */
		start = *cursor;
		end = *cursor+1;
		while(_is_varname_char(exp_str[end])) {end++;}
	}
	else if(_is_number_start(exp_str[*cursor])) {
		start = *cursor;
		end = *cursor + 1;
		while(_is_number_char(exp_str[end])) {end++;}
	}
	else if(_is_varname_char(exp_str[*cursor])) {
		start = *cursor;
		end = *cursor + 1;
		while(_is_varname_char(exp_str[end])) {end++;}
	}
	else {
		return 0;
	}

	char *data_tmp = (char *)malloc(end-start+1);
	memset(data_tmp, 0x00, end-start+1);
	memcpy(data_tmp, exp_str+start, end-start);

	expr_node_t * node = _new_node(_NODE_DATA);
	node->data = data_tmp;
	node->left = 0;
	node->right = 0;

	printf("data:%s end:%d\n", data_tmp, end);

	*cursor = end;
	return node;
}

static void _slip_space(char *exp_str, int *cursor) {
	if(exp_str[*cursor] == '\0') {
		return;
	}
	while(isspace(exp_str[*cursor])) {
		(*cursor)++;
	}
}

static expr_node_t * _get_next_node(char *exp_str, int *cursor) {
	_slip_space(exp_str, cursor);
	expr_node_t * node = _pick_oper(exp_str, cursor);
	if(node) {
		return node;
	}
	_slip_space(exp_str, cursor);
	node = _pick_data(exp_str, cursor);
	return node;
}

static void _get_pre_oper(array_t * stack1, expr_node_t ** pre_oper, size_t *oper_idx) {
	size_t idx = array_size(stack1);
	while(idx > 0) {
		idx --;
		expr_node_t *node = 0;
		array_at(stack1, idx, &node);
		if(node) {
			if(_NODE_OPER == node->type) {
				if(_OPER_BRK_L == node->oper) {
					*pre_oper = node;
					*oper_idx = idx;
					return;
				}
				opercfg_t * cfg = _opercfg_of(node->oper);
				if(cfg->need_right && 0 == node->right) {
					*pre_oper = node;
					*oper_idx = idx;
					return;
				}
			}
		}
	}
	return;
}

static int _deal_brk_r(array_t * stack1, expr_node_t * brk_node) {
	while(1) {
		expr_node_t * pre_oper = 0;
		size_t pre_oper_idx = 0;
		_get_pre_oper(stack1, &pre_oper, &pre_oper_idx);
		if(0 == pre_oper) {
			if(array_size(stack1) > 1) {
				// error
				printf("error at:%d unmatch ^) \n", brk_node->offset);
				return -1;
			}
			return 0;
		}
		if(_OPER_BRK_L == pre_oper->oper) {
			if(array_size(stack1)-pre_oper_idx > 2) {
				// error
				printf("error at:%d more than 1 param in ^()\n", pre_oper->offset);
				return -1;
			}
			array_erase(stack1, pre_oper_idx);
			return 0;
		}

		if(array_size(stack1)-pre_oper_idx<=1) {
			// error
			opercfg_t * cfg = _opercfg_of(pre_oper->oper);
			printf("error at:%d need param after ^%s\n", pre_oper->offset, cfg->text);
			return -1;
		}
		expr_node_t * right = 0;
		array_at(stack1, pre_oper_idx +1, &right);
		array_erase(stack1, pre_oper_idx+1);
		pre_oper->right = right;
		continue;
	}
}

static int _link_left(array_t* stack1, expr_node_t * link_node, size_t idx) {
	if(idx == -1) {
		opercfg_t * cfg = _opercfg_of(link_node->oper);
		if(cfg->need_left) {
			if(array_size(stack1) == 0) {
				// error
				printf("error at:%d need param before ^%s\n", link_node->offset, cfg->text);
				return -1;
			}
			expr_node_t *left = 0;
			array_at(stack1, array_size(stack1)-1, &left);
			if(_NODE_OPER == left->type) {
				opercfg_t * leftcfg = _opercfg_of(left->oper);
				if(leftcfg->need_right && 0 == left->right) {
					// error
					printf("error at:%d need param before ^%s\n", link_node->offset, cfg->text);
					return -1;
				}
			}
			array_erase(stack1, array_size(stack1)-1);
			link_node->left = left;
			return 0;
		}
		return 0;
	}
}

static int _link_right(array_t* stack1, expr_node_t * link_node, size_t idx) {
	opercfg_t * cfg = _opercfg_of(link_node->oper);
	if(cfg->need_right) {
		if(idx == array_size(stack1) -1) {
			// error
			printf("error at:%d need param after ^%s\n", link_node->offset, cfg->text);
			return -1;
		}
		expr_node_t *right = 0;
		array_at(stack1, idx+1, &right);
		link_node->right = right;
		array_erase(stack1, idx+1);
		return 0;
	}
	return 0;
}

static int _deal_oper_node(array_t * stack1, expr_node_t * oper_node) {
	opercfg_t * curcfg = _opercfg_of(oper_node->oper);
	while(1) {
		expr_node_t * pre_oper = 0;
		size_t pre_oper_idx = 0;
		_get_pre_oper(stack1, &pre_oper, &pre_oper_idx);
		if(0 == pre_oper || _OPER_BRK_L == pre_oper->oper) {
			if(_link_left(stack1, oper_node, -1) < 0) {
				return -1;
			}
			node_array_push_back(stack1, oper_node);
			return 0;
		}

		if(!curcfg->need_left) {
			node_array_push_back(stack1, oper_node);
			return 0;
		}
		
		opercfg_t *precfg = _opercfg_of(pre_oper->oper);
		if(precfg->priority_r >= curcfg->priority_r) {
			if(_link_right(stack1, pre_oper, pre_oper_idx) < 0) {
				return -1;
			}
			continue;
		}

		if( _link_left(stack1, oper_node, -1) < 0) {
			return -1;
		}
		node_array_push_back(stack1, oper_node);
		return 0;
	}
}

static int _deal_end(array_t * stack1) {
	while(1) {
		expr_node_t * pre_oper = 0;
		size_t pre_oper_idx = 0;
		_get_pre_oper(stack1, &pre_oper, &pre_oper_idx);
		if(0 == pre_oper) {
			size_t size = array_size(stack1);
			if(size > 1) {
				// error
				printf("error: too many values. \n");
				return -1;
			}
			if(size == 0) {
				// error
				printf("error: no value.\n");
				return -1;
			}
			return 0;
		}
		if(_OPER_BRK_L == pre_oper->oper) {
			// error
			opercfg_t * cfg = _opercfg_of(pre_oper->oper);
			printf("error at:%d unmatch with ^(\n", pre_oper->offset, cfg->text);
			return -1;
		}

		size_t size = array_size(stack1);

		if(size-pre_oper_idx<=1) {
			// error
			opercfg_t * cfg = _opercfg_of(pre_oper->oper);
			printf("error at:%d need param after ^%s\n", pre_oper->offset, cfg->text);
			return -1;
		}
		expr_node_t * right = 0;
		array_at(stack1, pre_oper_idx +1, &right);
		array_erase(stack1, pre_oper_idx+1);
		pre_oper->right = right;
		continue;
	}
}

static expr_node_t * _parse_it(char *exp_str, array_t * stack1) {
	_clear_stack(stack1);

	int cursor = 0;

	while(1) {
		expr_node_t * node = _get_next_node(exp_str, &cursor);
		if(!node) {
			if(exp_str[cursor] != '\0') {
				// error
				printf("error at:%d, unrecognized character:^%c\n", cursor, exp_str[cursor]);
				_clear_stack(stack1);
				return 0;
			}

			if(_deal_end(stack1) < 0) {
				return 0;
			}

			expr_node_t * ret = 0;
			array_at(stack1, 0, &ret);
			return ret;
		}

		if(node->type == _NODE_DATA) {
			node_array_push_back(stack1, node);
			continue;
		}
		if(node->type == _NODE_OPER) {
			if(_OPER_BRK_L == node->oper) {
				node_array_push_back(stack1, node);
				continue;
			}
			if(_OPER_BRK_R == node->oper) {
				if(	_deal_brk_r(stack1, node) < 0) {
					return 0;
				}
				continue;
			}

			if(_deal_oper_node(stack1, node) < 0) {
				return 0;
			}
		}
	}
}

void expr_parser_reset(expr_parser *parser) {
	if(parser) {
		_clear_stack(&parser->_stack1);
		if(parser->root) {
			_free_node(parser->root);
			parser->root = 0;
		}
	}
}

void expr_parser_parse(expr_parser * parser, char *exp_str) {
	if(parser) {
		expr_parser_reset(parser);
		parser->root = _parse_it(exp_str, &parser->_stack1);
	}
}

typedef struct _expr_value {
	int valueType;  // 0- 数字 1-字符串
	union {
		int64_t nValue;
		char *pValue;
	};
} expr_value;

static int _execute_data_node(expr_node_t *node, expr_value * value, expr_value_getter getter) {
	if(strncmp(node->data, "$", 1) == 0) {
		char *data = 0;
		if(getter((char *)node->data+1, &data) < 0) {
			return -1;
		}
		value->valueType = 1;
		value->pValue = data;
		return 0;
	}
	else {
		value->valueType = 1;
		value->pValue = node->data;
		return 0;
	}
}

static int64_t _get_number_value(expr_value * v) {
	int64_t vv = v->nValue;
	if(v->valueType == 1) {
		if(!v->pValue) {
			vv = 0;
		}
		else if(strcmp(v->pValue, "true") == 0) {
			vv = 1;
		}
		else if(strcmp(v->pValue, "false") == 0) {
			vv = 0;
		}
		else {
			vv = atoi(v->pValue); 
		}
	}
	return vv;
}

static int _execute_oper_node(expr_node_t *node, int * result, expr_value_getter getter) {
	opercfg_t * cfg = _opercfg_of(node->oper);
	expr_node_t * left = node->left;
	expr_node_t *right = node->right;
	expr_value val_l, val_r;
	if(cfg->need_left) {
		if(_NODE_OPER == left->type) {
			int lVal = 0;
			if(_execute_oper_node(left, &lVal, getter) < 0) {
				return -1;
			}
			val_l.valueType = 0;
			val_l.nValue = lVal;
		}
		else {
			if(	_execute_data_node(left, &val_l, getter) < 0) {
				return -1;
			}
		} 
	}
	if(cfg->need_right) {
		if(_NODE_OPER == right->type) {
			int rVal = 0;
			if(_execute_oper_node(right, &rVal, getter) < 0) {
				return -1;
			}
			val_r.valueType = 0;
			val_r.nValue = rVal;
		}
		else {
			if(	_execute_data_node(right, &val_r, getter) < 0) {
				return -1;
			}
		} 
	}

	if(node->oper == _OPER_EQ) {
			int64_t lNum = _get_number_value(&val_l);
			int64_t rNum = _get_number_value(&val_r);
			*result = lNum == rNum;
	}
	else if(node->oper == _OPER_NE) {
			int64_t lNum = _get_number_value(&val_l);
			int64_t rNum = _get_number_value(&val_r);
			*result = lNum != rNum;
	}
	else if(node->oper == _OPER_LT) {
			int64_t lNum = _get_number_value(&val_l);
			int64_t rNum = _get_number_value(&val_r);
			*result = lNum < rNum;
	}
	else if(node->oper == _OPER_LE) {
			int64_t lNum = _get_number_value(&val_l);
			int64_t rNum = _get_number_value(&val_r);
			*result = lNum <= rNum;
	}
	else if(node->oper == _OPER_GT) {
			int64_t lNum = _get_number_value(&val_l);
			int64_t rNum = _get_number_value(&val_r);
			*result = lNum > rNum;
	}
	else if(node->oper == _OPER_GE) {
			int64_t lNum = _get_number_value(&val_l);
			int64_t rNum = _get_number_value(&val_r);
			*result = lNum >= rNum;
	}
	else if(node->oper == _OPER_SE) {
			if(val_l.valueType != 1) return -1;
			if(val_r.valueType != 1) return -1;
			*result = strcmp(val_l.pValue, val_r.pValue) == 0;
	}
	else if(node->oper == _OPER_SNE) {
			if(val_l.valueType != 1) return -1;
			if(val_r.valueType != 1) return -1;
			*result = strcmp(val_l.pValue, val_r.pValue) != 0;
	}
	else if(node->oper == _OPER_CE) {
			if(val_l.valueType != 1) return -1;
			if(val_r.valueType != 1) return -1;
			*result = strcmp(val_l.pValue, val_r.pValue) == 0;
	}
	else if(node->oper == _OPER_CNE) {
			if(val_l.valueType != 1) return -1;
			if(val_r.valueType != 1) return -1;
			*result = strcmp(val_l.pValue, val_r.pValue) != 0;
	}
	else if(node->oper == _OPER_AND) {
			int64_t lNum = _get_number_value(&val_l);
			int64_t rNum = _get_number_value(&val_r);
			*result = lNum && rNum;
	}
	else if(node->oper == _OPER_OR) {
			int64_t lNum = _get_number_value(&val_l);
			int64_t rNum = _get_number_value(&val_r);
			*result = lNum || rNum;
	}
	else if(node->oper == _OPER_NOT) {
			int64_t rNum = _get_number_value(&val_r);
			*result = ! rNum;
	}
	else {
		return -1;
	}

	return 0;
}

int expr_parser_execute(expr_parser *parser, int *result, expr_value_getter getter) {
	if(!parser->root) return -1;
	if(parser->root->type != _NODE_OPER) return -1;
	return _execute_oper_node(parser->root, result, getter);
}
