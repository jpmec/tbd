/** tbd.c
 *  
 *  Tiny Basic Datastore.
 *
 *  Implementation of tbd.  All tbd confinguration and functionality is placed in this file.
 *
 */


#include "tbd.h"

#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>




#define TBD_ASSERT(_arg_)    assert(_arg_)








/** Key reference structure.
 */
typedef struct tbd_key_struct
{
  char* str;       /**< Pointer to key string data. */
  size_t size;     /**< Size in bytes of key string data. */
  
} tbd_key_t;




static void tbd_key_clear(tbd_key_t* key)
{
  TBD_ASSERT(key);
  
  key->str = NULL;
  key->size = 0;
}







/** Value reference structure.
 */
typedef struct tbd_value_struct
{
  void* data;     /**< Pointer to generic stored data. */
  size_t size;    /**< Size in bytes of generic stored data. */
  
} tbd_value_t;




static void tbd_value_clear(tbd_value_t* value)
{
  TBD_ASSERT(value);
  
  value->data = NULL;
  value->size = 0;
}








/** Heap structure.
 *  Represents range of contiguous memory.
 */
typedef struct tbd_heap_struct
{
  void* top;       ///< Pointer to top of heap.
  size_t size;     ///< Size in bytes of allocated heap. 
  
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
  
  return heap->size == 0;
}




static void* tbd_heap_push(tbd_heap_t* heap, size_t hunk_size)
{
  TBD_ASSERT(heap);
  
  heap->size += hunk_size;
  heap->top -= hunk_size / sizeof(void*);  // heap grows into lower memory addresses
  
  return heap->top;
}




static void* tbd_heap_pop(tbd_heap_t* heap, size_t hunk_size)
{
  TBD_ASSERT(heap);
  
  heap->size -= hunk_size;
  heap->top += hunk_size / sizeof(void*);  // heap grows into lower memory addresses
  
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
  struct tbd_keyvalue_struct* prev_garbage;    ///< Pointer to previous element before this that is garbage.
  struct tbd_keyvalue_struct* next_garbage;    ///< Pointer to next element after this that is garbage.
  
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
  keyvalue->prev_garbage = NULL;
  keyvalue->next_garbage = NULL;
}




/** Get the number of bytes used by the given keyvalue.
 */
static size_t tbd_keyvalue_size(const tbd_keyvalue_t* keyvalue)
{
  TBD_ASSERT(keyvalue);
  
  return keyvalue->heap.size + sizeof(tbd_keyvalue_t);
}




/** Copy keyvalue data from src to dest.
 */
static size_t tbd_keyvalue_copy(tbd_keyvalue_t* dest, const tbd_keyvalue_t* src)
{
  TBD_ASSERT(dest);
  TBD_ASSERT(src);
  
  // copy heap data
  memcpy(dest->value.data, src->value.data, src->value.size);
  memcpy(dest->key.str, src->key.str, src->key.size);
  
  // copy stack data
  *dest = *src;
  
  return tbd_keyvalue_size(src);
}




static size_t tbd_keyvalue_garbage_size(const tbd_keyvalue_t* keyvalue)
{
  size_t total_size = 0;
  
  while(keyvalue)
  {
    total_size += tbd_keyvalue_size(keyvalue);
    keyvalue = keyvalue->next_garbage;
  }
  
  return total_size;
}








/** A simple stack structure.
 */
typedef struct tbd_keyvalue_stack_struct
{
  struct tbd_keyvalue_struct* start;    ///< Pointer to start of keyvalue stack.
  size_t count;                         ///< Number of elements in keyvalue stack.  
  
} tbd_keyvalue_stack_t;




static void tbd_keyvalue_stack_clear(tbd_keyvalue_stack_t* stack)
{
  TBD_ASSERT(stack);
  
  stack->start = NULL;
  stack->count = 0;
}




static void tbd_keyvalue_stack_empty(tbd_keyvalue_stack_t* stack)
{
  TBD_ASSERT(stack);
  
  stack->count = 0;
}




static tbd_keyvalue_t* tbd_keyvalue_stack_top(const tbd_keyvalue_stack_t* stack)
{
  TBD_ASSERT(stack);
  
  return stack->start + stack->count;
}




