/** tbd.h
 *  
 * Tiny Basic Datastore
 * 
 * The tbd library has the following design features:
 *
 * - Used as a datastore inside a user-defined contiguous block of memory.
 * - Supports CRUD operations (Create, Read, Update, Delete).
 * - Written in C.  Requires C99 support.
 * - Only uses standard C library.
 * - Does not require any file I/O.
 * - Does not require calls to malloc() or free().
 * - Is serializeable in JSON format.
 *
 * The tbd library stores data in key-value pairs.
 *
 * Garbage collection is used to reclaim key-value pairs that are no longer used.  A variety of garbage collection functions are provided.
 *
 * By default when compiling with NDEBUG, parameters passed a function arguments are not checked for NULL or invalid values.
 * Uses assert() to detect null pointer conditions when compiling in debug mode.
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
 */


#ifndef _TBD_H_
#define _TBD_H_


#include <stddef.h>
#include <stdlib.h>




/** Maximum number of characters for a tbd key.  Includes null terminator.
 */
#define TBD_SIZE_T            size_t
#define TBD_MAX_SIZE          (0x8000u)
#define TBD_MAX_KEY_LENGTH    (8u)




/* Forward declarations for opaque structures. */
struct tbd_struct;
struct tbd_const_iterator_struct;


/* Type definitions for opaque structures. */
typedef struct tbd_struct tbd_t;
typedef struct tbd_const_iterator_struct tbd_const_iterator_t;




/* Error codes
 * All error codes are less than 0.
 */
#define TBD_NO_ERROR             (0)
#define TBD_ERROR               (-1)
#define TBD_ERROR_KEY_NOT_FOUND (-2)
#define TBD_ERROR_KEY_EXISTS    (-3)
#define TBD_ERROR_BAD_SIZE      (-4)








/** Structure for initializing tbd.
 */
typedef struct tbd_init_struct
{
  void* start;       ///< Start of the tbd.
  TBD_SIZE_T size;       ///< Size in bytes of the tbd. 
  TBD_SIZE_T hunk_size;  ///< The minimum size allocated from tbd heap. 
  
} tbd_init_t;




/** Initialize tbd.
 *  Returns opaque pointer to tbd_struct.
 */
tbd_t* tbd_init(const tbd_init_t* init);


/** Get the tbd library version.
 */
int tbd_version(void);


/** Returns 1 if the value represents a tbd error code,
 *  Returns 0 otherwise.
 */
int tbd_is_error(int value);


/** Clear a tdb, all data including stack and heap locations will be lost.
 */
void tbd_clear(tbd_t* tbd);


/** Empty a tbd.  Deletes all key:value pairs. 
 *  Does not clear stack and heap locations. 
 */
void tbd_empty(tbd_t* tbd);


/** Returns 1 if the tbd is empty,
 *  Returns 0 otherwise.
 */
int tbd_is_empty(const tbd_t* tbd);


/** Return total allocated size in bytes for the tbd.
 */
size_t tbd_size(const tbd_t* tbd);


/** Return number of bytes used by header information.
 */
size_t tbd_head_size(const tbd_t* tbd);


/** Return number of bytes used by the tbd.
 */
size_t tbd_size_used(const tbd_t* tbd);


/** Return number of keyvalues stored in the tbd.
 */
size_t tbd_count(const tbd_t* tbd);


/** Return maximum number of keyvalues of a given size that can be stored.
 */
TBD_SIZE_T tbd_max_count(const tbd_t* tbd, TBD_SIZE_T keyvalue_size);


/** Return the maximum key length.
 */
TBD_SIZE_T tbd_max_key_length(const tbd_t* tbd);


/** Copy data from one tbd to another.
 *  Returns TBD_NO_ERROR if successful.
 */
int tbd_copy(tbd_t* dest, const tbd_t* src);


/** Sort the tbd keyvalues in key order.
 */
int tbd_sort_by_key(tbd_t* tbd);


/** Sort the tbd keyvalues in heap order.
 */
int tbd_sort_by_heap(tbd_t* tbd);








/* 
 * Basic CRUD operations
 */


