/** tbd.c
 *  
 * Tiny Basic Datastore implementation.  
 * All tbd configuration and functionality is placed in this file.
 * 
 * Author: Joshua Petitt
 * Available at: https://github.com/jpmec/tbd 
 *
 * Copyright (c) 2012 Joshua Petitt
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 *
 *
 * TBD Memory Model
 * ================
 *
 *
 *    |                          |
 *    +--------------------------+
 *    | tbd header information   | <-- tbd start address
 *    | ...                      |
 *    +--------------------------+
 *    | tbd bottom of stack      | <-- last (oldest) stack element
 *    | ...                      |
 *    |-                         |
 *    | ...                      |
 *    |                          |
 *    |-                         |
 *    | tbd top of stack         | <-- first (newest) stack element
 *    | ...                      |
 *    +--------------------------+
 *    |                          |
 *    | ... available memory ... |
 *    |                          |
 *    +--------------------------+
 *    | tbd top of heap          | <-- newest heap data
 *    | ...                      |
 *    | tbd bottom of heap       | <-- oldest heap data
 *    +--------------------------+
 *    |                          | <-- tbd start address + tbd size
 *
 *
 *
 *
 * Configuration
 * =============
 * The tbd library is designed to be flexible.  It can be configured at compile time to be either small or fast.
 * The tbd library can be configured to use very little overhead.  This will reduce the amount of memory used by the header information
 * and stack.
 *
 * In the tiniest configuration for tbd, it uses the following memory
 *
 */




#include "tbd.h"

#include <string.h>
#include <stdbool.h>



/** Version number.
 * This will probably be 0 for a long, long time.
 */
#define TBD_VERSION    (0)




/* 
 * Compiler options
 * Comment out any functionality that shouldn't be included in the library.
 */

// Include the <assert.h> standard C library header.
#define TBD_INCLUDE_ASSERT

// Include the <stdio.h> standard C libray header.
#define TBD_INCLUDE_STDIO

// Use null-terminated C strings for keys.
// This will eliminate the need to store the length of the string.
// Some functions may run slower because of the need to call strlen().
// If not defined, a size_t size parameter will be used to store the length of each key.
// Some functions may run faster because of immediate access to the key size.
#define TBD_USE_NULL_TERMINATED_KEY_STRINGS

// Use null-terminated values.  
// This will not allow a null char to be stored.
// Some functions may run slower because of the need to call strlen().
// If not defined, a size_t size parameter will be used to store the length of each value.
// Some functions may run faster because of immediate access to the value size.
#define TBD_USE_NULL_TERMINATED_VALUES

// Cache the last found key:value.
//#define TBD_USE_LAST_FOUND_CACHE

// Use a list for accessing garbage elements.
//#define TBD_USE_GARBAGE_LIST

// Create the smallest possible database.
// The is requires keys and values to be null terimated strings.
#define TBD_MAKE_TINY






/* 
 * Assert support
 */
#if defined(TBD_INCLUDE_ASSERT)
  #include <assert.h>

  #define TBD_ASSERT(_arg_)    assert(_arg_)

#else

  #define TBD_ASSERT(_arg_)

#endif




/*
 * Standard I/O support.
 * Needed for JSON.
 */
#if defined(TBD_INCLUDE_STDIO)
  #include <stdio.h>
#endif





/** Key reference structure.
 */
typedef struct tbd_key_struct
{
  char* str;           ///< Pointer to key string data.
  
#ifndef TBD_USE_NULL_TERMINATED_KEY_STRINGS
  TBD_SIZE_T size;     ///< Size in bytes of key string data.
#endif  
  
} tbd_key_t;




static void tbd_key_clear(tbd_key_t* key)
{
  TBD_ASSERT(key);
  
  key->str = NULL;

#ifndef TBD_USE_NULL_TERMINATED_KEY_STRINGS  
  key->size = 0;
#endif
}




static TBD_SIZE_T tbd_key_size(const tbd_key_t* key)
{
  TBD_ASSERT(key);
  
#ifndef TBD_USE_NULL_TERMINATED_KEY_STRINGS  
  return key->size;
#else
  return strlen(key->str);
#endif  
}







/** Value reference structure.
 */
typedef struct tbd_value_struct
{
  unsigned char* data;     ///< Pointer to generic stored data.
 
#ifndef TBD_USE_NULL_TERMINATED_VALUES  
  TBD_TBD_SIZE_T size;     ///< Size in bytes of generic stored data.
#endif
  
} tbd_value_t;




static void tbd_value_clear(tbd_value_t* self)
{
  TBD_ASSERT(self);
  
  self->data = NULL;
  
#ifndef TBD_USE_NULL_TERMINATED_VALUES  
  self->size = 0;
#endif
  
}




static TBD_SIZE_T tbd_value_size(const tbd_value_t* self)
{
  TBD_ASSERT(self);
  
#ifndef TBD_USE_NULL_TERMINATED_VALUES
  return self->size;
#else
  return strlen((char*) self->data) + 1;  // add 1 for null terminator
#endif
}








/** Heap structure.
 *  Represents range of contiguous memory.
 */
typedef struct tbd_heap_struct
{
  unsigned char* top;       ///< Pointer to top of heap.
  TBD_SIZE_T size;          ///< Size in bytes of allocated heap. 
  
} tbd_heap_t;




static void tbd_heap_clear(tbd_heap_t* heap)
{
  TBD_ASSERT(heap);
  
  heap->top = NULL;
  heap->size = 0;
}




static void tbd_heap_empty(tbd_heap_t* heap)
{
  TBD_ASSERT(heap);
  
  heap->size = 0;
}




static int tbd_heap_is_empty(tbd_heap_t* heap)
{
  TBD_ASSERT(heap);
  
  return 0 == heap->size;
}




static unsigned char* tbd_heap_push(tbd_heap_t* heap, TBD_SIZE_T hunk_size)
{
  TBD_ASSERT(heap);
  
  heap->size += hunk_size;
  heap->top -= hunk_size;  // heap grows into lower memory addresses
  
  return heap->top;
}




static unsigned char* tbd_heap_pop(tbd_heap_t* heap, TBD_SIZE_T hunk_size)
{
  TBD_ASSERT(heap);
  
  heap->size -= hunk_size;
  heap->top += hunk_size;  // heap grows into lower memory addresses
  
  return heap->top;
}




/** Key-Value pair structure.
 */
