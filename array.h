/**
 * author: jason886
 * link: https://github.com/Jason886/expr_parser.git
 *
 */
#ifndef _ARRAY_H_
#define _ARRAY_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*array_element_dealloc)(void *p_ele);

typedef struct _array_t {
	void * _data;
	size_t _size;
	size_t _element_size;
	size_t _capacity;
	array_element_dealloc _element_dealloc;
} array_t;

typedef void (*array_foreach_callback)(array_t *arr, void *p_ele, size_t idx);

int array_init(array_t * arr, size_t element_size);
void array_uinit(array_t * arr);
size_t array_capacity(array_t * arr);
size_t array_size(array_t * arr);
size_t array_element_size(array_t *arr);
int array_empty(array_t * arr);
void array_shrink(array_t * arr);
int array_push_back(array_t *arr, void * p_ele);
int array_insert(array_t *arr, void * p_ele, size_t idx);
void array_pop_back(array_t *arr);
int array_at(array_t *arr, size_t idx, void * p_ele);
int array_ref_at(array_t *arr, size_t idx, void **pp_ele);
int array_front(array_t *arr, void * p_ele);
int array_back(array_t * arr, void * p_ele);
void array_erase(array_t *arr, size_t idx);
void array_clear(array_t *arr);
void array_foreach(array_t *arr, array_foreach_callback cb);
void array_foreach_reverse(array_t *arr, array_foreach_callback cb);
/*
#define arr_init(T, arr) array_init(arr, sizeof(T))
#define arr_uinit(arr) array_uinit(arr)
#define arr_cap(arr) array_capacity(arr)
#define arr_n(arr) array_size(arr)
#define arr_ele_size(arr) array_element_size(arr)
#define arr_empty(arr) array_empty(arr)
#define arr_shrk(arr) array_shrink(arr)
#define arr_append(arr, e) \
(T ee = (T)(e), array_push_back(arr, &ee))
#define arr_insert(arr, idx, e) \
(T ee = (T)(e), array_insert(arr, &ee, idx))
#define arr_append_r(arr, pe) array_push_back(arr, pe)
#define arr_insert_r(arr, idx, pe) array_insert(arr, pe, idx)
#define arr_rm_last(arr) array_pop_back(arr)
#define arr_get(T, arr, idx) \
(T e, array_at(arr, idx, &e), &e)
#define arr_get_ref(T, arr, idx) \
(T *pe =0, array_ref_at(arr, idx, &pe), pe)
#define arr_set(arr, idx, e) \
{} \
#define arr_last(T, arr) \
(T e, array_back(arr, &e), e)
#define arr_last_ref(T, arr) \
{} \
#define arr_remove(arr, idx) array_erase(arr, idx)
#define arr_clear(arr) array_clear(arr, idx)
#define arr_for(arr, cb) array_foreach(arr, (array_foreach_callback)(cb))
#define arr_rfor(arr, cb) array_foreach_reverse(arr, (array_foreach_callback)(cb));
*/

#define ARRAY_DEFINE(T, pre) \
static int pre##_array_init(array_t *arr) { \
	return array_init(arr, sizeof(T)); \
} \
static int pre##_array_push_back(array_t *arr, T e) { \
	return array_push_back(arr, &e); \
} \
static int pre##_array_push_back_ref(array_t *arr, T*e) { \
	return array_push_back(arr, e); \
} \
static int pre##_array_insert(array_t *arr, T e, size_t idx) { \
	return array_insert(arr, &e, idx); \
} \
typedef void (* pre##_array_foreach_callback)(array_t *arr, T *p_ele, size_t idx);\
typedef pre##_array_foreach_callback pre##_arr_for_cb; \
static void pre##_array_foreach(array_t * arr, pre##_array_foreach_callback cb) { \
	array_foreach(arr, (array_foreach_callback) cb); \
} \
static void pre##_array_foreach_reverse(array_t *arr, pre##_array_foreach_callback cb) { \
	array_foreach_reverse(arr, (array_foreach_callback) cb); \
}



#ifdef __cplusplus
}
#endif


#endif
