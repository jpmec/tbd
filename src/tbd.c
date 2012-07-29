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




/** Main structure for a tbd.
 *
 *  Stores meta-data about the memory region used for tbd.
 *
 *  The key list grows downward into memory (higher memory addresses are used for added key elements).
 *  The heap grows upward into memory (lower memory addresses are used for added heap elements).
 *
 */
struct tbd_struct
{
  size_t size;          /**< Total size in bytes of the allocated datastore in bytes. */
  size_t block_size;    /**< Block size for the datastore, this is the minimum size that is allocated. */

  struct tbd_keyvalue_struct* keyvalue_free_list;    /**< Pointer to the list of elements that can are free (can be garbage collected). */  
  
  struct tbd_keyvalue_struct* keyvalue_list_start;   /**< Pointer to start of key list. */ 
  size_t keyvalue_list_size;                         /**< Size in bytes of key list. */
  
  void* heap_start;     /**< Pointer to start of heap. */
  size_t heap_size;     /**< Size in bytes of allocated heap. */
};




/** Key reference structure.
 */
typedef struct tbd_key_struct
{
  char* str;       /**< Pointer to key string data. */
  size_t size;     /**< Size in bytes of key string data. */
  
} tbd_key_t;




/** Value reference structure.
 */
typedef struct tbd_value_struct
{
  void* data;     /**< Pointer to generic stored data. */
  size_t size;    /**< Size in bytes of generic stored data. */
  
} tbd_value_t;




/** Key-Value pair structure.
 */
typedef struct tbd_keyvalue_struct
{
  tbd_key_t key;        /**< The key reference */
  tbd_value_t value;    /**< The value reference */
  
  bool free;                                /**< True if the data in the keyvalue is free (i.e. ready to be reclaimed). */
  struct tbd_keyvalue_struct* next_free;    /**< Pointer to next element after this one that can is free. */
  
  
} tbd_keyvalue_t;








static int tbd_value_read(tbd_t* tbd, const char* key)
{
  assert(tbd);
  
  return TBD_ERROR;   
}




static int tbd_value_update(tbd_t* tbd, const char* key)
{
  assert(tbd);
  
  return TBD_ERROR;   
}




static int tbd_value_delete(tbd_t* tbd, const char* key)
{
  assert(tbd);
  
  return TBD_ERROR;   
}




static struct tbd_keyvalue_struct* tbd_keyvalue_create(tbd_t* tbd, size_t key_size, size_t value_size)
{
  assert(tbd);
  
  const size_t chunk_size = key_size + value_size;  
  
  // check there is enough room to add elements of that size
  if ((void*) (tbd->keyvalue_list_start + tbd->keyvalue_list_size + sizeof(struct tbd_keyvalue_struct)) < (void*) (tbd->heap_start - chunk_size))
  {
    return 0;
  }
  
  // allocate a keyvalue list element
  struct tbd_keyvalue_struct* keyvalue_ptr = (struct tbd_keyvalue_struct*) (tbd->keyvalue_list_start + tbd->keyvalue_list_size);
  
  tbd->keyvalue_list_size += sizeof(struct tbd_keyvalue_struct);
  
  // allocate from heap
  tbd->heap_start -= chunk_size;
  tbd->heap_size  += chunk_size;
  
  keyvalue_ptr->value.data = tbd->heap_start;
  keyvalue_ptr->value.size = value_size;
  
  keyvalue_ptr->key.str = tbd->heap_start + value_size;
  keyvalue_ptr->key.size = key_size;
  
  // initialize free list node
  keyvalue_ptr->free = false;
  keyvalue_ptr->next_free = NULL;

  
  return keyvalue_ptr;
}




static struct tbd_keyvalue_struct* tbd_keyvalue_find(const tbd_t* tbd, const char* key)
{
  assert(tbd);
  
  // a simple linear search for a given key
  // TODO convert to standard C search
  struct tbd_keyvalue_struct* keyvalue_ptr = tbd->keyvalue_list_start;
  
  const tbd_keyvalue_t* end = (tbd->keyvalue_list_start + tbd->keyvalue_list_size);
  
  while (keyvalue_ptr != end)
  {
    if (strcmp(key, keyvalue_ptr->key.str) == 0)
    {
      return keyvalue_ptr;
    }
  }
  
  return 0;
}








int tbd_create(tbd_t* tbd, const char* key, const void* value, size_t value_size)
{
  assert(tbd);
  assert(key);
  assert(value);
  assert(value_size);
  
  
  // check that an element with given key does not already exist
  if (tbd_keyvalue_find(tbd, key))
  {
    return 0;
  }
  
  const size_t key_size = strlen(key) + 1;
  
  struct tbd_keyvalue_struct* keyvalue = tbd_keyvalue_create(tbd, key_size, value_size);
  
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
  assert(tbd);
  assert(key);
  
  // find the element
  struct tbd_keyvalue_struct* ptr = tbd_keyvalue_find(tbd, key);
  if (!ptr)
  {
    return TBD_NO_ERROR; // no error if it did not exist
  }  
  
  ptr->free = true;
  
  return TBD_NO_ERROR;
}




void tbd_clear(tbd_t* tbd)
{  
  tbd->keyvalue_list_size = 0;
  tbd->heap_size = 0;

  tbd->keyvalue_free_list = NULL;
}




/** Initialize the tbd inside the memory bounds in init structure.
 */
tbd_t* tbd_init(const tbd_init_t* init)
{
  assert(init);
  
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
  
  struct tbd_struct* tbd = init->start;
  tbd->size = init->size;
  
  // locate keyvalue list immediately after tbd struct
  tbd->keyvalue_list_start = (void*) (tbd + sizeof(struct tbd_struct));  
  
  // locate start of heap at end of allocated region
  tbd->heap_start = init->start + init->size;
  
  tbd_clear(tbd);
  
  return tbd;
}



static size_t tbd_key_to_json(char* json, size_t json_size, const tbd_key_t* key, TBD_KEY_TO_JSON_FORMAT_ENUM format)
{
  assert(json);
  assert(json_size);
  assert(key);
  
  size_t printed = 0;
  size_t total_printed = 0;
  
  printed += snprintf(json, json_size, "%s", key->str); 
  
  total_printed += printed;
  
  return total_printed;
}








static size_t tbd_value_to_json(char* json, size_t json_size, const tbd_value_t* value, TBD_VALUE_TO_JSON_FORMAT_ENUM format)
{
  assert(json);
  assert(json_size);
  assert(value);
  
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
  assert(json);
  assert(json_size);
  assert(tbd);
  assert(key);
  
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