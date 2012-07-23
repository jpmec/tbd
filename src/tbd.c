

#include "tbd.h"



/** Main structure for a tbd.
 */
struct tbd_struct
{
  unsigned size;       /* Total size of the allocated datastore in bytes. */
  unsigned block_size; /* Block size for the datastore, this is the minimum size that is allocated. */
  
  void* store_start;
};





int tbd_create(tbd_t* tbd, const char* key, const void* value, unsigned size)
{
  return -1;   
}




int tbd_read(tbd_t* tbd, const char* key, void* value, unsigned size)
{
  return -1;
}




int tbd_update(tbd_t* tbd, const char* key, const void* value, unsigned size)
{
  return -1;
}




int tbd_delete(tbd_t* tbd, const char* key)
{
  return -1;
}




tbd_t* tbd_init(const tbd_init_t* init)
{
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
  
  return tbd;
}