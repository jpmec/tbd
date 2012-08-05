/** Tests for tbd.
 */




#include "tbd.h"


#include <assert.h>
#include <stdio.h>



#define START_TEST_TBD(_tbd_ptr_) \
  puts(__FUNCTION__); \
  assert(_tbd_ptr_); \
  tbd_empty(_tbd_ptr_);



static unsigned tbd_memory[256];
static char json_buffer[256];



/* Simple structures used by tests. */
struct Foo {
  int a;
  char b;
};




struct Bar {
  char c;
};




static int test_tbd_size(tbd_t* tbd)
{
  START_TEST_TBD(tbd);  
  
  size_t tbd_size_result0 = tbd_size(tbd);
  assert(0 < tbd_size_result0);
  
  struct Foo foo1 = {1, 'a'};  
  int tbd_create_result = tbd_create(tbd, "foo", &foo1, sizeof(struct Foo));
  assert(TBD_NO_ERROR == tbd_create_result);
  
  size_t tbd_size_result1 = tbd_size(tbd);
  assert(0 < tbd_size_result1);
  assert(tbd_size_result0 == tbd_size_result1);
  
  return 0;
}




static int test_tbd_size_used(tbd_t* tbd)
{
  START_TEST_TBD(tbd);  
  
  size_t tbd_size_used_result0 = tbd_size_used(tbd);
  assert(0 < tbd_size_used_result0);
  
  struct Foo foo1 = {1, 'a'};  
  int tbd_create_result = tbd_create(tbd, "foo", &foo1, sizeof(struct Foo));
  assert(TBD_NO_ERROR == tbd_create_result);
  
  size_t tbd_size_used_result1 = tbd_size(tbd);
  assert(0 < tbd_size_used_result1);
  assert(tbd_size_used_result0 < tbd_size_used_result1);
  
  return 0;
}




static int test_tbd_create(tbd_t* tbd)
{
  START_TEST_TBD(tbd);  

  /* Setup tbd_create */
  struct Foo foo1 = {1, 'a'};  
  
  /* Exercise tbd_create */
  int tbd_create_result = tbd_create(tbd, "foo", &foo1, sizeof(struct Foo));
  
  /* Verify tbd_create */
  assert(TBD_NO_ERROR == tbd_create_result);
  
  tbd_to_json(json_buffer, sizeof(json_buffer), tbd, TBD_KEY_TO_JSON_FORMAT_RAW, TBD_VALUE_TO_JSON_FORMAT_HEX);  
  puts(json_buffer);
  
  return 0;
}




static int test_tbd_read(tbd_t* tbd)
{
  START_TEST_TBD(tbd);
  
  /* Setup tbd_create */
  struct Foo foo1 = {1, 'a'};  
  int tbd_create_result = tbd_create(tbd, "foo", &foo1, sizeof(struct Foo));
  assert(TBD_NO_ERROR == tbd_create_result);  
  
  /* Setup tbd_read */
  struct Foo foo2 = {0};
  
  /* Exercise tbd_read */
  int tbd_read_result = tbd_read(tbd, "foo", &foo2, sizeof(struct Foo));
  
  /* Verify tbd_read */
  assert(TBD_NO_ERROR == tbd_read_result);
  assert(foo1.a == foo2.a);
  assert(foo1.b == foo2.b);
  
  return 0;
}




static int test_tbd_update(tbd_t* tbd)
{
  START_TEST_TBD(tbd);  

  struct Foo foo1 = {1, 'a'};  
  int tbd_create_result = tbd_create(tbd, "foo", &foo1, sizeof(struct Foo));
  assert(TBD_NO_ERROR == tbd_create_result);    
  
  // Setup tbd_update
  foo1.a = 2;
  
  // Exercise tbd_update
  int tbd_update_result = tbd_update(tbd, "foo", &foo1, sizeof(struct Foo));
  
  // Verify tbd_update
  assert(TBD_NO_ERROR == tbd_update_result);

  struct Foo foo2 = {0};   
  int tbd_read_result = tbd_read(tbd, "foo", &foo2, sizeof(struct Foo));
  
  // Verify tbd_read
  assert(TBD_NO_ERROR == tbd_read_result);
  assert(foo1.a == foo2.a);
  assert(foo1.b == foo2.b);   
  
  return 0;
}




static int test_tbd_delete(tbd_t* tbd)
{
  START_TEST_TBD(tbd);  
  
  struct Foo foo1 = {1, 'a'};  
  int tbd_create_result = tbd_create(tbd, "foo", &foo1, sizeof(struct Foo));
  assert(TBD_NO_ERROR == tbd_create_result);   
  
  
  /* Exercise tbd_delete */
  int tbd_delete_result = tbd_delete(tbd, "foo");
  
  /* verify tbd_delete */
  assert(TBD_NO_ERROR == tbd_delete_result);
  
  // attempt to read deleted object
  int tbd_read_result = tbd_read(tbd, "foo", &foo1, sizeof(struct Foo));
  assert(TBD_ERROR == tbd_read_result);  
  
  return 0;
}




