#include "array.h"
#include <stdlib.h>
#include <stdio.h>

ARRAY_DEFINE(int, int)
ARRAY_DEFINE(char*, string)

void out_put_int(array_t *arr, int *p_ele, size_t idx)
{
	printf("[%d] = [%d]\n", idx, * p_ele);
}

void out_put_string(array_t *arr, char **p_ele, size_t idx)
{
	printf("[%d] = [%s]\n", idx, *p_ele);
}

void test_array()
{
	array_t str_arr;
	string_array_init(&str_arr);

	printf("empty = %d\n", array_empty(&str_arr));

	int a=3, b=6, c=2;
	char * aa = "hello";
	char * bb = "world";
	char * cc = "bye";
	char * dd = "what";
	string_array_push_back(&str_arr, "hello");
	string_array_push_back(&str_arr, "world");
	string_array_push_back(&str_arr, "bye");
	array_pop_back(&str_arr);
	array_insert(&str_arr, &dd, 1);

	printf("capacity = %d\n", array_capacity(&str_arr));
	printf("size = %d\n", array_size(&str_arr));

	array_insert(&str_arr, &dd, 0);
	array_insert(&str_arr, &cc, array_size(&str_arr));

	printf("capacity = %d\n", array_capacity(&str_arr));
	printf("size = %d\n", array_size(&str_arr));
	printf("element_size = %d\n", array_element_size(&str_arr));
	printf("empty = %d\n", array_empty(&str_arr));

	string_array_foreach(&str_arr, out_put_string);

	char * result = 0;
	array_at(&str_arr, 3, &result);

	printf("at 3: %s\n", result);

	result = 0;
	array_front(&str_arr, &result);
	printf("front: %s\n", result);

	result = 0;
	array_back(&str_arr, &result);
	printf("back : %s\n", result);

	array_erase(&str_arr, 2);
	array_erase(&str_arr, 0);
	array_erase(&str_arr, 2);

	array_shrink(&str_arr);

	
	string_array_foreach(&str_arr, out_put_string);

	
	printf("capacity = %d\n", array_capacity(&str_arr));
	printf("size = %d\n", array_size(&str_arr));
	printf("element_size = %d\n", array_element_size(&str_arr));
	printf("empty = %d\n", array_empty(&str_arr));

	string_array_foreach(&str_arr, out_put_string);

	string_array_foreach_reverse(&str_arr, out_put_string);

	array_clear(&str_arr);

	string_array_foreach(&str_arr, out_put_string);
	string_array_foreach_reverse(&str_arr, out_put_string);
	printf("capacity = %d\n", array_capacity(&str_arr));
	printf("size = %d\n", array_size(&str_arr));
	printf("element_size = %d\n", array_element_size(&str_arr));
	printf("empty = %d\n", array_empty(&str_arr));

	array_shrink(&str_arr);

	printf("capacity = %d\n", array_capacity(&str_arr));
	printf("size = %d\n", array_size(&str_arr));
	printf("element_size = %d\n", array_element_size(&str_arr));
	printf("empty = %d\n", array_empty(&str_arr));
	printf("data = %d\n", str_arr._data);

	array_uinit(&str_arr);
	printf("capacity = %d\n", array_capacity(&str_arr));
	printf("size = %d\n", array_size(&str_arr));
	printf("element_size = %d\n", array_element_size(&str_arr));
	printf("empty = %d\n", array_empty(&str_arr));
	printf("data = %d\n", str_arr._data);

}


int main()
{
	test_array();
	return 0;
}
