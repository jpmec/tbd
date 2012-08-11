/** Tests for tbd.
 */



#include "test_tbd.h"
#include "tbd.h"


#include <assert.h>
#include <stdio.h>



#define START_TEST_TBD(_tbd_ptr_) \
  printf("%s:{\n",__FUNCTION__); \
  assert(_tbd_ptr_); \
  tbd_empty(_tbd_ptr_); \
  tbd_print_stats(_tbd_ptr_);




#define FINISH_TEST_TBD(_tbd_ptr_) \
  tbd_print_stats(_tbd_ptr_); \
  puts("}\n");




static unsigned char tbd_memory[1024];
static char json_buffer[256];



/* Simple structures used by tests. */
struct Foo {
  int a;
  char b;
};




struct Bar {
  char c;
};




/* Simple structure for a single char */
struct Char {
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
  
  FINISH_TEST_TBD(tbd);  
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
  
  FINISH_TEST_TBD(tbd);   
  return 0;
}




static int test_tbd_create(tbd_t* tbd)
{
  START_TEST_TBD(tbd);  
  
  // Setup tbd_create
  struct Foo foo1 = {1, 'a'};  
  
  // Exercise tbd_create
  int tbd_create_result = tbd_create(tbd, "foo", &foo1, sizeof(struct Foo));
  assert(TBD_NO_ERROR == tbd_create_result);
  
  tbd_print_stats(tbd);
  
  size_t json_size = tbd_to_json(json_buffer, sizeof(json_buffer), tbd, TBD_KEY_TO_JSON_FORMAT_RAW, TBD_VALUE_TO_JSON_FORMAT_HEX);  
  
  if (json_size)
    puts(json_buffer);
  
  // try to create something with the same key
  tbd_create_result = tbd_create(tbd, "foo", &foo1, sizeof(struct Foo));
  assert(TBD_ERROR_KEY_EXISTS == tbd_create_result);
  
  tbd_print_stats(tbd);
  
  
  FINISH_TEST_TBD(tbd);   
  return 0;
}




static int test_tbd_create__fill_tbd(tbd_t* tbd)
{
  START_TEST_TBD(tbd);  
  
  struct Char c = {0};
  char key[TBD_MAX_KEY_LENGTH] = {0};
  int key_int = 0;
  
  int tbd_create_result = TBD_NO_ERROR;
  
  while (TBD_NO_ERROR == tbd_create_result)
  {
    // TODO use TBD_MAX_KEY_LENGTH here
    ++key_int;
    sprintf(key, "%07d", key_int);
    
    tbd_create_result = tbd_create(tbd, key, &c, sizeof(struct Char));
    
    size_t key_count = tbd_count(tbd);
    
    if (TBD_NO_ERROR == tbd_create_result)
    {
      assert(key_count == key_int);
    }
  }
  
  tbd_print_stats(tbd);
  
  FINISH_TEST_TBD(tbd);   
  return 0;  
}




static int test_tbd_read(tbd_t* tbd)
{
  START_TEST_TBD(tbd);  
  
  /* Setup tbd_create */
  struct Foo foo1 = {1, 'a'};  
  int tbd_create_result = tbd_create(tbd, "foo1", &foo1, sizeof(struct Foo));
  assert(TBD_NO_ERROR == tbd_create_result);  

  tbd_print_stats(tbd);
    
  /* Setup tbd_read */
  struct Foo foo_result = {0};
  
  /* Exercise tbd_read */
  int tbd_read_result = tbd_read(tbd, "foo1", &foo_result, sizeof(struct Foo));
  
  /* Verify tbd_read */
  assert(TBD_NO_ERROR == tbd_read_result);
  assert(foo1.a == foo_result.a);
  assert(foo1.b == foo_result.b);
  
  // Add more key:value pairs
  struct Foo foo = {2, 'b'};  
  tbd_create_result = tbd_create(tbd, "foo2", &foo, sizeof(struct Foo));
  assert(TBD_NO_ERROR == tbd_create_result);    
  
  foo = (struct Foo) {3, 'c'};
  tbd_create_result = tbd_create(tbd, "foo3", &foo, sizeof(struct Foo));
  assert(TBD_NO_ERROR == tbd_create_result);    

  foo = (struct Foo) {4, 'd'};
  tbd_create_result = tbd_create(tbd, "foo4", &foo, sizeof(struct Foo));
  assert(TBD_NO_ERROR == tbd_create_result);    

  foo = (struct Foo) {5, 'e'};
  tbd_create_result = tbd_create(tbd, "foo5", &foo, sizeof(struct Foo));
  assert(TBD_NO_ERROR == tbd_create_result);    

  foo = (struct Foo) {6, 'f'};
  tbd_create_result = tbd_create(tbd, "foo6", &foo, sizeof(struct Foo));
  assert(TBD_NO_ERROR == tbd_create_result);    
    
  
  FINISH_TEST_TBD(tbd);   
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
  
  
  // Exercise with different object
  struct Bar bar1 = {0};
  
  tbd_update_result = tbd_update(tbd, "foo", &bar1, sizeof(struct Bar));
  assert(TBD_ERROR_BAD_SIZE == tbd_update_result);
  
  FINISH_TEST_TBD(tbd);   
  return 0;
}