static tbd_keyvalue_t* tbd_keyvalue_stack_get(const tbd_keyvalue_stack_t* stack, size_t index)
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







/** Garbage list structure.
 */
typedef struct tbd_garbage_list_struct
{
  struct tbd_keyvalue_struct* front;   ///< Pointer to the front of a list of elements that can are garbage (can be garbage collected). 
  struct tbd_keyvalue_struct* back;    ///< Pointer to the back of a list of elements that can are garbage (can be garbage collected).  
 
} tbd_garbage_list_t;




static void tbd_garbage_list_clear(tbd_garbage_list_t* garbage)
{
  TBD_ASSERT(garbage);
  
  garbage->front = NULL;
  garbage->back = NULL;
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
  }
  else
  {
    // find the insertion point
    tbd_keyvalue_t* prev = garbage->front;
    tbd_keyvalue_t* next = garbage->front;
    
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
    
    if (next != prev)
    {
      next->prev_garbage = keyvalue_last;
      keyvalue_last->next_garbage = next;
    }
  }
}




static void tbd_garbage_list_delete(tbd_garbage_list_t* garbage, tbd_keyvalue_t* keyvalue)
{
  TBD_ASSERT(garbage);
  TBD_ASSERT(keyvalue);
  
  if (garbage->front)
  {
    return;
  }
  
  if (garbage->front == keyvalue)
  {
    garbage->front = keyvalue->next_garbage;
    keyvalue->prev_garbage = NULL;
    return;
  }
  
  tbd_keyvalue_t* prev = garbage->front;  
  tbd_keyvalue_t* next = prev->next_garbage;
  
  while ((next) && (next != keyvalue))
  {
    prev = next;
    next = next->next_garbage;
  }
  
  if (next != keyvalue)
  {
    return;
  }
  
  prev->next_garbage = keyvalue->next_garbage;
  next->prev_garbage = keyvalue->prev_garbage;
}








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
  size_t size;         ///< Total size in bytes of the allocated datastore in bytes.
  size_t hunk_size;    ///< Hunk size for the datastore, this is the minimum size that is allocated from the heap.
  
  tbd_garbage_list_t garbage;    ///< The garbage list.
  tbd_keyvalue_stack_t stack;    ///< The keyvalue stack.
  tbd_heap_t heap;               ///< The data heap.
};




/** Compute number of bytes needed from heap to store key and value.
 */
static size_t tbd_keyvalue_hunk_size(const tbd_t* tbd, size_t key_size, size_t value_size)
{
  TBD_ASSERT(tbd);
  
  return (((key_size + value_size) / tbd->hunk_size) + 1) * tbd->hunk_size;
}




/** Create a new keyvalue with the given sizes.
 */
static struct tbd_keyvalue_struct* tbd_create_keyvalue(tbd_t* tbd, size_t key_size, size_t value_size)
{
  TBD_ASSERT(tbd);
  TBD_ASSERT(tbd->hunk_size);
  
  const size_t hunk_size = tbd_keyvalue_hunk_size(tbd, key_size, value_size);  
  
  // check there is enough room to add elements of that size
  tbd_keyvalue_t* stack_top = tbd_keyvalue_stack_push(&tbd->stack);
  void* heap_top = tbd_heap_push(&tbd->heap, hunk_size);
  
  if (heap_top < (void*) stack_top)
  {
    tbd_keyvalue_stack_pop(&tbd->stack);
    tbd_heap_pop(&tbd->heap, hunk_size);
    return 0;
  }
    
  // allocate from heap
  stack_top->heap.top = heap_top;  
  stack_top->heap.size = hunk_size;
  
  stack_top->value.data = heap_top;
  stack_top->value.size = value_size;
  
  stack_top->key.str = heap_top + value_size / sizeof(void*);
  stack_top->key.size = key_size;
  
  // initialize garbage list node
  stack_top->is_garbage = false;
  stack_top->next_garbage = NULL;
  stack_top->prev_garbage = NULL;
  
  return stack_top;
}




static struct tbd_keyvalue_struct* tbd_keyvalue_find(const tbd_t* tbd, const char* key)
{
  TBD_ASSERT(tbd);
  