typedef struct tbd_keyvalue_struct
{ 
  tbd_heap_t heap;      ///< Heap data allocated to this keyvalue.
  tbd_key_t key;        ///< The key reference.
  tbd_value_t value;    ///< The value reference.
  
  bool is_garbage;                             ///< True if the data in the keyvalue is garbage (i.e. ready to be reclaimed).
  
#ifdef TBD_USE_GARBAGE_LIST  
  struct tbd_keyvalue_struct* prev_garbage;    ///< Pointer to previous element before this that is garbage.
  struct tbd_keyvalue_struct* next_garbage;    ///< Pointer to next element after this that is garbage.
#endif  
  
} tbd_keyvalue_t;




/** Clear values for given keyvalue.
 */
static void tbd_keyvalue_clear(tbd_keyvalue_t* keyvalue)
{
  TBD_ASSERT(keyvalue);
  
  tbd_key_clear(&keyvalue->key);
  tbd_value_clear(&keyvalue->value);
  tbd_heap_clear(&keyvalue->heap);

  keyvalue->is_garbage = true;
  
#ifdef TBD_USE_GARBAGE_LIST   
  keyvalue->prev_garbage = NULL;
  keyvalue->next_garbage = NULL;
#endif
}




/** Get the number of bytes used by the given keyvalue.
 */
static TBD_SIZE_T tbd_keyvalue_size(const tbd_keyvalue_t* keyvalue)
{
  TBD_ASSERT(keyvalue);
  
  return keyvalue->heap.size + sizeof(tbd_keyvalue_t);
}




/** Copy keyvalue data from src to dest.
 */
static TBD_SIZE_T tbd_keyvalue_copy(tbd_keyvalue_t* dest, const tbd_keyvalue_t* src)
{
  TBD_ASSERT(dest);
  TBD_ASSERT(src);
  
  // copy heap data

  const TBD_SIZE_T value_size = tbd_value_size(&src->value);
  
  memcpy(dest->value.data, src->value.data, value_size);
  
  const TBD_SIZE_T key_size = tbd_key_size(&src->key);
  
  memcpy(dest->key.str, src->key.str, key_size);
  
  // copy stack data
  dest->key = src->key;
  dest->value = src->value;
  
  return tbd_keyvalue_size(src);
}




#ifdef TBD_USE_GARBAGE_LIST

/** Return number of bytes of garbage for the keyvalue.
 */
static TBD_SIZE_T tbd_keyvalue_garbage_list_size(const tbd_keyvalue_t* keyvalue)
{
  TBD_ASSERT(keyvalue);
  
  TBD_SIZE_T total_size = 0;
  
  while(keyvalue)
  {
    total_size += tbd_keyvalue_size(keyvalue);
    keyvalue = keyvalue->next_garbage;
  }
  
  return total_size;
}

#endif






/** Simple iterator structure for stack
 */
typedef struct tbd_keyvalue_stack_iterator_struct
{
  tbd_keyvalue_t* ptr;
  
} tbd_keyvalue_stack_iterator_t;




/** Move iterator to next element.
 */
static tbd_keyvalue_t* tbd_keyvalue_stack_iterator_next(tbd_keyvalue_stack_iterator_t* self)
{
  TBD_ASSERT(self);
  
  --self->ptr;
  return self->ptr;
}




/** Move iterator to previous element.
 */
static tbd_keyvalue_t* tbd_keyvalue_stack_iterator_prev(tbd_keyvalue_stack_iterator_t* self)
{
  TBD_ASSERT(self);
  
  ++self->ptr;
  return self->ptr;
}




/** Simple iterator structure for stack
 */
typedef struct tbd_keyvalue_stack_reverse_iterator_struct
{
  tbd_keyvalue_t* ptr;
  
} tbd_keyvalue_stack_reverse_iterator_t;




/** Move iterator to next element.
 */
static tbd_keyvalue_t* tbd_keyvalue_stack_reverse_iterator_next(tbd_keyvalue_stack_reverse_iterator_t* self)
{
  TBD_ASSERT(self);
  
  ++self->ptr;
  return self->ptr;
}




/** Move iterator to previous element.
 */
static tbd_keyvalue_t* tbd_keyvalue_stack_reverse_iterator_prev(tbd_keyvalue_stack_reverse_iterator_t* self)
{
  TBD_ASSERT(self);
  
  --self->ptr;
  return self->ptr;
}






/** Simple const iterator structure for stack
 */
typedef struct tbd_keyvalue_stack_const_iterator_struct
{
  const tbd_keyvalue_t* ptr;
  
} tbd_keyvalue_stack_const_iterator_t;




/** Move iterator to next element.
 */
static const tbd_keyvalue_t* tbd_keyvalue_stack_const_iterator_next(tbd_keyvalue_stack_const_iterator_t* self)
{
  TBD_ASSERT(self);
  
  --self->ptr;
  return self->ptr;
}




/** Move iterator to previous element.
 */
static const tbd_keyvalue_t* tbd_keyvalue_stack_const_iterator_prev(tbd_keyvalue_stack_const_iterator_t* self)
{
  TBD_ASSERT(self);
  
  ++self->ptr;
  return self->ptr;
}




/** Test if iterators are equivalent.
 */
static int tbd_keyvalue_stack_const_iterator_is_equal(const tbd_keyvalue_stack_const_iterator_t* self, const tbd_keyvalue_stack_iterator_t* iter)
{
  TBD_ASSERT(self);
  TBD_ASSERT(iter);
  
  return (self->ptr == iter->ptr);
}



/** Simple const iterator structure for stack
 */
typedef struct tbd_keyvalue_stack_const_reverse_iterator_struct
{
  const tbd_keyvalue_t* ptr;
  
} tbd_keyvalue_stack_const_reverse_iterator_t;




/** Move iterator to next element.
 */
static const tbd_keyvalue_t* tbd_keyvalue_stack_const_reverse_iterator_next(tbd_keyvalue_stack_const_reverse_iterator_t* self)
{
  TBD_ASSERT(self);
  
  ++self->ptr;
  return self->ptr;
}




/** Move iterator to previous element.
 */
static const tbd_keyvalue_t* tbd_keyvalue_stack_const_iterator_reverse_prev(tbd_keyvalue_stack_const_reverse_iterator_t* self)
{
  TBD_ASSERT(self);
  
  --self->ptr;
  return self->ptr;
}



/** Compare iterators to see if they are equivalent.
 */
static int tbd_keyvalue_stack_reverse_iterator_is_equal(const tbd_keyvalue_stack_const_reverse_iterator_t* self, const tbd_keyvalue_stack_reverse_iterator_t* iter)
{
  TBD_ASSERT(self);
  TBD_ASSERT(iter);
  
  return self->ptr == iter->ptr;
}




/** A simple stack structure.
 */