static int test_tbd_delete(tbd_t* tbd)
{
  START_TEST_TBD(tbd);
  
  struct Foo foo1 = {1, 'a'};  
  int tbd_create_result = tbd_create(tbd, "foo", &foo1, sizeof(struct Foo));
  assert(TBD_NO_ERROR == tbd_create_result);   
  
  tbd_print_stats(tbd);  
  
  /* Exercise tbd_delete */
  int tbd_delete_result = tbd_delete(tbd, "foo");
  
  /* verify tbd_delete */
  assert(TBD_NO_ERROR == tbd_delete_result);
  
  tbd_print_stats(tbd);  

  // attempt to read deleted object
  int tbd_read_result = tbd_read(tbd, "foo", &foo1, sizeof(struct Foo));
  assert(TBD_ERROR_KEY_NOT_FOUND == tbd_read_result);  
  
  FINISH_TEST_TBD(tbd);   
  return 0;
}




static int test_tbd_read_size(tbd_t* tbd)
{
  START_TEST_TBD(tbd);
  
  // Exercise with empty tbd
  size_t tbd_read_size_result = tbd_read_size(tbd, "foo");
  assert(0 == tbd_read_size_result);
  
  
  // Exercise with element
  struct Foo foo1 = {1, 'a'};  
  int tbd_create_result = tbd_create(tbd, "foo", &foo1, sizeof(struct Foo));
  assert(TBD_NO_ERROR == tbd_create_result);   
  
  tbd_read_size_result = tbd_read_size(tbd, "foo");
  assert(sizeof(struct Foo) == tbd_read_size_result);
  
  
  FINISH_TEST_TBD(tbd);   
  return 0;  
}




static int test_tbd_garbage_size(tbd_t* tbd)
{
  START_TEST_TBD(tbd);  

  // test with clear database
  size_t tbd_garbage_size_result = tbd_garbage_size(tbd);
  assert(0 == tbd_garbage_size_result);  
  
  
  // setup database with 1 element
  struct Foo foo1 = {1, 'a'};
  
  int tbd_create_result = tbd_create(tbd, "foo1", &foo1, sizeof(struct Foo));
  assert(TBD_NO_ERROR ==  tbd_create_result);
  
  tbd_print_stats(tbd);
  
  int tbd_delete_result = tbd_delete(tbd, "foo1");
  assert(TBD_NO_ERROR ==  tbd_delete_result);
  
  tbd_garbage_size_result = tbd_garbage_size(tbd);
  assert(0 < tbd_garbage_size_result);

  tbd_print_stats(tbd);  
  
  // add another element
  struct Foo foo2 = {2, 'b'};  
  
  tbd_create_result = tbd_create(tbd, "foo2", &foo2, sizeof(struct Foo));
  assert(TBD_NO_ERROR ==  tbd_create_result);
  
  tbd_print_stats(tbd);  
  
  tbd_delete_result = tbd_delete(tbd, "foo2");
  assert(TBD_NO_ERROR ==  tbd_delete_result);
  
  tbd_garbage_size_result = tbd_garbage_size(tbd);
  assert(0 < tbd_garbage_size_result);
  
  FINISH_TEST_TBD(tbd);   
  return 0;
}




