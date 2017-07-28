#include "expr_parser.h"
#include <stdlib.h>
#include <assert.h>

int get_value(char *varname, expr_value_t * value) {
	assert(varname);
	assert(value);
	printf("varname = %s\n", varname);
	if(strcmp(varname, "var_int") == 0) {
		expr_value_set_int(value, 35);
	}
	if(strcmp(varname, "var_double") == 0) {
		expr_value_set_double(value, 3.4);
	}
	if(strcmp(varname, "var_pchar") == 0) {
		expr_value_set_str(value, "hello", strlen("hello"));
	}
	else if(strcmp(varname, "b.a[5]") == 0) {
		expr_value_set_str(value, "ab", strlen("ab"));
	}
	else if(strcmp(varname, "hello_c") == 0) {
		expr_value_set_int(value, 36.67777);
	}
	else if(strcmp(varname, "d") == 0) {
		expr_value_set_double(value, 36.677771);
	}
	else if(strcmp(varname, "e") == 0) {
		expr_value_set_double(value, -178);
	}
	else if(strcmp(varname, "world_c") ==0) {
		expr_value_set_int(value, 1);
	}
	else {
		return -1;
	}
	return 0;
}

int main()
{
	expr_parser * parser = expr_parser_new();
	expr_parser_parse(parser, (char *)"[[a]] > $b.a[5]||hello_c<$d&&(e>=-179.4.)||!world_c");
	/*
	expr_parser_parse(parser, (char*)"a > b && ! e");
	expr_parser_parse(parser, (char *)"('hello'-sne \"hell\")");
	*/
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