/** Copy an element into to the data store.
 * 
 *  Returns TBD_ERROR_KEY_EXISTS if a key:value pair already exists in the data store.
 *  Returns TBD_NO_ERROR if successful, 
 */
int tbd_create(tbd_t* tbd, const char* key, const void* value, size_t value_size);


/** Get an element from the data store.
 *  Returns TBD_ERROR_KEY_NOT_FOUND if key does not exist.
 *  Returns TBD_NO_ERROR if successful.
 */
int tbd_read(tbd_t* tbd, const char* key, void* value, size_t value_size);


/** Update an existing element in the data store.
 *  
 *  Returns TBD_ERROR_BAD_SIZE if value_size does not match size of element in data store.
 *  Returns TBD_NO_ERROR if successful.
 */
int tbd_update(tbd_t* tbd, const char* key, const void* value, size_t value_size);


/** Delete an existing element in the data store.
 *
 *  Returns TBD_NO_ERROR if key does not exist.
 *  Returns TBD_NO_ERROR if successful.
 */
int tbd_delete(tbd_t* tbd, const char* key);








/* 
 * Advanced CRUD operations 
 */


/** Get the size of a value from the data store.
 */
size_t tbd_read_size(tbd_t* tbd, const char* key);








/*
 * Iterator operations
 *
 * Provides functions to iterate over the data store.
 * These operations are intended to be read-only.
 * When manipulating data store, use CRUD functions.
 * 
 */

struct tbd_const_iterator_struct
{
  const void* ptr;  ///< Nothing to see here, move along.
};   


/** Get forward iterator to first keyvalue element.
 */
tbd_const_iterator_t tbd_const_begin(const tbd_t* tbd);


/** Get forward iterator to end+1 keyvalue element.
 */
tbd_const_iterator_t tbd_const_end(const tbd_t* tbd);


/** Move iterator to next element.
 */
tbd_const_iterator_t tbd_const_iterator_next(tbd_const_iterator_t i);


/** Returns 1 if the two iterators represent the same element.
 */
int tbd_const_iterator_is_equal(tbd_const_iterator_t a, tbd_const_iterator_t b);


/** Get pointer to key pointed to by iterator.
 * Will return NULL if iterator is invalid.
 */
const char* tbd_const_iterator_key(const tbd_const_iterator_t i);


/** Get the size in bytes of the value pointed to by iterator.
 */
size_t tbd_const_iterator_value_size(const tbd_const_iterator_t i);


/** Get the pointer to the value pointd to by the iterator.
 */
const void* tbd_const_iterator_value(const tbd_const_iterator_t i);








/* 
 * Memory Management
 */


/** Return number of bytes of garbage.
 */
TBD_SIZE_T tbd_garbage_size(const tbd_t* tbd);


/** Number of keyvalues that are garbage.
 */
TBD_SIZE_T tbd_garbage_count(const tbd_t* tbd);


/** Combine keyvalues that are garbage and are located next to each other in the heap.
 *  Re-assigns values so lowest keyvalue stack element references garbage 
 *  and highest keyvalue stack element references none of the heap.
 *  This function is the most effective if tbd_sort_by_heap has been called prior to this function,
 *  however the tbd is not required to be in heap sorted order.
 */
TBD_SIZE_T tbd_garbage_merge(tbd_t* tbd);


/** Collect up to a given number of bytes of garbage from the top of the stack and heap.
 *  Will stop collecting when first used keyvalue is encountered.
 *
 *  This is the fastest type of garbage collection.
 *  No data moving is required.  Does not invalidate existing pointers.
 *
 *  TODO: is garbage_limit needed?
 */
TBD_SIZE_T tbd_garbage_pop(tbd_t* tbd, size_t garbage_limit);


/** Fold a given number of used bytes in garbage bytes.
 *  Will fold objects from the top of the heap into the lowest memory regions.
 *
 *  This is slow garbage collection because moving is required.
 *  Invalidates existing pointers.
 *  
 */
TBD_SIZE_T tbd_garbage_fold(tbd_t* tbd, size_t garbage_limit);


/** Pack up to a given number of bytes of garbage so that heap is contiguous.
 */