typedef struct tbd_keyvalue_stack_struct
{
  struct tbd_keyvalue_struct* start;    ///< Pointer to start of keyvalue stack.
  TBD_SIZE_T count;                         ///< Number of elements in keyvalue stack.  
  
} tbd_keyvalue_stack_t;




static TBD_SIZE_T tbd_keyvalue_stack_count(tbd_keyvalue_stack_t* self)
{
  TBD_ASSERT(self);
  
  return self->count;
}




static void tbd_keyvalue_stack_clear(tbd_keyvalue_stack_t* stack)
{
  TBD_ASSERT(stack);
  
  stack->start = NULL;
  stack->count = 0;
}




/** Empties the stack.
 */
static void tbd_keyvalue_stack_empty(tbd_keyvalue_stack_t* stack)
{
  TBD_ASSERT(stack);
  
  stack->count = 0;
}




/** Returns pointer to top element on the stack.
 *  Returns NULL if no elements exist.
 */
static tbd_keyvalue_t* tbd_keyvalue_stack_top(const tbd_keyvalue_stack_t* stack)
{
  TBD_ASSERT(stack);
  
  if (!stack->count)
  {
    return NULL;
  }
  
  return stack->start + (stack->count - 1);
}




static tbd_keyvalue_t* tbd_keyvalue_stack_get(const tbd_keyvalue_stack_t* stack, TBD_SIZE_T index)
{
  TBD_ASSERT(stack);
  
  if (index >= stack->count)
  {
    return NULL;
  }
  
  return stack->start + index;
}




static tbd_keyvalue_t* tbd_keyvalue_stack_push(tbd_keyvalue_stack_t* stack)
{
  TBD_ASSERT(stack);
  
  ++stack->count;
  
  return tbd_keyvalue_stack_get(stack, stack->count - 1);
}




static void tbd_keyvalue_stack_pop(tbd_keyvalue_stack_t* stack)
{
  TBD_ASSERT(stack);
  
  --stack->count;
}




static tbd_keyvalue_stack_iterator_t tbd_keyvalue_stack_begin(tbd_keyvalue_stack_t* stack)
{
  TBD_ASSERT(stack);
  
  return (tbd_keyvalue_stack_iterator_t) 
  {
    .ptr = tbd_keyvalue_stack_top(stack)
  };
}




static tbd_keyvalue_stack_const_iterator_t tbd_keyvalue_stack_const_begin(const tbd_keyvalue_stack_t* stack)
{
  TBD_ASSERT(stack);
  
  return (tbd_keyvalue_stack_const_iterator_t) 
  {
    .ptr = tbd_keyvalue_stack_top(stack)
  };
}




static tbd_keyvalue_stack_const_iterator_t tbd_keyvalue_stack_end(const tbd_keyvalue_stack_t* stack)
{
  TBD_ASSERT(stack);
  
  return (tbd_keyvalue_stack_const_iterator_t) 
  {
    .ptr = stack->start - 1
  };
}




static tbd_keyvalue_stack_reverse_iterator_t tbd_keyvalue_stack_rbegin(tbd_keyvalue_stack_t* stack)
{
  TBD_ASSERT(stack);
  
  return (tbd_keyvalue_stack_reverse_iterator_t) 
  {
    .ptr = stack->start
  };
}




static tbd_keyvalue_stack_const_reverse_iterator_t tbd_keyvalue_stack_rend(const tbd_keyvalue_stack_t* stack)
{
  TBD_ASSERT(stack);
  
  if (!stack->count)
  {
    return (tbd_keyvalue_stack_const_reverse_iterator_t) 
    {
      .ptr = stack->start
    }; 
  }
  else
  {
    return (tbd_keyvalue_stack_const_reverse_iterator_t) 
    {
      .ptr = tbd_keyvalue_stack_top(stack) + 1
    };
  }
}




static bool tbd_keyvalue_stack_is_contiguous(const tbd_keyvalue_stack_t* stack)
{
  TBD_ASSERT(stack);
  
  tbd_keyvalue_stack_const_iterator_t top = tbd_keyvalue_stack_const_begin(stack);  
  tbd_keyvalue_stack_const_iterator_t end = tbd_keyvalue_stack_end(stack);
  
  while (top.ptr != end.ptr)
  {
    // TODO check for contiguity.
    tbd_keyvalue_stack_const_iterator_next(&top);
  }
  
  return true;
}






#ifdef TBD_USE_GARBAGE_LIST

/** Garbage list structure.
 */
typedef struct tbd_garbage_list_struct
{
  struct tbd_keyvalue_struct* front;   ///< Pointer to the front of a list of elements that are garbage (can be garbage collected). 
  struct tbd_keyvalue_struct* back;    ///< Pointer to the back of a list of elements that are garbage (can be garbage collected).  
 
} tbd_garbage_list_t;




static void tbd_garbage_list_clear(tbd_garbage_list_t* garbage)
{
  TBD_ASSERT(garbage);
  
  garbage->front = NULL;
  garbage->back = NULL;
}



static TBD_SIZE_T tbd_garbage_list_count(tbd_garbage_list_t* self)
{
  TBD_ASSERT(self);
  
  size_t result = 0;
  
  tbd_keyvalue_t* ptr = tbd->garbage.front;
  
  while (ptr)
  {
    ++result;
    ptr = ptr->next_garbage;
  }
  
  return result;
}




static tbd_keyvalue_t* tbd_garbage_list_last(tbd_garbage_list_t* garbage)
{
  TBD_ASSERT(garbage);
  
  tbd_keyvalue_t* keyvalue = garbage->front; 
  
  if (!keyvalue)
  {
    return NULL;
  }
  
  while(keyvalue->next_garbage)
  {
    keyvalue = keyvalue->next_garbage;
  }
  
  return keyvalue;
}




static void tbd_garbage_list_insert(tbd_garbage_list_t* garbage, tbd_keyvalue_t* keyvalue)
{
  TBD_ASSERT(garbage);
  TBD_ASSERT(keyvalue);
  
  if (!keyvalue)
  {
    return;
  }
  
  if (!garbage->front)
  {
    garbage->front = keyvalue;
    garbage->back = keyvalue;
    keyvalue->prev_garbage = 0;
    keyvalue->next_garbage = 0;
  }
  else
  {
    // find the insertion point
    tbd_keyvalue_t* prev = garbage->front;
    tbd_keyvalue_t* next = garbage->front->next_garbage;
    
    while ( (next) && (tbd_keyvalue_size(next) < tbd_keyvalue_size(keyvalue)) )
    {
      prev = next;
      next = next->next_garbage;         
    }
    
    // find the last keyvalue element
    tbd_keyvalue_t* keyvalue_last = keyvalue;
    
    while (keyvalue_last->next_garbage)
    {
      keyvalue_last = keyvalue_last->next_garbage;
    }
    
    // insert the sub list
    prev->next_garbage = keyvalue;
    keyvalue->prev_garbage = prev;
    
    if (next && (next != prev))
    {
      next->prev_garbage = keyvalue_last;
      keyvalue_last->next_garbage = next;
    }
  }
  
  keyvalue->is_garbage = true;
}