static int test_tbd_garbage_size(tbd_t* tbd)
{
  START_TEST_TBD(tbd);  

  /* test with clear database */
  size_t tbd_garbage_size_result = tbd_garbage_size(tbd);
  assert(0 == tbd_garbage_size_result);  
  
  
  /* setup database with 1 element */
  struct Foo foo1 = {1, 'a'};
  
  int tbd_create_result = tbd_create(tbd, "foo1", &foo1, sizeof(struct Foo));
  assert(TBD_NO_ERROR ==  tbd_create_result);
  
  int tbd_delete_result = tbd_delete(tbd, "foo1");
  assert(TBD_NO_ERROR ==  tbd_delete_result);
  
  tbd_garbage_size_result = tbd_garbage_size(tbd);
  assert(0 < tbd_garbage_size_result);
  
  tbd_create_result = tbd_create(tbd, "foo2", &foo1, sizeof(struct Foo));
  assert(TBD_NO_ERROR ==  tbd_create_result);
  
  tbd_delete_result = tbd_delete(tbd, "foo2");
  assert(TBD_NO_ERROR ==  tbd_delete_result);
  
  tbd_garbage_size_result = tbd_garbage_size(tbd);
  assert(0 < tbd_garbage_size_result);
  
  return 0;
}




static int test_tbd_garbage_pop(tbd_t* tbd)
{
  START_TEST_TBD(tbd);  

  /* setup database with 1 element */
  struct Foo foo1 = {1, 'a'};
  
  int tbd_create_result = tbd_create(tbd, "foo1", &foo1, sizeof(struct Foo));
  assert(TBD_NO_ERROR ==  tbd_create_result);
  
  size_t tbd_garbage_size_result = tbd_garbage_size(tbd);
  assert(0 == tbd_garbage_size_result); 
  
  int tbd_delete_result = tbd_delete(tbd, "foo1");
  assert(TBD_NO_ERROR ==  tbd_delete_result);
  
  tbd_garbage_size_result = tbd_garbage_size(tbd);
  assert(0 < tbd_garbage_size_result);  
  
  size_t tbd_garbage_pop_result = tbd_garbage_pop(tbd, 0);
  assert(0 == tbd_garbage_pop_result);
  
  tbd_garbage_size_result = tbd_garbage_size(tbd);
  assert(0 < tbd_garbage_size_result);   
  
  tbd_garbage_pop_result = tbd_garbage_pop(tbd, tbd_garbage_size_result - 1);
  assert(0 == tbd_garbage_pop_result);
  
  tbd_garbage_size_result = tbd_garbage_size(tbd);
  assert(0 < tbd_garbage_size_result);   

  tbd_garbage_pop_result = tbd_garbage_pop(tbd, tbd_garbage_size_result);
  assert(0 < tbd_garbage_pop_result);

  tbd_garbage_size_result = tbd_garbage_size(tbd);
  assert(0 == tbd_garbage_size_result);     
  
  return 0;
}




static int test_tbd_garbage_fold(tbd_t* tbd)
{
  START_TEST_TBD(tbd);  
  
  /* setup database with 1 element */
  struct Foo foo1 = {1, 'a'};
  struct Foo foo2 = {2, 'b'};
  struct Foo foo3 = {3, 'c'};
  
  int tbd_create_result = tbd_create(tbd, "foo1", &foo1, sizeof(struct Foo));
  assert(TBD_NO_ERROR ==  tbd_create_result);

  tbd_create_result = tbd_create(tbd, "foo2", &foo2, sizeof(struct Foo));
  assert(TBD_NO_ERROR ==  tbd_create_result);  
  
  tbd_create_result = tbd_create(tbd, "foo3", &foo3, sizeof(struct Foo));
  assert(TBD_NO_ERROR ==  tbd_create_result);  
  
  size_t tbd_garbage_size_result = tbd_garbage_size(tbd);
  assert(0 == tbd_garbage_size_result); 
  
  int tbd_delete_result = tbd_delete(tbd, "foo1");
  assert(TBD_NO_ERROR ==  tbd_delete_result);
  
  tbd_garbage_size_result = tbd_garbage_size(tbd);
  assert(0 < tbd_garbage_size_result);  
  
  size_t tbd_garbage_fold_result = tbd_garbage_fold(tbd, 0);
  assert(0 == tbd_garbage_fold_result);
  
  tbd_garbage_size_result = tbd_garbage_size(tbd);
  assert(0 < tbd_garbage_size_result);   
  
  tbd_garbage_fold_result = tbd_garbage_fold(tbd, tbd_garbage_size_result - 1);
  assert(0 == tbd_garbage_fold_result);
  
  tbd_garbage_size_result = tbd_garbage_size(tbd);
  assert(0 < tbd_garbage_size_result);   
  
  tbd_garbage_fold_result = tbd_garbage_fold(tbd, tbd_garbage_size_result);
  assert(0 < tbd_garbage_fold_result);
  
  tbd_garbage_size_result = tbd_garbage_size(tbd);
  assert(0 == tbd_garbage_size_result);     
  
  return 0;
}




