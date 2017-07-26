#include <assert.h>
#include <memory.h>
#include <malloc.h>
#include "array.h"

#define _ORI_CAPACITY 4
#define _MAX_ELEMENT_SIZE 2048

static void _expand_capacity(array_t *arr) {
	if(arr->_size == arr->_capacity) {
		int capacity = arr->_capacity << 1;
		void * data = realloc(arr->_data, arr->_element_size * capacity);
		if(data) {
			arr->_data = data;
			arr->_capacity = capacity;
		}
	}
}

static void _dealloc_element(array_t * arr, size_t idx) {
	if(arr->_element_dealloc) {
		size_t offset = arr->_element_size * idx;
		void *p_ele = (void *) (((char *)arr->_data) + offset);
		arr->_element_dealloc(p_ele);
	}
}

int array_init(array_t * arr, size_t element_size) {
	assert(arr);
	assert(element_size > 0 && element_size <= _MAX_ELEMENT_SIZE);

	arr->_size = 0;
	arr->_capacity = _ORI_CAPACITY;
	arr->_element_size = element_size;
	arr->_element_dealloc = 0;
	arr->_data = malloc(element_size * _ORI_CAPACITY);
	if(!arr->_data) {
		return -1;
	}
	return 0;
}

void array_uinit(array_t * arr) {
	assert(arr);

	array_clear(arr);

	if(arr->_data) {
		free(arr->_data);
	}

	memset(arr, 0x00, sizeof(*arr));
}

size_t array_capacity(array_t * arr) {
	assert(arr);
	return arr->_capacity;
}

size_t array_size(array_t * arr) {
	assert(arr);
	return arr->_size;
}

size_t array_element_size(array_t *arr) {
	assert(arr);
	return arr->_element_size;
}

int array_empty(array_t * arr) {
	assert(arr);
	return arr->_size == 0;
}

void array_shrink(array_t * arr) {
	assert(arr);

	if(arr->_size <= _ORI_CAPACITY) {
		if(arr->_capacity > _ORI_CAPACITY) {
			void * data = realloc(arr->_data, arr->_element_size *_ORI_CAPACITY);
			if(data) {
				arr->_data = data;
				arr->_capacity = _ORI_CAPACITY;
			}
		}
		return;
	}

	if(arr->_size < arr->_capacity) {
		void *data = realloc(arr->_data, arr->_element_size * arr->_size);
		if(data) {
			arr->_data = data;
			arr->_capacity = arr->_size;
		}
	}
}

int array_push_back(array_t *arr, void * p_ele) {
	assert(arr);
	assert(p_ele);

	_expand_capacity(arr);

	if(arr->_capacity > arr->_size) {
		size_t offset = arr->_element_size * arr->_size;
		memcpy(((char *)arr->_data) + offset, (char *)p_ele, arr->_element_size);
		arr->_size++;
		return 0;
	}

	return -1;
}

void array_pop_back(array_t *arr) {
	assert(arr);

	if(arr->_size > 0) {
		_dealloc_element(arr, arr->_size-1);
		arr->_size--;
	}
}

int array_at(array_t *arr, size_t idx, void * p_ele) {
	assert(arr);
	assert(idx < arr->_size);

	if(idx < arr->_size) {
		size_t offset = arr->_element_size *idx;
		memcpy((char *)p_ele, ((char *)arr->_data) + offset, arr->_element_size);
		return 0;
	}
	return -1;
}

int array_front(array_t *arr, void * p_ele) {
	assert(arr);
	assert(arr->_size > 0);
	
	if(arr->_size > 0) {
		memcpy((char *)p_ele, (char *)arr->_data, arr->_element_size);
		return 0;
	}
	return -1;
}

int array_back(array_t * arr, void * p_ele) {
	assert(arr);
	assert(arr->_size > 0);

	if(arr->_size > 0) {
		size_t offset = arr->_element_size * (arr->_size -1);
		memcpy((char *)p_ele, ((char *)arr->_data) + offset, arr->_element_size);
		return 0;
	}
	return -1;
}

int array_insert(array_t *arr, void * p_ele, size_t idx) {
	assert(arr);
	assert(idx <= arr->_size);

	if(idx <= arr->_size) {
		_expand_capacity(arr);
		if(arr->_capacity > arr->_size) {
			arr->_size++;
			size_t i = arr->_size;
			while(i>idx) {
				i--;
				size_t offset = arr->_element_size *i;
				size_t offset_prev = offset - arr->_element_size;
				memcpy(((char *)arr->_data)+offset, ((char *)arr->_data)+offset_prev, arr->_element_size);
			}
			size_t offset = arr->_element_size * idx;
			memcpy(((char *)arr->_data)+offset, (char *)p_ele, arr->_element_size);
			return 0;
		}
	}
	return -1;
}

void array_erase(array_t *arr, size_t idx) {
	assert(arr);
	assert(idx < arr->_size);

	if(idx < arr->_size) {
		_dealloc_element(arr, idx);

		int i=idx;
		for(; i<(arr->_size-1); ++i) {
			size_t offset = arr->_element_size * i;
			size_t offset_next = offset + arr->_element_size;
			memcpy(((char *)arr->_data)+offset, ((char *)arr->_data)+offset_next, arr->_element_size);
		}
		arr->_size--;
	}
}

void array_clear(array_t *arr) {
	assert(arr);
	
	size_t i = 0;
	while(i<arr->_size) {
		_dealloc_element(arr, i);
		i++;
	}
	
	arr->_size = 0;
}

void array_foreach(array_t *arr, array_foreach_callback cb) {
	assert(arr);
	size_t i=0;
	while(i< arr->_size) {
		size_t offset = arr->_element_size *i;
		void * p_ele = (void *) (((char *)arr->_data)+offset);
		if(cb) {
			cb(arr, p_ele, i);
		}
		i++;
	}
}

void array_foreach_reverse(array_t *arr, array_foreach_callback cb) {
	assert(arr);

	size_t i = arr->_size;
	while(i>0) {
		i--;
		size_t offset = arr->_element_size *i;
		char * p_ele = (void *) (((char *)arr->_data)+offset);
		if(cb) {
			cb(arr, p_ele, i);
		}
	}
}