static void tbd_garbage_list_delete(tbd_garbage_list_t* garbage, tbd_keyvalue_t* keyvalue)
{
  TBD_ASSERT(garbage);
  TBD_ASSERT(keyvalue);
  
  if (garbage->front == keyvalue)
  {
    garbage->front = keyvalue->next_garbage;
  }
  
  if (garbage->back == keyvalue)
  {
    garbage->back = keyvalue->prev_garbage;
  }
  
  // unlink keyvalue from garbage list
  tbd_keyvalue_t* prev = keyvalue->prev_garbage;  
  tbd_keyvalue_t* next = keyvalue->next_garbage;
  
  if (prev)
  {
    prev->next_garbage = keyvalue->next_garbage;
  }
  
  if (next)
  {
    next->prev_garbage = keyvalue->prev_garbage;
  }
  
  keyvalue->is_garbage = false;
}


#endif//TBD_USE_GARBAGE_LIST






/** Main structure for a tbd.
 *
 *  Stores meta-data about the memory region used for tbd.
 *
 *  The keyvalue stack grows downward into memory (higher memory addresses are used for added key elements).
 *  The heap grows upward into memory (lower memory addresses are used for added heap elements).
 *
 */
struct tbd_struct
{
  TBD_SIZE_T size;         ///< Total size in bytes of the allocated datastore in bytes.
  TBD_SIZE_T hunk_size;    ///< Hunk size for the datastore, this is the minimum size that is allocated from the heap.
  
#ifdef TBD_USE_LAST_FOUND_CACHE  
  tbd_keyvalue_t* last_found;    ///< Pointer to last keyvalue that was found using tbd_keyvalue_find.
#endif  
  
#ifdef TBD_USE_GARBAGE_LIST  
  tbd_garbage_list_t garbage;    ///< The garbage list.
#endif  
  
  tbd_keyvalue_stack_t stack;    ///< The keyvalue stack.
  tbd_heap_t heap;               ///< The data heap.
};




/** Compute number of bytes needed from heap to store key and value.
 */
static TBD_SIZE_T tbd_keyvalue_hunk_size(const tbd_t* tbd, TBD_SIZE_T key_size, TBD_SIZE_T value_size)
{
  TBD_ASSERT(tbd);
  
  TBD_SIZE_T hunk_count = (key_size + value_size) / tbd->hunk_size; // calculate number of hunks required for a keyvalue
  
  if (!hunk_count) // must use at least 1 hunk
  {
    hunk_count = 1;
  }
  
  return hunk_count * tbd->hunk_size;
}


/**
 */
static void tbd_reclaim_garbage(tbd_t* tbd, tbd_keyvalue_t* keyvalue)
{
  TBD_ASSERT(tbd);
  TBD_ASSERT(keyvalue);
  
  #if defined(TBD_USE_GARBAGE_LIST)
    tbd_garbage_list_delete(&tbd->garbage, btm.ptr);  
  #endif
}




/** Find first garbage keyvalue that uses a given hunk size from the heap.
 */
static tbd_keyvalue_t* tbd_find_first_garbage_hunk(tbd_t* tbd, TBD_SIZE_T hunk_size)
{
  tbd_keyvalue_stack_reverse_iterator_t btm = tbd_keyvalue_stack_rbegin(&tbd->stack);
  tbd_keyvalue_stack_const_reverse_iterator_t btm_end = tbd_keyvalue_stack_rend(&tbd->stack);
  
  if (btm.ptr && btm_end.ptr)
  {
    while (!tbd_keyvalue_stack_reverse_iterator_is_equal(&btm_end, &btm))
    {
      if (btm.ptr->is_garbage && (btm.ptr->heap.size == hunk_size))
      {                
        return btm.ptr;
      }
      
      tbd_keyvalue_stack_reverse_iterator_next(&btm);
    }
  }  
  
  return NULL;
}




/** Create a new keyvalue with the given sizes.
 */
static tbd_keyvalue_t* tbd_create_keyvalue(tbd_t* tbd, TBD_SIZE_T key_size, TBD_SIZE_T value_size)
{
  TBD_ASSERT(tbd);

  const TBD_SIZE_T hunk_size = tbd_keyvalue_hunk_size(tbd, key_size, value_size);
  
  tbd_keyvalue_t* keyvalue = NULL;

  // try to find a garbage element with same heap size  
  keyvalue = tbd_find_first_garbage_hunk(tbd, hunk_size);  

  if (keyvalue)
  {
    tbd_reclaim_garbage(tbd, keyvalue);
  }

  // if no suitable garbage could be reclaimed, then allocate from heap
  else
  {
    // check there is enough room to add elements of that size
    keyvalue = tbd_keyvalue_stack_push(&tbd->stack);
    unsigned char* heap_top = tbd_heap_push(&tbd->heap, hunk_size);
  
    const unsigned char* stack_btm = (unsigned char*) keyvalue + sizeof(tbd_keyvalue_t);
    
    if (heap_top < stack_btm)
    {
      tbd_keyvalue_stack_pop(&tbd->stack);
      tbd_heap_pop(&tbd->heap, hunk_size);
      return NULL;
    }

    keyvalue->heap.top = heap_top;  
    keyvalue->heap.size = hunk_size;  

    // initialize garbage list node
    keyvalue->is_garbage = false;
    
#ifdef TBD_USE_GARBAGE_LIST    
    keyvalue->next_garbage = NULL;
    keyvalue->prev_garbage = NULL;
#endif
    
  }
  
  // set value and key pointers
  keyvalue->value.data = keyvalue->heap.top;
  
#ifndef TBD_USE_NULL_TERMINATED_VALUES   
  keyvalue->value.size = value_size;
#else
  memset(keyvalue->value.data, 0, value_size);
#endif
  
  // store string at end so each key:value heap allocation is null terminated
  keyvalue->key.str = (char*) (keyvalue->heap.top + value_size);
  
#ifndef TBD_USE_NULL_TERMINATED_KEY_STRINGS  
  keyvalue->key.size = key_size;
#endif
  
  return keyvalue;
}




/** Key comparision
 */
static int tbd_keyvalue_keycmp(tbd_keyvalue_t* keyvalue, const char* key)
{
  TBD_ASSERT(keyvalue);
  TBD_ASSERT(key);
  
  return strcmp(keyvalue->key.str, key);
}