static int test_tbd_garbage_pop(tbd_t* tbd)
{
  START_TEST_TBD(tbd);
  
  /* setup database with 1 element */
  struct Foo foo1 = {1, 'a'};
  
  int tbd_create_result = tbd_create(tbd, "foo1", &foo1, sizeof(struct Foo));
  assert(TBD_NO_ERROR ==  tbd_create_result);
  tbd_print_stats(tbd);
  
  size_t tbd_garbage_size_result = tbd_garbage_size(tbd);
  assert(0 == tbd_garbage_size_result); 
  
  int tbd_delete_result = tbd_delete(tbd, "foo1");
  assert(TBD_NO_ERROR ==  tbd_delete_result);
  tbd_print_stats(tbd);
  
  tbd_garbage_size_result = tbd_garbage_size(tbd);
  assert(0 < tbd_garbage_size_result);  
  
  size_t tbd_garbage_pop_result = tbd_garbage_pop(tbd, 0);
  assert(0 == tbd_garbage_pop_result);
  tbd_print_stats(tbd);
  
  tbd_garbage_size_result = tbd_garbage_size(tbd);
  assert(0 < tbd_garbage_size_result);   
  
  tbd_garbage_pop_result = tbd_garbage_pop(tbd, tbd_garbage_size_result - 1);
  assert(0 == tbd_garbage_pop_result);
  tbd_print_stats(tbd);
  
  tbd_garbage_size_result = tbd_garbage_size(tbd);
  assert(0 < tbd_garbage_size_result);   

  tbd_garbage_pop_result = tbd_garbage_pop(tbd, tbd_garbage_size_result);
  assert(0 < tbd_garbage_pop_result);
  tbd_print_stats(tbd);
  
  tbd_garbage_size_result = tbd_garbage_size(tbd);
  assert(0 == tbd_garbage_size_result);     
  
  FINISH_TEST_TBD(tbd);  
  return 0;
}




static int test_tbd_garbage_fold(tbd_t* tbd)
{
  START_TEST_TBD(tbd);  
  
  // setup database elements
  struct Foo foo1 = {1, 'a'};
  struct Foo foo2 = {2, 'b'};
  struct Foo foo3 = {3, 'c'};
  struct Foo foo4 = {4, 'd'};

  struct Bar bar1 = {'w'};  
  struct Bar bar2 = {'x'};
  struct Bar bar3 = {'y'};
  struct Bar bar4 = {'z'};  
  
  int tbd_create_result = tbd_create(tbd, "foo1", &foo1, sizeof(struct Foo));
  assert(TBD_NO_ERROR ==  tbd_create_result);

  tbd_create_result = tbd_create(tbd, "bar1", &bar1, sizeof(struct Bar));
  assert(TBD_NO_ERROR ==  tbd_create_result);  
  
  tbd_create_result = tbd_create(tbd, "foo2", &foo2, sizeof(struct Foo));
  assert(TBD_NO_ERROR ==  tbd_create_result);  

  tbd_create_result = tbd_create(tbd, "bar2", &bar2, sizeof(struct Bar));
  assert(TBD_NO_ERROR ==  tbd_create_result);    
  
  tbd_create_result = tbd_create(tbd, "foo3", &foo3, sizeof(struct Foo));
  assert(TBD_NO_ERROR ==  tbd_create_result);
  
  tbd_create_result = tbd_create(tbd, "bar3", &bar3, sizeof(struct Bar));
  assert(TBD_NO_ERROR ==  tbd_create_result);    

  tbd_create_result = tbd_create(tbd, "foo4", &foo4, sizeof(struct Foo));
  assert(TBD_NO_ERROR ==  tbd_create_result);
  
  tbd_create_result = tbd_create(tbd, "bar4", &bar4, sizeof(struct Bar));
  assert(TBD_NO_ERROR ==  tbd_create_result);    
  
  size_t tbd_garbage_size_result = tbd_garbage_size(tbd);
  assert(0 == tbd_garbage_size_result);
  
  tbd_print_stats(tbd);
  
  int tbd_delete_result = tbd_delete(tbd, "foo2");
  assert(TBD_NO_ERROR ==  tbd_delete_result);
  
  tbd_print_stats(tbd);  
  
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
    
  FINISH_TEST_TBD(tbd);   
  return 0;
}




