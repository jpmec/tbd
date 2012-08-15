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
 * Garbage collection is used to reclaim key-value pairs that are no longer used.
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




/** Maximum number of characters for a tbd key.  Includes null terminator.
 */
#define TBD_SIZE_T            size_t
#define TBD_MAX_SIZE          ((TBD_SIZE_T) -1)
#define TBD_MAX_KEY_LENGTH    (8u)


/* Forward declarations for opaque structures. */
struct tbd_struct;


/* Type definitions for opaque structures. */
typedef struct tbd_struct tbd_t;




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
 * Memory Management
 */


/** Return number of bytes of garbage.
 */
size_t tbd_garbage_size(const tbd_t* tbd);


/** Number of keyvalues that are garbage.
 */
size_t tbd_garbage_count(const tbd_t* tbd);


/** Collect up to a given number of bytes of garbage from the top of the stack and heap.
 *  Will stop collecting when first used keyvalue is encountered.
 *
 *  This is the fastest type of garbage collection.
 *  No data moving is required.  Does not invalidate existing pointers.
 *
 *  TODO: is garbage_limit needed?
 */
size_t tbd_garbage_pop(tbd_t* tbd, size_t garbage_limit);


/** Fold a given number of used bytes in garbage bytes.
 *  Will fold objects from the top of the heap into the lowest memory regions.
 *
 *  This is slow garbage collection because moving is required.
 *  Invalidates existing pointers.
 *  
 */
size_t tbd_garbage_fold(tbd_t* tbd, size_t garbage_limit);


/** Pack up to a given number of bytes of garbage so that heap is contiguous.
 */
size_t tbd_garbage_pack(tbd_t* tbd, size_t garbage_limit);


/** Collect up to a given number of bytes of garbage.
 *  This will pop, fold and pack garbage until limit is reached.
 */
size_t tbd_garbage_collect(tbd_t* tbd, size_t garbage_limit);


/** Clean out all the garbage.
 *  This will collect all garbage.  tbd_garbage_size() should always be 0 after this function.
 */
size_t tbd_garbage_clean(tbd_t* tbd);







/* Statistics and other general info.
 */
typedef struct tbd_stats_struct
{
  unsigned tbd_address;
  TBD_SIZE_T   tbd_size;
  TBD_SIZE_T   tbd_size_used;
  TBD_SIZE_T   tbd_head_size;
  
  TBD_SIZE_T   tbd_keyvalue_size;    ///< Size of a tbd_keyvalue_t element in bytes
  
  unsigned stack_top;       ///< Address of top element.
  unsigned stack_btm;       ///< Address of bottom element.
  TBD_SIZE_T   stack_count;     ///< Number of elements in stack.
  TBD_SIZE_T   stack_size;      ///< Size of stack in bytes.
  
  unsigned heap_top;        ///< Top of heap.
  TBD_SIZE_T   heap_size;       ///< Size of heap in bytes.
  
  unsigned garbage_front;   ///< First element of garbage.
  unsigned garbage_back;    ///< Last element of garbage.
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




/* JSON support
 */



typedef enum TBD_KEY_TO_JSON_FORMAT
{
  TBD_KEY_TO_JSON_FORMAT_RAW
  
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


/** Convert a keyvalue pair to json format.
 */
size_t tbd_keyvalue_to_json(char* json, size_t json_size, const tbd_t* tbd, const char* key, TBD_KEY_TO_JSON_FORMAT_ENUM key_format, TBD_VALUE_TO_JSON_FORMAT_ENUM value_format);


/** Copy json string into tbd datastore.  Will overwrite old contents.
 */
int tbd_from_json(tbd_t* tbd, const char* json);






#endif//_TBD_H_