/** Find a keyvalue struct with a given key.
 *  Returns NULL if key was not found.
 */
static tbd_keyvalue_t* tbd_keyvalue_find(tbd_t* tbd, const char* key)
{
  TBD_ASSERT(tbd);
  
  if (tbd_is_empty(tbd))
  {
    return NULL;
  }
  
#ifdef TBD_USE_LAST_FOUND_CACHE  
  
  // Look in cached pointer first.
  if (tbd->last_found && !tbd->last_found->is_garbage && tbd_keyvalue_keycmp(tbd->last_found, key) == 0)
  {
    return tbd->last_found;
  }
  
#endif  
  
  // a simple linear search for a given key
  // TODO convert to standard C search
  
  tbd_keyvalue_stack_iterator_t iter = tbd_keyvalue_stack_begin(&tbd->stack);
  tbd_keyvalue_stack_const_iterator_t end = tbd_keyvalue_stack_end(&tbd->stack);
  
  if (!iter.ptr)
  {
    return NULL;
  }
  
  while (!tbd_keyvalue_stack_const_iterator_is_equal(&end, &iter))
  {
    if (!iter.ptr->is_garbage && (tbd_keyvalue_keycmp(iter.ptr, key) == 0))
    {
      
#ifdef TBD_USE_LAST_FOUND_CACHE      
      tbd->last_found = iter.ptr;
#endif      
      
      return iter.ptr;
    }
    
    tbd_keyvalue_stack_iterator_next(&iter);
  }
  
  return 0;
}




static const tbd_keyvalue_t* tbd_keyvalue_find_const(const tbd_t* tbd, const char* key)
{
  // Cast away constness, then cast it back before returning.
  // Allows the use of tbd_keyvalue_find function.
  return (const tbd_keyvalue_t*) tbd_keyvalue_find((tbd_t*) tbd, key);
}




int tbd_version(void)
{
  return TBD_VERSION;
}



int tbd_is_error(int value)
{
  // all error codes are less than 0
  return value < 0;
}



size_t tbd_size(const tbd_t* tbd)
{
  TBD_ASSERT(tbd);
  
  return tbd->size;
}




size_t tbd_head_size(const tbd_t* tbd)
{
  TBD_ASSERT(tbd);
  
  return sizeof(tbd_t);
}




size_t tbd_size_used(const tbd_t* tbd)
{
  TBD_ASSERT(tbd);
  
  return tbd_head_size(tbd) + tbd->stack.count * sizeof(tbd_keyvalue_t) + tbd->heap.size;
}




size_t tbd_count(const tbd_t* tbd)
{
  TBD_ASSERT(tbd);
  
  return tbd->stack.count;
}




int tbd_copy(tbd_t* dest, const tbd_t* src)
{
  TBD_ASSERT(dest);
  TBD_ASSERT(src);

  tbd_keyvalue_stack_const_iterator_t src_stack = tbd_keyvalue_stack_const_begin(&src->stack);

  while (src_stack.ptr)
  {
    //tbd_create(dest, src_stack.ptr->key, 
    
    tbd_keyvalue_stack_const_iterator_next(&src_stack);
  }
  
  return TBD_NO_ERROR;
}




int tbd_create(tbd_t* tbd, const char* key, const void* value, size_t value_size)
{
  TBD_ASSERT(tbd);
  TBD_ASSERT(key);
  TBD_ASSERT(value);
  TBD_ASSERT(value_size);
  
  TBD_ASSERT(TBD_MAX_SIZE >= value_size);
  
  
  // check that an element with given key does not already exist
  if (tbd_keyvalue_find(tbd, key))
  {
    return TBD_ERROR_KEY_EXISTS;
  }
  
  const size_t key_size = strlen(key) + 1;
  
  assert(TBD_MAX_KEY_LENGTH >= key_size - 1);
  
  struct tbd_keyvalue_struct* keyvalue = tbd_create_keyvalue(tbd, key_size, value_size);
  
  if (!keyvalue)
  {
    return TBD_ERROR;
  }
  
  // copy the data into the newly allocated memory
  memcpy(keyvalue->key.str, key, key_size);
  memcpy(keyvalue->value.data, value, value_size);
  
  return TBD_NO_ERROR;   
}




int tbd_read(tbd_t* tbd, const char* key, void* value, size_t value_size)
{
  TBD_ASSERT(tbd);
  TBD_ASSERT(key);
  TBD_ASSERT(value);
  TBD_ASSERT(value_size);
  
  // find the element
  struct tbd_keyvalue_struct* ptr = tbd_keyvalue_find(tbd, key);
  if (!ptr)
  {
    return TBD_ERROR_KEY_NOT_FOUND;
  }
  
  // copy value size
  const TBD_SIZE_T ptr_value_size = tbd_value_size(&ptr->value);
  
  if (ptr_value_size != value_size)
  {
    return TBD_ERROR_BAD_SIZE;
  }

  // copy the data to the value
  memcpy(value, ptr->value.data, value_size);
  
  return TBD_NO_ERROR;
}




int tbd_update(tbd_t* tbd, const char* key, const void* value, size_t value_size)
{
  TBD_ASSERT(tbd);
  TBD_ASSERT(key);
  TBD_ASSERT(value);
  TBD_ASSERT(value_size);
  
  // find the element
  struct tbd_keyvalue_struct* ptr = tbd_keyvalue_find(tbd, key);
  if (!ptr)
  {
    return TBD_ERROR_KEY_NOT_FOUND;
  }
  
  const TBD_SIZE_T ptr_value_size = tbd_value_size(&ptr->value);
  
  if (value_size != ptr_value_size)
  {
    return TBD_ERROR_BAD_SIZE;
  }
  
  // compute minimum copy size, truncate to value_size
  const size_t copy_size = (ptr_value_size < value_size) ? ptr_value_size: value_size;
  
  // copy the value into datastore
  memcpy(ptr->value.data, value, copy_size);
  
  return TBD_NO_ERROR;
}




int tbd_delete(tbd_t* tbd, const char* key)
{
  TBD_ASSERT(tbd);
  TBD_ASSERT(key);
  
  // find the element
  struct tbd_keyvalue_struct* ptr = tbd_keyvalue_find(tbd, key);
  if (!ptr)
  {
    return TBD_NO_ERROR; // no error if it did not exist
  }  
  
  #if defined(TBD_USE_GARBAGE_LIST)
    tbd_garbage_list_insert(&tbd->garbage, ptr);
  #endif  
  
  ptr->is_garbage = true;
  
  return TBD_NO_ERROR;
}