  // a simple linear search for a given key
  // TODO convert to standard C search
  struct tbd_keyvalue_struct* keyvalue_ptr = tbd->stack.start;
  
  const tbd_keyvalue_t* end = tbd_keyvalue_stack_top(&tbd->stack);
  
  while (keyvalue_ptr != end)
  {
    if (!keyvalue_ptr->is_garbage && (strcmp(key, keyvalue_ptr->key.str) == 0))
    {
      return keyvalue_ptr;
    }
    
    ++keyvalue_ptr;
  }
  
  return 0;
}




size_t tbd_size(const tbd_t* tbd)
{
  TBD_ASSERT(tbd);
  
  return tbd->size;
}




size_t tbd_size_used(const tbd_t* tbd)
{
  TBD_ASSERT(tbd);
  
  return sizeof(tbd_t) + tbd->stack.count * sizeof(tbd_keyvalue_t) + tbd->heap.size;
}




int tbd_create(tbd_t* tbd, const char* key, const void* value, size_t value_size)
{
  TBD_ASSERT(tbd);
  TBD_ASSERT(key);
  TBD_ASSERT(value);
  TBD_ASSERT(value_size);
  
  
  // check that an element with given key does not already exist
  if (tbd_keyvalue_find(tbd, key))
  {
    return 0;
  }
  
  const size_t key_size = strlen(key) + 1;
  
  struct tbd_keyvalue_struct* keyvalue = tbd_create_keyvalue(tbd, key_size, value_size);
  
  if (!keyvalue)
  {
    return 0;
  }
  
  // copy the data into the newly allocated memory
  memcpy(keyvalue->key.str, key, key_size);
  memcpy(keyvalue->value.data, value, value_size);
  
  return TBD_NO_ERROR;   
}




int tbd_read(tbd_t* tbd, const char* key, void* value, size_t value_size)
{
  // find the element
  struct tbd_keyvalue_struct* ptr = tbd_keyvalue_find(tbd, key);
  if (!ptr)
  {
    return TBD_ERROR;
  }
  
  // compute minimum copy size, truncate to value_size
  const size_t copy_size = (ptr->value.size < value_size) ? ptr->value.size: value_size;
  
  // copy the data to the value
  memcpy(value, ptr->value.data, copy_size);
  
  return TBD_NO_ERROR;
}




int tbd_update(tbd_t* tbd, const char* key, const void* value, size_t value_size)
{
  // find the element
  struct tbd_keyvalue_struct* ptr = tbd_keyvalue_find(tbd, key);
  if (!ptr)
  {
    return TBD_ERROR;
  }
  
  // compute minimum copy size, truncate to value_size
  const size_t copy_size = (ptr->value.size < value_size) ? ptr->value.size: value_size;
  
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
  
  ptr->is_garbage = true;
  tbd_garbage_list_insert(&tbd->garbage, ptr);
  
  return TBD_NO_ERROR;
}




void tbd_clear(tbd_t* tbd)
{
  TBD_ASSERT(tbd);

  tbd_keyvalue_stack_clear(&tbd->stack);
  tbd_heap_clear(&tbd->heap);
  tbd_garbage_list_clear(&tbd->garbage);
}




void tbd_empty(tbd_t* tbd)
{
  TBD_ASSERT(tbd);

  tbd_keyvalue_stack_empty(&tbd->stack);
  tbd_heap_empty(&tbd->heap);
  tbd_garbage_list_clear(&tbd->garbage);
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
  
  // locate start of heap at end of allocated region
  tbd->heap.top = init->start + init->size / sizeof(void*);
  
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
  size_t result = 0;
  
  tbd_keyvalue_t* ptr = tbd->garbage.front;
  
  if (ptr)
  {
    result = tbd_keyvalue_garbage_size(ptr); 
 }
  
  return result;
}




