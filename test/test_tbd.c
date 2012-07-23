/** Tests for tbd.
 */




#include "tbd.h"


#include <assert.h>




static unsigned tbd_memory[256];


struct Foo {
  int a;
  char b;
};




int test_tbd(void)
{
  tbd_init_t init = {
    .start = tbd_memory,
    .size = sizeof(tbd_memory)
  };
  
  
  /* Exercise tbd_init */
  tbd_t* tbd = tbd_init(&init);

  
  /* Verify tbd_init */
  assert(tbd);
  

  /* Setup tbd_create */
  struct Foo foo1 = {1, 2};  
    
  /* Exercise tbd_create */
  int tbd_create_result = tbd_create(tbd, "foo", &foo1, sizeof(struct Foo));
  
  /* Verify tbd_create */
  assert (tbd_create_result == 0);
  
  
  /* Setup tbd_read */
  struct Foo foo2 = {0};
  
  /* Exercise tbd_read */
  int tbd_read_result = tbd_read(tbd, "foo", &foo2, sizeof(struct Foo));
  
  /* Verify tbd_read */
  assert(foo1.a == foo2.a);
  assert(foo1.b == foo2.b);
  
  
  /* Setup tbd_update */
  foo1.a = 3;
  
  /* Exercise tbd_update */
  int tbd_update_result = tbd_update(tbd, "foo", &foo1, sizeof(struct Foo));
  
  /* Verify tbd_update */
  assert(tbd_update_result == 0);
  
  
  return 0; // return 0 for success
}