size_t tbd_read_size(tbd_t* tbd, const char* key)
{
  TBD_ASSERT(tbd);
  TBD_ASSERT(key);
  
  // find the element
  struct tbd_keyvalue_struct* ptr = tbd_keyvalue_find(tbd, key);
  if (!ptr)
  {
    return 0;
  }

  return tbd_value_size(&ptr->value);
}




void tbd_clear(tbd_t* tbd)
{
  TBD_ASSERT(tbd);

  tbd_keyvalue_stack_clear(&tbd->stack);
  tbd_heap_clear(&tbd->heap);

  #if defined(TBD_USE_LAST_FOUND_CACHE)  
    tbd->last_found = NULL;
  #endif  
  
  #if defined(TBD_USE_GARBAGE_LIST)
    tbd_garbage_list_clear(&tbd->garbage);
  #endif
}




void tbd_empty(tbd_t* tbd)
{
  TBD_ASSERT(tbd);

  tbd_keyvalue_stack_empty(&tbd->stack);
  tbd_heap_empty(&tbd->heap);

  #if defined(TBD_USE_LAST_FOUND_CACHE)  
    tbd->last_found = NULL;
  #endif  
  
  #if defined (TBD_USE_GARBAGE_LIST)
    tbd_garbage_list_clear(&tbd->garbage);
  #endif
}




int tbd_is_empty(const tbd_t* tbd)
{
  TBD_ASSERT(tbd);
  
  return (0 == tbd->stack.count);
}




/** Initialize the tbd inside the memory bounds in init structure.
 */
tbd_t* tbd_init(const tbd_init_t* init)
{
  TBD_ASSERT(init);
  
  /* check for non-null pointer */
  if (!init)
  {
    return 0;
  }
  
  /* check for enough room to store tbd_struct */
  if (init->size < sizeof(struct tbd_struct))
  {
    return 0;
  }
  
  tbd_t* tbd = init->start;
  
  tbd_clear(tbd);
  
  tbd->size = init->size;
  tbd->hunk_size = init->hunk_size;
  
  // locate keyvalue list immediately after tbd struct
  tbd->stack.start = (tbd_keyvalue_t*) (tbd + 1);
  tbd->stack.count = 0;
  
  // locate start of heap at end of allocated region
  tbd->heap.top = ((unsigned char*) init->start) + init->size;
  tbd->heap.size = 0;
  
  return tbd;
}








/*
 *
 * GARBAGE COLLECTION FUNCTIONS
 *
 */




/** Returns size in bytes of garbage in tbd.
 */
size_t tbd_garbage_size(const tbd_t* tbd)
{
  TBD_ASSERT(tbd);
  
  size_t result = 0;
  
#ifdef TBD_USE_GARBAGE_LIST  
  
  tbd_keyvalue_t* ptr = tbd->garbage.front;
  
  if (ptr)
  {
    result = tbd_keyvalue_garbage_list_size(ptr); 
  }
  
#else
  
  if (tbd_keyvalue_stack_count(&tbd->stack))
  {
    tbd_keyvalue_stack_iterator_t stack_ptr = tbd_keyvalue_stack_begin(&tbd->stack);
    tbd_keyvalue_stack_const_iterator_t stack_end = tbd_keyvalue_stack_end(&tbd->stack);
    
    while (!tbd_keyvalue_stack_const_iterator_is_equal(&stack_end, &stack_ptr))
    {
      tbd_keyvalue_stack_iterator_next(&stack_ptr);  
    }    
  }
  
#endif
  
  return result;
}




/** Returns number of garbage keyvalue elements in tbd.
 */
size_t tbd_garbage_count(const tbd_t* tbd)
{
  TBD_ASSERT(tbd);  
  
  #if defined(TBD_USE_GARBAGE_LIST)
    return tbd_garbage_list_count();
  #else
   
    // TODO iterate over keyvalue stack and count garbage
    return 0;
  
  #endif
}




size_t tbd_garbage_pop(tbd_t* tbd, size_t garbage_limit)
{
  TBD_ASSERT(tbd);
  
  size_t pop_total = 0;
  
  if (!garbage_limit)
  {
    return 0;
  }
  
#if defined(TBD_USE_GARBAGE_LIST)
  
  if (!tbd->garbage.front)
  {
    return 0;
  }
  
  tbd_keyvalue_t* ptr = tbd_garbage_list_last(&tbd->garbage);
  
  while (ptr && (ptr->value.data == tbd->heap.top))
  {
    const size_t keyvalue_size = tbd_keyvalue_size(ptr);
    
    if ((pop_total + keyvalue_size) > garbage_limit)
    {
      break;
    }
    
    pop_total += keyvalue_size;
    
    size_t pop_offset = ptr->heap.size;
    tbd->heap.top += pop_offset;
    
    --tbd->stack.count;
    
    if (ptr == tbd->garbage.front)
    {
      tbd->garbage.front = NULL;
    }
    
    ptr = ptr->prev_garbage;
  }
  
#endif  
  
  return pop_total;
}




size_t tbd_garbage_fold(tbd_t* tbd, size_t garbage_limit)
{
  TBD_ASSERT(tbd);
  
  if (!garbage_limit)
  {
    return 0;
  }
  
  if (0 == tbd_garbage_size(tbd))
  {
    return 0;
  }
  
  size_t garbage_total = 0;

  tbd_keyvalue_stack_iterator_t top = tbd_keyvalue_stack_begin(&tbd->stack);
  tbd_keyvalue_stack_const_iterator_t top_end = tbd_keyvalue_stack_end(&tbd->stack);
  
  tbd_keyvalue_stack_reverse_iterator_t btm = tbd_keyvalue_stack_rbegin(&tbd->stack);
  tbd_keyvalue_stack_const_reverse_iterator_t btm_end = tbd_keyvalue_stack_rend(&tbd->stack);
  
  if (!(top.ptr || top_end.ptr || btm.ptr || btm_end.ptr))
  {
    return 0;
  }
  
  while ((garbage_total < garbage_limit) && (top.ptr != top_end.ptr) && (btm.ptr != btm_end.ptr))
  { 
    if (top.ptr->is_garbage)
    {
      tbd_keyvalue_stack_iterator_next(&top);
      continue;  
    }
    
    if (!btm.ptr->is_garbage)
    {
      tbd_keyvalue_stack_reverse_iterator_next(&btm);
      continue;
    }
    
    tbd_keyvalue_stack_iterator_t temp_top = top;
    
    while (temp_top.ptr != top_end.ptr)
    {
      if (temp_top.ptr->is_garbage)
      {
        tbd_keyvalue_stack_iterator_next(&temp_top);
        continue;  
      }      
      
      size_t garbage_size = tbd_keyvalue_size(temp_top.ptr);
      
      if (garbage_total + garbage_size > garbage_limit)
      {
        tbd_keyvalue_stack_iterator_next(&temp_top);
        continue;
      }
      
      if (btm.ptr->heap.size == temp_top.ptr->heap.size)
      {
        garbage_total += tbd_keyvalue_copy(btm.ptr, temp_top.ptr);
        
        btm.ptr->is_garbage = false;
        temp_top.ptr->is_garbage = true;
        
#if defined(TBD_USE_GARBAGE)
        
        temp_top.ptr->next_garbage = btm.ptr->next_garbage;
        temp_top.ptr->prev_garbage = btm.ptr->prev_garbage;  
        
        if (tbd->garbage.front == btm.ptr)
        {
          tbd->garbage.front = temp_top.ptr;
        }
        
        if (tbd->garbage.back == btm.ptr)
        {
          tbd->garbage.back = temp_top.ptr;
        }
#endif
        
      }
      
      tbd_keyvalue_stack_iterator_next(&temp_top);      
    }
    
    tbd_keyvalue_stack_iterator_next(&top);
    tbd_keyvalue_stack_reverse_iterator_next(&btm);
  }
  
  return garbage_total;
}