size_t tbd_garbage_pop(tbd_t* tbd, size_t garbage_limit)
{
  TBD_ASSERT(tbd);
  
  size_t pop_total = 0;
  
  if (!garbage_limit)
  {
    return 0;
  }
  
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
    
    size_t pop_offset = ptr->heap.size / sizeof(void*);
    tbd->heap.top += pop_offset;
    
    --tbd->stack.count;
    
    if (ptr == tbd->garbage.front)
    {
      tbd->garbage.front = NULL;
    }
    
    ptr = ptr->prev_garbage;
  }
  
  return pop_total;
}




size_t tbd_garbage_fold(tbd_t* tbd, size_t garbage_limit)
{
  TBD_ASSERT(tbd);
  
  if (!garbage_limit)
  {
    return 0;
  }
  
  if (!tbd->garbage.front)
  {
    return 0;
  }
  
  size_t garbage_total = 0;
  size_t stack_index = 0; 

  
  while (garbage_total < garbage_limit)
  {
    tbd_keyvalue_t* top = tbd_keyvalue_stack_top(&tbd->stack);
    tbd_keyvalue_t* btm = tbd_keyvalue_stack_get(&tbd->stack, stack_index);
    
    if (btm->is_garbage)
    {
      if (!top->is_garbage && (btm->heap.size >= top->heap.size))
      {
        tbd_keyvalue_copy(btm, top);
//        tbd_keyvalue_delete(top);
        
        garbage_total += top->heap.size;
      
        // copy heap data
        memcpy(btm->value.data, top->value.data, top->value.size);
        memcpy(btm->key.str, top->key.str, top->key.size);

        const size_t heap_offset = top->heap.size / sizeof(void*);      
        tbd->heap.top -= heap_offset;
    
        // copy stack data
        *btm = *top;
        --tbd->stack.count;
      }
    }
    ++stack_index;
  }
  
  return garbage_total;
  
}




size_t tbd_garbage_pack(tbd_t* tbd, size_t garbage_limit)
{
  TBD_ASSERT(tbd);
  
  if (!garbage_limit)
  {
    return 0;
  }
  
  return 0;
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
  
  if (!tbd->garbage.front)
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


void tbd_get_stats(const tbd_t* tbd, tbd_stats_t* stats)
{
  TBD_ASSERT(tbd);
  TBD_ASSERT(stats);
}




void tbd_print_stats(const tbd_stats_t* stats)
{
  TBD_ASSERT(stats);
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
  
  size_t printed = 0;
  size_t total_printed = 0;
  
  printed = snprintf(json, json_size, "%s", key->str); 
  
  total_printed += printed;
  
  return total_printed;
}




static size_t tbd_value_to_json(char* json, size_t json_size, const tbd_value_t* value, TBD_VALUE_TO_JSON_FORMAT_ENUM format)
{
  TBD_ASSERT(json);
  TBD_ASSERT(json_size);
  TBD_ASSERT(value);
  
  size_t printed = 0;
  size_t total_printed = 0;

  switch(format)
  {
    case TBD_VALUE_TO_JSON_FORMAT_RAW:
    {
      if (value->size < json_size)
      {
        memcpy(json, value->data, value->size);
      }
      printed = value->size;
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
      size_t data_size = value->size;
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

  return total_printed;
}




size_t tbd_keyvalue_to_json(char* json, size_t json_size, const tbd_t* tbd, const char* key, TBD_KEY_TO_JSON_FORMAT_ENUM key_format, TBD_VALUE_TO_JSON_FORMAT_ENUM value_format)
{
  TBD_ASSERT(json);
  TBD_ASSERT(json_size);
  TBD_ASSERT(tbd);
  TBD_ASSERT(key);
  
  // find the element
  struct tbd_keyvalue_struct* ptr = tbd_keyvalue_find(tbd, key);
  if (!ptr)
  {
    return 0;
  }  
  
  size_t printed = 0;
  size_t total_printed = 0;

  if (ptr->key.size)
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
  
  
  return total_printed;
}




size_t tbd_to_json(char* json, size_t json_size, const tbd_t* tbd, TBD_KEY_TO_JSON_FORMAT_ENUM key_format, TBD_VALUE_TO_JSON_FORMAT_ENUM value_format)
{
  tbd_keyvalue_t* keyvalue_ptr = tbd->stack.start;
  
  size_t printed = 0;
  size_t total_printed = 0;
  
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
  return total_printed;
}