TBD_SIZE_T tbd_garbage_pack(tbd_t* tbd, size_t garbage_limit);


/** Collect up to a given number of bytes of garbage.
 *  This will pop, fold and pack garbage until limit is reached.
 */
TBD_SIZE_T tbd_garbage_collect(tbd_t* tbd, size_t garbage_limit);


/** Clean out all the garbage.
 *  This will collect all garbage.  tbd_garbage_size() should always be 0 after this function.
 */
TBD_SIZE_T tbd_garbage_clean(tbd_t* tbd);







/* 
 * Statistics and other general info.
 */

/** Struct to hold statistics about a tbd.
 */
typedef struct tbd_stats_struct
{
  const void* tbd_address;
  TBD_SIZE_T   tbd_size;
  TBD_SIZE_T   tbd_size_used;
  TBD_SIZE_T   tbd_head_size;
  
  TBD_SIZE_T   tbd_keyvalue_size;    ///< Size of a tbd_keyvalue_t element in bytes
  
  const void* stack_top;       ///< Address of top element.
  const void* stack_btm;       ///< Address of bottom element.
  TBD_SIZE_T   stack_count;     ///< Number of elements in stack.
  TBD_SIZE_T   stack_size;      ///< Size of stack in bytes.
  
  const void* heap_top;        ///< Top of heap.
  TBD_SIZE_T   heap_size;       ///< Size of heap in bytes.
  
  const void* garbage_front;   ///< First element of garbage.
  const void* garbage_back;    ///< Last element of garbage.
  TBD_SIZE_T garbage_size;      ///< Number of bytes of garbage.
  TBD_SIZE_T garbage_count;     ///< Number of garbage elements.
  
} tbd_stats_t;




/** Get all statistics from a tbd.
 */
void tbd_stats_get(tbd_stats_t* stats, const tbd_t* tbd);


/** Pretty-print statistics to standard out in JSON format.
 *  Returns number of characters printed.
 */
int tbd_stats_print(const tbd_stats_t* stats);


/** Pretty-print statistics to standard out in JSON format.
 *  Returns number of characters printed.
 */
int tbd_print_stats(const tbd_t* tbd);








/* 
 * JSON support
 */


typedef enum TBD_KEY_TO_JSON_FORMAT
{
  TBD_KEY_TO_JSON_FORMAT_RAW,
  TBD_KEY_TO_JSON_FORMAT_STRING,  
  
} TBD_KEY_TO_JSON_FORMAT_ENUM;




typedef enum TBD_VALUE_TO_JSON_FORMAT
{
  TBD_VALUE_TO_JSON_FORMAT_RAW,
  TBD_VALUE_TO_JSON_FORMAT_HEX,
  
} TBD_VALUE_TO_JSON_FORMAT_ENUM;




/** Convert tbd database to json format.  Writes to json string, result is null terminated.
 *  Returns number of bytes written.
 */
size_t tbd_to_json(char* json, size_t json_size, const tbd_t* tbd, TBD_KEY_TO_JSON_FORMAT_ENUM key_format, TBD_VALUE_TO_JSON_FORMAT_ENUM value_format);


/** Convert tbd keys to json formatted array.  Writes to json string, result is null terminated.
 *  Returns number of bytes written.
 */
size_t tbd_keys_to_json(char* json, size_t json_size, const tbd_t* tbd, TBD_KEY_TO_JSON_FORMAT_ENUM key_format);


/** Convert tbd garbage to json formatted array.  Writes to json string, result is null terminated.
 *  Returns number of bytes written.
 */
size_t tbd_garbage_list_to_json(char* json, size_t json_size, const tbd_t* tbd);


/** Convert a keyvalue pair to json format.
 */
size_t tbd_keyvalue_to_json(char* json, size_t json_size, const tbd_t* tbd, const char* key, TBD_KEY_TO_JSON_FORMAT_ENUM key_format, TBD_VALUE_TO_JSON_FORMAT_ENUM value_format);


/** Copy json string into tbd datastore.  Will overwrite old contents.
 */
int tbd_from_json(tbd_t* tbd, const char* json);








#endif//_TBD_H_