/** Pack heap elements.
 *  Process adjacent elements.  If lower element is garbage and upper element is used, 
 *  then copy upper element and align with bottom of lower element
 */
size_t tbd_garbage_pack(tbd_t* tbd, size_t garbage_limit)
{
  TBD_ASSERT(tbd);
  
  if (!garbage_limit)
  {
    return 0;
  }
  
  if (!tbd->stack.count)
  {
    return 0;
  }
  
  int garbage_total = 0;
  
  tbd_keyvalue_stack_reverse_iterator_t dest = tbd_keyvalue_stack_rbegin(&tbd->stack);
  const tbd_keyvalue_stack_const_reverse_iterator_t end = tbd_keyvalue_stack_rend(&tbd->stack);
    
  if (!dest.ptr || !end.ptr)
  {
    return 0;
  }

  tbd_keyvalue_stack_reverse_iterator_t src = dest;  
  tbd_keyvalue_stack_reverse_iterator_next(&src);
  
  while(src.ptr != end.ptr)
  {
    if (dest.ptr->is_garbage && !src.ptr->is_garbage)
    {
      const size_t src_size = src.ptr->heap.size;
      const size_t dest_size = dest.ptr->heap.size;
      
      // TODO handle situation where src and dest overlap
      
      dest.ptr->heap.top = dest.ptr->heap.top + dest.ptr->heap.size - src_size;
      dest.ptr->heap.size = src_size;
      
      tbd_keyvalue_copy(dest.ptr, src.ptr);
      
      src.ptr->heap.size = dest_size;
      
#if defined(TBD_USE_GARBAGE_LIST)      
      tbd_garbage_list_delete(&tbd->garbage, dest.ptr);
      tbd_garbage_list_insert(&tbd->garbage, src.ptr);    
#endif
    }
    
    tbd_keyvalue_stack_reverse_iterator_next(&dest);    
    tbd_keyvalue_stack_reverse_iterator_next(&src);
  }
  
  return garbage_total;
}




size_t tbd_garbage_collect(tbd_t* tbd, size_t garbage_limit)
{
  TBD_ASSERT(tbd);
  
  size_t collected_size = 0;
  size_t total_collected_size = 0;
  
  if (!garbage_limit)
  {
    return total_collected_size = 0;
  }
  
  if (0 == tbd_garbage_size(tbd))
  {
    return total_collected_size;
  }
  
  // Get easy collection from garbage at top of heap
  collected_size = tbd_garbage_pop(tbd, garbage_limit);
  total_collected_size += collected_size;
  
  if (garbage_limit <= collected_size)
  {
    return total_collected_size;
  }
  
  garbage_limit -= collected_size;
  
  // fold data into garbage areas
  collected_size = tbd_garbage_fold(tbd, garbage_limit);  
  total_collected_size += collected_size;
  
  if (garbage_limit <= collected_size)
  {
    return total_collected_size;
  }
  
  garbage_limit -= collected_size;  
  
  // pack remaining data
  collected_size = tbd_garbage_pack(tbd, garbage_limit);
  total_collected_size += collected_size;
  
  return total_collected_size;
}




size_t tbd_garbage_clean(tbd_t* tbd)
{
  TBD_ASSERT(tbd);
  
  const size_t garbage_size = tbd_garbage_size(tbd);
  
  return tbd_garbage_collect(tbd, garbage_size);
}







/* 
 * 
 * Statistics and other general info.
 *
 */


void tbd_stats_get(tbd_stats_t* stats, const tbd_t* tbd)
{
  TBD_ASSERT(tbd);
  TBD_ASSERT(stats);
  
  stats->tbd_address = (unsigned) tbd;
  stats->tbd_size = tbd_size(tbd);
  stats->tbd_size_used = tbd_size_used(tbd);
  stats->tbd_head_size = tbd_head_size(tbd);
  
  stats->tbd_keyvalue_size = sizeof (tbd_keyvalue_t);
  
  stats->stack_top = (unsigned) tbd_keyvalue_stack_top(&tbd->stack);
  stats->stack_btm = (unsigned) tbd->stack.start;
  stats->stack_count = tbd->stack.count;
  stats->stack_size = tbd->stack.count * sizeof(tbd_keyvalue_t);
  
  stats->heap_top = (unsigned) tbd->heap.top;
  stats->heap_size = tbd->heap.size;

  stats->garbage_size = tbd_garbage_size(tbd);
  stats->garbage_count = tbd_garbage_count(tbd);

#if defined(TBD_USE_GARBAGE_LIST)
  stats->garbage_front = (unsigned) tbd->garbage.front;
  stats->garbage_back = (unsigned) tbd->garbage.back; 
#endif
}




