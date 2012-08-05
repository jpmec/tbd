/** tbd.h
 *  
 * Tiny Basic Datastore
 * 
 * The tbd library has the following design features:
 *
 * - Used as a datastore inside a user-defined contiguous block of memory.
 * - Only uses standard C library
 * - Does not require any file I/O
 * - Is serializeable in JSON format
 *
 * The tbd library stores data in key-value pairs.
 *
 * Garbage collection is used to reclaim key-value pairs that are no longer used.
 *
 */


#ifndef _TBD_H_
#define _TBD_H_


#include <stddef.h>




/* Forward declarations for opaque structures. */
struct tbd_struct;


/* Type definitions for opaque structures. */
typedef struct tbd_struct tbd_t;



/* Error codes */
#define TBD_NO_ERROR    (0)
#define TBD_ERROR       (-1)




/** Structure for initializing tbd.
 */
typedef struct tbd_init_struct
{
  void* start;       ///< Start of the tbd.
  size_t size;       ///< Size in bytes of the tbd. 
  size_t hunk_size;  ///< The minimum size allocated from tbd heap. 
  
} tbd_init_t;




/** Initialize tbd.
 *  Returns opaque pointer to tbd_struct.
 */
tbd_t* tbd_init(const tbd_init_t* init);


/** Clear a tdb, all data including stack and heap locations will be lost.
 */
void tbd_clear(tbd_t* tbd);


/** Empty a tbd. 
 */
void tbd_empty(tbd_t* tbd);




/** Return total allocated size in bytes for the tbd.
 */
size_t tbd_size(const tbd_t* tbd);


/** Return number of bytes used by the tbd.
 */
size_t tbd_size_used(const tbd_t* tbd);


/* Basic CRUD operations
 */


/** Copy an element into to the data store.
 *  Returns TBD_NO_ERROR if successful, 
 */
int tbd_create(tbd_t* tbd, const char* key, const void* value, size_t value_size);


/** Get an element from the data store.
 *  Returns TBD_NO_ERROR if successful.
 */
int tbd_read(tbd_t* tbd, const char* key, void* value, size_t value_size);


/** Update an existing element in the data store.
 *  Returns TBD_NO_ERROR if successful.
 */
int tbd_update(tbd_t* tbd, const char* key, const void* value, size_t value_size);


/** Delete an existing element in the data store.
 *  Returns TBD_NO_ERROR if successful.
 */
int tbd_delete(tbd_t* tbd, const char* key);







/* Memory Management
 */

/** Return number of bytes of garbage.
 */
size_t tbd_garbage_size(const tbd_t* tbd);


/** Collect up to a given number of bytes of garbage.
 */
size_t tbd_garbage_pop(tbd_t* tbd, size_t garbage_limit);


/** Fold a given number of used bytes in garbage bytes.
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
  size_t   tbd_size;
  
  
  
} tbd_stats_t;




/** Get all statistics from a tbd.
 */
void tbd_get_stats(const tbd_t* tbd, tbd_stats_t* stats);


/** Pretty-print statistics to standard out in JSON format.
 */
void tbd_print_stats(const tbd_stats_t* stats);





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