static int test_tbd_garbage_pack(tbd_t* tbd)
{
  START_TEST_TBD(tbd);  
   
  return 0;
}




static int test_tbd_garbage_collect(tbd_t* tbd)
{
  START_TEST_TBD(tbd);  
  
  // exercise with empty tbd
  size_t tbd_garbage_collect_result = tbd_garbage_collect(tbd, 0);
  
  assert(0 == tbd_garbage_collect_result);
  
  
  // exercise with no garbage
  struct Foo foo1 = {1, 'a'};
  
  int tbd_create_result = tbd_create(tbd, "foo1", &foo1, sizeof(struct Foo));
  assert(TBD_NO_ERROR ==  tbd_create_result);
  
  tbd_garbage_collect_result = tbd_garbage_collect(tbd, 0);  
  assert(0 == tbd_garbage_collect_result);

  // exercise with garbage  
  int tbd_delete_result = tbd_delete(tbd, "foo1");
  assert(TBD_NO_ERROR ==  tbd_delete_result);
  
  tbd_garbage_collect_result = tbd_garbage_collect(tbd, 0);  
  assert(0 == tbd_garbage_collect_result);

  tbd_garbage_collect_result = tbd_garbage_collect(tbd, 0x1000);  
  assert(0 < tbd_garbage_collect_result);  
  
  return 0;
}




static int test_tbd_garbage_clean(tbd_t* tbd)
{
  START_TEST_TBD(tbd);  
  
  // exercise with empty tbd
  size_t tbd_garbage_clean_result = tbd_garbage_clean(tbd);
  assert(0 == tbd_garbage_clean_result);
  
  
  // exercise with no garbage
  struct Foo foo1 = {1, 'a'};
  
  int tbd_create_result = tbd_create(tbd, "foo1", &foo1, sizeof(struct Foo));
  assert(TBD_NO_ERROR ==  tbd_create_result);
  
  tbd_garbage_clean_result = tbd_garbage_clean(tbd);  
  assert(0 == tbd_garbage_clean_result);
  
  // exercise with garbage  
  int tbd_delete_result = tbd_delete(tbd, "foo1");
  assert(TBD_NO_ERROR ==  tbd_delete_result);
  
  tbd_garbage_clean_result = tbd_garbage_clean(tbd);  
  assert(0 < tbd_garbage_clean_result);
  
  return 0;
}





static int test_tbd_json(tbd_t* tbd)
{
  START_TEST_TBD(tbd);  
  
  /* Add an element */
  struct Foo foo1 = {1, 'a'};
  struct Foo foo2 = {2, 'b'};
  
  int tbd_create_result = tbd_create(tbd, "foo1", &foo1, sizeof(struct Foo));
  
  assert(TBD_NO_ERROR ==  tbd_create_result);
  
  tbd_create_result = tbd_create(tbd, "foo2", &foo2, sizeof(struct Foo));
  
  assert(TBD_NO_ERROR ==  tbd_create_result);
  
  
  tbd_keyvalue_to_json(json_buffer, sizeof(json_buffer), tbd, "foo1", TBD_KEY_TO_JSON_FORMAT_RAW, TBD_VALUE_TO_JSON_FORMAT_HEX);
                       
  puts(json_buffer);

  tbd_keyvalue_to_json(json_buffer, sizeof(json_buffer), tbd, "foo2", TBD_KEY_TO_JSON_FORMAT_RAW, TBD_VALUE_TO_JSON_FORMAT_HEX);
  
  puts(json_buffer);  
  
  tbd_to_json(json_buffer, sizeof(json_buffer), tbd, TBD_KEY_TO_JSON_FORMAT_RAW, TBD_VALUE_TO_JSON_FORMAT_HEX);
  
  puts(json_buffer);
  
  return 0;
}




int test_tbd(void);

int test_tbd(void)
{
  tbd_init_t init = {
    .start = tbd_memory,
    .size = sizeof(tbd_memory),
    .hunk_size = sizeof(int)
  };
  
  
  /* Exercise tbd_init */
  tbd_t* tbd = tbd_init(&init);
  
  /* Verify tbd_init */
  assert(tbd);

  /* Test basic operations */
  assert(0 == test_tbd_size(tbd));
  assert(0 == test_tbd_size_used(tbd));  
  
  /* Test the CRUD */
  assert(0 == test_tbd_create(tbd));
  assert(0 == test_tbd_read(tbd));
  assert(0 == test_tbd_update(tbd));
  assert(0 == test_tbd_delete(tbd));
  
  /* Test garbage collection */
  assert(0 == test_tbd_garbage_size(tbd));
  assert(0 == test_tbd_garbage_pop(tbd));  
  assert(0 == test_tbd_garbage_fold(tbd));
  assert(0 == test_tbd_garbage_pack(tbd));    
  assert(0 == test_tbd_garbage_collect(tbd));
  assert(0 == test_tbd_garbage_clean(tbd));
  
  /* Test JSON support */
  assert(0 == test_tbd_json(tbd));
  
  
  return 0; // return 0 for success
}