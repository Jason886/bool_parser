#define Define_Array(element_type) \
typedef struct _##element_type_array_t \
{ \
	element_type * _elements; \
	size_t _size; \
	size_t _capacity; \
} ##element_array_t;

