#include "expr_parser.h"
#include <stdlib.h>
#include <assert.h>

int get_value(char *varname, expr_value_t * value) {
	assert(varname);
	assert(value);
	if(strcmp(varname, "var_int") == 0) {
		expr_value_set_int(value, 35);
	}
	if(strcmp(varname, "var_double") == 0) {
		expr_value_set_double(value, 3.4);
	}
	if(strcmp(varname, "var_pchar") == 0) {
		expr_value_set_str(value, "hello", strlen("hello"));
	}
	else {
		return -1;
	}
	return 0;
}

int main()
{
	expr_parser * parser = expr_parser_new();
	expr_parser_parse(parser, (char *)"[[a]]==$b.a[5]||\"hello_c\"<$d&&(e>=15.6)||!\'world_c\'");
	//expr_parser_parse(parser, (char*)"a > b && ! e");
	expr_parser_print_tree(parser);
	printf("\nend\n");
	if(0 != parser) {
		int result;
		if( expr_parser_execute(parser, &result, get_value) == 0) {
			
		
			printf("result = %d\n", result);
		}	
		expr_parser_delete(parser);
	}
	return 0;
}
