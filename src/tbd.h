/** tbd.h
 *  
 * Tiny Bare-metal Datastore
 * 
 */



/* Forward declarations for opaque structures. */
struct tbd_struct;


/* Type definitions for opaque structures. */
typedef struct tbd_struct tbd_t;




/** Structure for initializing tbd.
 */
typedef struct tbd_init_struct
{
  void* start;
  unsigned size;
  
} tbd_init_t;




/** Initialize tbd.
 *  Returns opaque pointer to tbd_struct.
 */
tbd_t* tbd_init(const tbd_init_t* init);


/** Add an element to the data store.
 *  Returns 0 if successful.
 */
int tbd_create(tbd_t* tbd, const char* key, const void* value, unsigned size);


/** Get an element from the data store.
 *  Returns 0 if successful.
 */
int tbd_read(tbd_t* tbd, const char* key, void* value, unsigned size);


/** Update an existing element in the data store.
 *  Returns 0 if successful.
 */
int tbd_update(tbd_t* tbd, const char* key, const void* value, unsigned size);


/** Delete an existing element in the data store.
 *  Returns 0 if successful.
 */
int tbd_delete(tbd_t* tbd, const char* key);



