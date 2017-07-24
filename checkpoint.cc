#include <vector>
#include <stdlib.h>

namespace checkpoint 
{


/*
* 操作符枚举
*/
const int OPER_EQ	= 0;	// ==	, 数字相等
const int OPER_NE	= 1;	// !=	, 数字不等
const int OPER_LT	= 2;	// <	, 数字小于
const int OPER_LE	= 3;	// >=	, 数字小于等于
const int OPER_GT	= 4;	// >	, 数字大于
const int OPER_GE	= 5;	// >=	, 数字大于等于
const int OPER_SE	= 6;	// -se	, 字符串相等
const int OPER_SNE	= 7;	// -sne	, 字符串不等
const int OPER_CE	= 8;	// -ce	, 字符串相等（忽略大小写）
const int OPER_CNE	= 9;	// -cne	, 字符串不等（忽略大小写）
const int OPER_AND	= 10;	// &&	, 逻辑与
const int OPER_OR	= 11;	// ||	, 逻辑或
const int OPER_NOT	= 12;	// !	, 逻辑非
const int OPER_BRACKET_L = 13;
const int OPER_BRACKET_R = 14;

/*
* 操作符字符串
*/
const char * TEXT_EQ	= "==";
const char * TEXT_NE	= "!=";
const char * TEXT_LT	= "<";
const char * TEXT_LE	= "<=";
const char * TEXT_GT	= ">";
const char * TEXT_GE	= ">=";
const char * TEXT_SE	= "-se";
const char * TEXT_SNE	= "-sne";
const char * TEXT_CE	= "-ce";
const char * TEXT_CNE	= "-cne";
const char * TEXT_AND	= "&&";
const char * TEXT_OR	= "||";
const char * TEXT_NOT	= "!";
const char * TEXT_BRACKET_L	= "(";
const char * TEXT_BRACKET_R	= ")";

/*
* 操作符配置
*/
typedef struct _opercfg_t 
{
	int oper;
	char * text;
	bool need_left;
	bool need_right;
	int priority_l;	// 在左边时优先级
	int priority_r;	// 在右边时优先级

	_opercfg_t(int oper, char * text, bool need_left, bool need_right, int priority_l, int priority_r)
	{
		this->oper = oper;
		this->text = text;
		this->need_left = need_left;
		this->need_right = need_right;
		this->priority_l = priority_l;
		this->priority_r = priority_r;
	}
} opercfg_t;

static std::vector<opercfg_t> _oper_cfgs;

typedef enum _nodetype
{
	TYPE_OPER = 0,
	TYPE_DATA = 1,
} nodetype;

typedef struct _node_t 
{
	nodetype type;	// 0-操作符, 1-数据
	union {
		int oper;
		char * data;
	};
	struct _node_t * left;
	struct _node_t * right;
} node_t;

static std::vector<node_t *> _stack1;
static std::vector<node_t *> _stack2;

opercfg_t * _get_oper_cfg(int oper) 
{
	for(int i=0; i< _oper_cfgs.size(); ++i)	
	{
		if(oper == _oper_cfgs[i].oper) 
		{
			return &_oper_cfgs[i];
		}
	}
	return NULL;
}

bool _compare_oper_len(opercfg_t &a, opercfg_t &b) 
{
	int alen = a.text ? strlen(a.text) : 0;
	int blen = b.text ? strlen(b.text) : 0;
	return alen > blen;
}

static void _init_oper_cfgs() 
{
	_oper_cfgs.push_back(opercfg_t(OPER_EQ, (char *)TEXT_EQ, true, true, 4, 4));
	_oper_cfgs.push_back(opercfg_t(OPER_NE, (char *)TEXT_NE, true, true, 4, 4));
	_oper_cfgs.push_back(opercfg_t(OPER_LT, (char *)TEXT_LT, true, true, 4, 4));
	_oper_cfgs.push_back(opercfg_t(OPER_LE, (char *)TEXT_LE, true, true, 4, 4));
	_oper_cfgs.push_back(opercfg_t(OPER_GT, (char *)TEXT_GT, true, true, 4, 4));
	_oper_cfgs.push_back(opercfg_t(OPER_GE, (char *)TEXT_GE, true, true, 4, 4));
	_oper_cfgs.push_back(opercfg_t(OPER_SE, (char *)TEXT_SE, true, true, 4, 4));
	_oper_cfgs.push_back(opercfg_t(OPER_SNE, (char *)TEXT_SNE, true, true, 4, 4));
	_oper_cfgs.push_back(opercfg_t(OPER_CE, (char *)TEXT_CE, true, true, 4, 4));
	_oper_cfgs.push_back(opercfg_t(OPER_CNE, (char *)TEXT_CNE, true, true, 4, 4));
	_oper_cfgs.push_back(opercfg_t(OPER_AND, (char *)TEXT_AND, true, true, 3, 3));
	_oper_cfgs.push_back(opercfg_t(OPER_OR, (char *)TEXT_OR, true, true, 2, 2));
	_oper_cfgs.push_back(opercfg_t(OPER_NOT, (char *)TEXT_NOT, false, true, 1, 7));
	_oper_cfgs.push_back(opercfg_t(OPER_BRACKET_L, (char *)TEXT_BRACKET_L, false, false, 0, 8));
	_oper_cfgs.push_back(opercfg_t(OPER_BRACKET_R, (char *)TEXT_BRACKET_R, false, false, 8, 0));

	std::sort(_oper_cfgs.begin(), _oper_cfgs.end(), _compare_oper_len);
}

node_t * _new_node(nodetype type) 
{
	node_t * node = (node_t *) malloc(sizeof(node_t));
	memset(node, 0x00, sizeof(node_t));
	node->type = type;
	return node;
}

void _free_node(node_t *node) 
{
	if(NULL != node) 
	{
		if(TYPE_DATA == node->type) 
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

	if(node->type == TYPE_OPER)
	{
		opercfg_t * cfg = _get_oper_cfg(node->oper);
		printf("%s, ", cfg->text);
	}
	else
	{
		printf("%s, ", node->data);
	}
}

void _clear_stack1()
{
	for(int i=0; i<_stack1.size(); ++i)
	{
		_free_node(_stack1[i]);
	}
	_stack1.clear();
}

void _clear_stack2()
{
	for(int i=0; i<_stack2.size(); ++i)
	{
		_free_node(_stack2[i]);
	}
	_stack2.clear();
}

node_t * _pop_left_oper()
{
	while(true)
	{
		if(_stack1.size() == 0)
		{
			return NULL;
		}
		node_t * node = _stack1.back();
		_stack1.pop_back();
		if(node->type == TYPE_DATA)
		{
			_stack2.push_back(node);
			continue;
		}

		opercfg_t *cfg = _get_oper_cfg(node->oper);
		if(!cfg->need_right || NULL != node->right)
		{
			_stack2.push_back(node);
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
	for(int i=0; i < _oper_cfgs.size(); ++i)
	{
		opercfg_t cfg = _oper_cfgs[i];
		int len = strlen(cfg.text);
		//printf("cfg text %s\n", cfg.text);
		//printf("exp_str %s\n", exp_str + (*cursor));
		//printf("len %d\n", len);
		if(strncmp(exp_str + (*cursor), cfg.text, len) == 0) 
		{
			node_t *node = _new_node(TYPE_OPER);
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
		while(exp_str[end] != 0 && (isalpha(exp_str[end]) || isnumber(exp_str[end]) || exp_str[end] == '_' || exp_str[end] == '.')) {end++;}
	}
	else if(!isspace(exp_str[*cursor]))
	{
		start = *cursor;
		end = *cursor + 1;
		while(!isspace(exp_str[end])) {end++;}
	}
	else {
		return NULL;
	}

	char *data_tmp = (char *)malloc(end-start+1);
	memset(data_tmp, 0x00, end-start+1);
	memcpy(data_tmp, exp_str+start, end-start);

	node_t * node = _new_node(TYPE_DATA);
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

node_t * parse(char *exp_str)
{
	_clear_stack1();
	_clear_stack2();

	int cursor = 0;

	while(true)
	{
		node_t * node = _get_next_node(exp_str, &cursor);
		if(NULL == node)
		{
			if(exp_str[cursor] != '\0')
			{
				// error
				_clear_stack1();
				_clear_stack2();
				return NULL;
			}

			while(true)
			{
				//printf("_pop_left_oper\n");
				node_t * left_oper = _pop_left_oper();
				if(NULL == left_oper)
				{
					while(!_stack2.empty()) 
					{
						_stack1.push_back(_stack2.back());
						_stack2.pop_back();
					}

					if(_stack1.size() > 1)
					{
						// error
						_clear_stack1();
						_clear_stack2();
						return NULL;
					}

					node_t * ret = _stack1.back();
					_stack1.pop_back();
					return ret;
				}

				if(_stack2.empty())
				{
					// error
					_clear_stack1();
					_clear_stack2();
					_free_node(left_oper);
					return NULL;
				}
				left_oper->right = _stack2.back();
				_stack2.pop_back();
				_stack2.push_back(left_oper);
			}
		}

		if(node->type == TYPE_DATA)
		{
			_stack1.push_back(node);
			continue;
		}

		if(node->type == TYPE_OPER)
		{
			if(node->oper == OPER_BRACKET_L)
			{
				_stack1.push_back(node);
				continue;
			}

			if(node->oper == OPER_BRACKET_R)
			{
				node_t * data = _stack1.back();
				_stack1.pop_back();
				if(data->type != TYPE_DATA)
				{
					opercfg_t * cfg = _get_oper_cfg(data->oper);
					if(cfg->need_right && data->right == NULL)
					{
						// error
						_clear_stack1();
						_clear_stack2();
						_free_node(data);
						_free_node(node);
						return NULL;
					}
				}

				node_t * left_bracket = _stack1.back();
				_stack1.pop_back();
				if(left_bracket->oper != OPER_BRACKET_L)
				{
					// error
					_clear_stack1();
					_clear_stack2();
					_free_node(data);
					_free_node(left_bracket);
					free(node);
					return NULL;
				}

				_free_node(left_bracket);
				_free_node(node);
				_stack1.push_back(data);
				continue;
			}

			while(true)
			{
				//printf("_pop_left_oper\n");
				node_t * left_oper = _pop_left_oper();
				if(NULL == left_oper)
				{
					opercfg_t * cfg = _get_oper_cfg(node->oper);
					if(NULL == cfg)
					{
						//printf("NULL == cfg\n");
					}
					if(cfg->need_left)
					{
						if(_stack2.empty())
						{
							// error
							_clear_stack1();
							_clear_stack2();
							_free_node(node);
							return NULL;
						}
						while(!_stack2.empty()) 
						{
							_stack1.push_back(_stack2.back());
							_stack2.pop_back();
						}
						node->left = _stack1.back();
						_stack1.pop_back();
					}
					else
					{
						while(!_stack2.empty()) 
						{
							_stack1.push_back(_stack2.back());
							_stack2.pop_back();
						}
					}
					_stack1.push_back(node);
					break;
				}

				//printf("left_oper != NULL\n");

				opercfg_t * cfg = _get_oper_cfg(node->oper);
				if(!cfg->need_left)
				{
					_stack1.push_back(left_oper);
					while(!_stack2.empty()) 
					{
						_stack1.push_back(_stack2.back());
						_stack2.pop_back();
					}
					_stack1.push_back(node);
					break;
				}

				opercfg_t * left_cfg = _get_oper_cfg(left_oper->oper);
				if(left_cfg->priority_r < cfg->priority_l)
				{
					_stack1.push_back(left_oper);
					if (_stack2.empty())
					{
						// error
						_clear_stack1();
						_clear_stack2();
						_free_node(node);
						return NULL;
					}
					while(!_stack2.empty()) 
					{
						_stack1.push_back(_stack2.back());
						_stack2.pop_back();
					}
					node->left = _stack1.back();
					_stack1.pop_back();
					_stack1.push_back(node);
					break;
				}

				if(_stack2.empty())
				{
					// error
					_clear_stack1();
					_clear_stack2();
					_free_node(node);
					_free_node(left_oper);
					return NULL;
				}
				left_oper->right = _stack2.back();
				_stack2.pop_back();
				_stack2.push_back(left_oper);
			}
		}
	}
}

}

int main()
{
	checkpoint::_init_oper_cfgs();
	checkpoint::node_t * node = checkpoint::parse((char *)"a == b || c < d && e >= f || ! c");
	if(node == NULL)
	{
		printf("NULL\n");
	}
	checkpoint::_output_node(node);
	printf("\nend\n");
	//checkpoint::_free_node(node);
	return 0;
}