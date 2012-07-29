/** tbd.h
 *  
 * Tiny Basic Datastore
 * 
 * The tbd library has the following design features:
 *
 * - Used as a datastore inside a contiguous block of memory.
 * - Only uses standard C library
 * - Does not require any file I/O
 * - Is serializeable in JSON format
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
  void* start;
  size_t size;
  
} tbd_init_t;




/** Initialize tbd.
 *  Returns opaque pointer to tbd_struct.
 */
tbd_t* tbd_init(const tbd_init_t* init);


/** Clear a database, all data will be lost.
 */
void tbd_clear(tbd_t* tbd);


/** Copy an element into to the data store.
 *  Returns TBD_NO_ERROR if successful, 
 */
int tbd_create(tbd_t* tbd, const char* key, const void* value, size_t value_size);


/** Get an element from the data store.
 *  Returns 0 if successful.
 */
int tbd_read(tbd_t* tbd, const char* key, void* value, size_t value_size);


/** Update an existing element in the data store.
 *  Returns 0 if successful.
 */
int tbd_update(tbd_t* tbd, const char* key, const void* value, size_t value_size);


/** Delete an existing element in the data store.
 *  Returns 0 if successful.
 */
int tbd_delete(tbd_t* tbd, const char* key);




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
