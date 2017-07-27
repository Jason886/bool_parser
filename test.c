#include "expr_parser.h"

int main()
{
	bparser * parser = bparser_new();
	bparser_parse(parser, (char *)"a == b || c < d && e >= f || ! c");
	//bparser_parse(parser, (char*)"a == b");
	node_t *node = parser->root;
	if(node == NULL)
	{
		printf("NULL\n");
		return;
	}
	_output_node(node);
	printf("\nend\n");
	bparser_delete(parser);
	return 0;
}