static int test_tbd_garbage_pack(tbd_t* tbd)
{
  START_TEST_TBD(tbd);  

  // exercise with empty tbd
  size_t tbd_garbage_pack_result = tbd_garbage_pack(tbd, tbd_size(tbd));
  assert(0 == tbd_garbage_pack_result);  
  
  
  // setup database element
  struct Foo foo1 = {1, 'a'};
  struct Foo foo2 = {2, 'b'};
  struct Foo foo3 = {3, 'c'};
  struct Foo foo4 = {4, 'd'};
  
  struct Bar bar1 = {'w'};  
  struct Bar bar2 = {'x'};
  struct Bar bar3 = {'y'};
  struct Bar bar4 = {'z'};  
  
  int tbd_create_result = tbd_create(tbd, "foo1", &foo1, sizeof(struct Foo));
  assert(TBD_NO_ERROR ==  tbd_create_result);
  
  tbd_create_result = tbd_create(tbd, "bar1", &bar1, sizeof(struct Bar));
  assert(TBD_NO_ERROR ==  tbd_create_result);  
  
  tbd_create_result = tbd_create(tbd, "foo2", &foo2, sizeof(struct Foo));
  assert(TBD_NO_ERROR ==  tbd_create_result);  
  
  tbd_create_result = tbd_create(tbd, "bar2", &bar2, sizeof(struct Bar));
  assert(TBD_NO_ERROR ==  tbd_create_result);    
  
  tbd_create_result = tbd_create(tbd, "foo3", &foo3, sizeof(struct Foo));
  assert(TBD_NO_ERROR ==  tbd_create_result);
  
  tbd_create_result = tbd_create(tbd, "bar3", &bar3, sizeof(struct Bar));
  assert(TBD_NO_ERROR ==  tbd_create_result);    
  
  tbd_create_result = tbd_create(tbd, "foo4", &foo4, sizeof(struct Foo));
  assert(TBD_NO_ERROR ==  tbd_create_result);
  
  tbd_create_result = tbd_create(tbd, "bar4", &bar4, sizeof(struct Bar));
  assert(TBD_NO_ERROR ==  tbd_create_result);      
  
  int tbd_delete_result = tbd_delete(tbd, "foo2");
  assert(TBD_NO_ERROR ==  tbd_delete_result);
  
  size_t tbd_garbage_size_result = tbd_garbage_size(tbd);
  assert(0 < tbd_garbage_size_result);     
  
  tbd_print_stats(tbd);
  
  tbd_garbage_pack_result = tbd_garbage_pack(tbd, tbd_garbage_size_result);
  assert(0 < tbd_garbage_pack_result);
  
  FINISH_TEST_TBD(tbd);   
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
  
  FINISH_TEST_TBD(tbd);   
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
  
  FINISH_TEST_TBD(tbd);   
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
  
  
  size_t json_size = tbd_keyvalue_to_json(json_buffer, sizeof(json_buffer), tbd, "foo1", TBD_KEY_TO_JSON_FORMAT_RAW, TBD_VALUE_TO_JSON_FORMAT_HEX);
                     
  if (json_size)
    puts(json_buffer);

  json_size = tbd_keyvalue_to_json(json_buffer, sizeof(json_buffer), tbd, "foo2", TBD_KEY_TO_JSON_FORMAT_RAW, TBD_VALUE_TO_JSON_FORMAT_HEX);
  
  if (json_size)
    puts(json_buffer);
  
  json_size = tbd_to_json(json_buffer, sizeof(json_buffer), tbd, TBD_KEY_TO_JSON_FORMAT_RAW, TBD_VALUE_TO_JSON_FORMAT_HEX);
  
  if (json_size)
    puts(json_buffer);
  
  FINISH_TEST_TBD(tbd);   
  return 0;
}




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
  tbd_print_stats(tbd);
  
  
  /* Test basic operations */
  assert(0 == test_tbd_size(tbd));
  assert(0 == test_tbd_size_used(tbd));  
  
  
  /* Test the basic CRUD */
  assert(0 == test_tbd_create(tbd));
  assert(0 == test_tbd_create__fill_tbd(tbd));
  
  assert(0 == test_tbd_read(tbd));
  assert(0 == test_tbd_update(tbd));
  assert(0 == test_tbd_delete(tbd));
  
  
  /* Test the advance CRUD */
  assert(0 == test_tbd_read_size(tbd));
  
  
  /* Test garbage collection */
//  assert(0 == test_tbd_garbage_size(tbd));
//  assert(0 == test_tbd_garbage_pop(tbd));  
//  assert(0 == test_tbd_garbage_fold(tbd));
//  assert(0 == test_tbd_garbage_pack(tbd));    
//  assert(0 == test_tbd_garbage_collect(tbd));
//  assert(0 == test_tbd_garbage_clean(tbd));
  
  
  /* Test JSON support */
  assert(0 == test_tbd_json(tbd));
  
  
  return 0; // return 0 for success
}