int tbd_stats_print(const tbd_stats_t* stats)
{
  TBD_ASSERT(stats);
  
  int total_printed = 0;
  
  
#ifdef TBD_INCLUDE_STDIO
  
  total_printed += puts("{");
  total_printed += printf("\ttbd_address:\t0x%0X,\n", stats->tbd_address);
  total_printed += printf("\ttbd_size:\t0x%0X,\n", (unsigned) stats->tbd_size);
  total_printed += printf("\ttbd_size_used:\t0x%0X,\n", (unsigned) stats->tbd_size_used);
  total_printed += printf("\ttbd_head_size:\t0x%0X,\n", (unsigned) stats->tbd_head_size);
  
  total_printed += printf("\ttbd_keyvalue_size:\t0x%0X,\n", (unsigned) stats->tbd_keyvalue_size); 
  
  total_printed += printf("\tstack_top:\t0x%0X,\n", (unsigned) stats->stack_top);
  total_printed += printf("\tstack_btm:\t0x%0X,\n", (unsigned) stats->stack_btm);
  
  total_printed += printf("\tstack_count:\t0x%0X,\n", (unsigned) stats->stack_count);
  total_printed += printf("\tstack_size:\t0x%0X,\n", (unsigned) stats->stack_size);
  
  total_printed += printf("\theap_top:\t0x%0X,\n", (unsigned) stats->heap_top);  
  total_printed += printf("\theap_size:\t0x%0X,\n", (unsigned) stats->heap_size);  

  total_printed += printf("\tgarbage_front:\t0x%0X,\n", (unsigned) stats->garbage_front);  
  total_printed += printf("\tgarbage_back:\t0x%0X,\n", (unsigned) stats->garbage_back);   
  total_printed += printf("\tgarbage_size:\t0x%0X,\n", (unsigned) stats->garbage_size);  
  total_printed += printf("\tgarbage_count:\t0x%0X,\n", (unsigned) stats->garbage_count);  

  
  total_printed += puts("}");

#endif/*TBD_INCLUDE_STDIO*/  
  
  return total_printed;
}




int tbd_print_stats(const tbd_t* tbd)
{
  TBD_ASSERT(tbd);
  
  tbd_stats_t stats;
  
  tbd_stats_get(&stats, tbd);
  
  return tbd_stats_print(&stats);
}




/*
 *
 * JSON FUNCTIONS
 *
 */


static size_t tbd_key_to_json(char* json, size_t json_size, const tbd_key_t* key, TBD_KEY_TO_JSON_FORMAT_ENUM format)
{
  TBD_ASSERT(json);
  TBD_ASSERT(json_size);
  TBD_ASSERT(key);
  
  size_t total_printed = 0;
  
#ifdef TBD_INCLUDE_STDIO  
  
  size_t printed = snprintf(json, json_size, "%s", key->str); 
  
  total_printed += printed;
  
#endif
  
  return total_printed;
}




static size_t tbd_value_to_json(char* json, size_t json_size, const tbd_value_t* value, TBD_VALUE_TO_JSON_FORMAT_ENUM format)
{
  TBD_ASSERT(json);
  TBD_ASSERT(json_size);
  TBD_ASSERT(value);
  
  size_t total_printed = 0;

#ifdef TBD_INCLUDE_STDIO
  
  size_t printed = 0;
    
  const size_t value_size = tbd_value_size(value);
  
  switch(format)
  {
    case TBD_VALUE_TO_JSON_FORMAT_RAW:
    {
      if (value_size < json_size)
      {
        memcpy(json, value->data, value_size);
      }
      printed = value_size;
      total_printed += printed;
    } break;
      
    case TBD_VALUE_TO_JSON_FORMAT_HEX:  
    default:
    {
      printed = snprintf(json, json_size, "'");
      
      if (printed)
      {
        json += printed;
        json_size -= printed;
        total_printed += printed;
      }
      size_t data_size = value_size;
      unsigned char* data = value->data;
      
      while (data_size)
      {
        printed = snprintf(json, json_size, "%X", *data);
        json += printed;
        json_size -= printed;
        total_printed += printed;
        
        ++data;
        --data_size;
      }
      
      printed = snprintf(json, json_size, "'");
      json += printed;
      json_size -= printed;
      total_printed += printed;      
    }
  }

#endif//TBD_INCLUDE_STDIO  
  
  return total_printed;
}




size_t tbd_keyvalue_to_json(char* json, size_t json_size, const tbd_t* tbd, const char* key, TBD_KEY_TO_JSON_FORMAT_ENUM key_format, TBD_VALUE_TO_JSON_FORMAT_ENUM value_format)
{
  TBD_ASSERT(json);
  TBD_ASSERT(json_size);
  TBD_ASSERT(tbd);
  TBD_ASSERT(key);

  size_t total_printed = 0;
  
#ifdef TBD_INCLUDE_STDIO  
  
  // find the element
  const tbd_keyvalue_t* ptr = tbd_keyvalue_find_const(tbd, key);
  if (!ptr)
  {
    return 0;
  }  
  
  size_t printed = 0;

  const size_t key_size = tbd_key_size(&ptr->key);

  if (key_size)
  {
    printed = tbd_key_to_json(json, json_size, &ptr->key, key_format);

    total_printed += printed;
    json += printed;
    json_size -= printed;
    
    printed = snprintf(json, json_size, ":");    
    total_printed += printed;
    json += printed;
    json_size -= printed;
  }
  
  printed = tbd_value_to_json(json, json_size, &ptr->value, value_format);
  
  total_printed += printed;
  json += printed;
  json_size -= printed;
  
#endif//TBD_INCLUDE_STDIO  
  
  return total_printed;
}




size_t tbd_to_json(char* json, size_t json_size, const tbd_t* tbd, TBD_KEY_TO_JSON_FORMAT_ENUM key_format, TBD_VALUE_TO_JSON_FORMAT_ENUM value_format)
{
  TBD_ASSERT(json);
  TBD_ASSERT(json_size);
  TBD_ASSERT(tbd);
  
  
  size_t total_printed = 0;
  
#ifdef TBD_INCLUDE_STDIO  
  
  tbd_keyvalue_t* keyvalue_ptr = tbd->stack.start;
  
  size_t printed = 0;

  const tbd_keyvalue_t* keyvalue_end = tbd_keyvalue_stack_top(&tbd->stack);
  
  if (keyvalue_ptr != keyvalue_end)
  {
    // go to first item
    while (keyvalue_ptr != keyvalue_end)
    {
      if (!keyvalue_ptr->is_garbage)
        break;
      
      ++keyvalue_ptr;
    }
  
    if (keyvalue_ptr != keyvalue_end)
    {
      // print the first item
      printed = tbd_keyvalue_to_json(json, json_size, tbd, keyvalue_ptr->key.str, key_format, value_format);
      
      json += printed;
      json_size -= printed;
      
      ++keyvalue_ptr;
      
      // print the rest
      while (keyvalue_ptr != keyvalue_end)
      {
        printed = snprintf(json, json_size, ",");

        json += printed;
        json_size -= printed;
        
        printed = tbd_keyvalue_to_json(json, json_size, tbd, keyvalue_ptr->key.str, key_format, value_format);
      
        json += printed;
        json_size -= printed;
      
        ++keyvalue_ptr;
      }
    }
  }

#endif//TBD_INCLUDE_STDIO
  
  return total_printed;
}