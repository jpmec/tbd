// Include the Ruby headers and goodies
#include "ruby.h"
#include "../../../../tbd.h"
#include "../../../../tbd.c"


static unsigned char array[1024];

static tbd_t* tbd;


// Defining a space for information and references about the module to be stored internally
VALUE TbdModule = Qnil;


// Prototype for our methods. Methods are prefixed by 'method_' here
VALUE method_create(VALUE self, VALUE key, VALUE value);
VALUE method_read(VALUE self, VALUE key);
VALUE method_update(VALUE self, VALUE key, VALUE value);
VALUE method_delete(VALUE self, VALUE key);

VALUE method_max_key_length(VALUE self);



// The initialization method for this module
void Init_tbdrb() 
{
	TbdModule = rb_define_module("Tbdrb");

	rb_define_method(TbdModule, "create", method_create, 2);	
	rb_define_method(TbdModule, "read", method_read, 1);	
	rb_define_method(TbdModule, "update", method_update, 2);	
	rb_define_method(TbdModule, "delete", method_delete, 1);
  
  rb_define_method(TbdModule, "max_key_length", method_max_key_length, 0);
  
  tbd_init_t init_params =
  {
    .start = array,
    .size = sizeof(array),
    .hunk_size = 1
  };
  
  tbd = tbd_init(&init_params);
}




VALUE method_max_key_length(VALUE self)
{
  return INT2NUM(tbd_max_key_length(tbd));
}




VALUE method_create(VALUE self, VALUE key, VALUE value) 
{  
  Check_Type(key, T_STRING);
  
  int result = tbd_create(tbd, RSTRING(key)->ptr, RSTRING(value)->ptr, RSTRING(value)->len + 1);
  
  return INT2NUM(result);
}




VALUE method_read(VALUE self, VALUE key) 
{
  Check_Type(key, T_STRING);
  
	const size_t value_size = tbd_read_size(tbd, RSTRING(key)->ptr);
	
  char* value = ALLOC_N(char, value_size);
  
  int result = tbd_read(tbd, RSTRING(key)->ptr, value, value_size);
  
  if (TBD_NO_ERROR == result)
    return rb_str_new2(value);
  else
    return Qnil;
}




VALUE method_update(VALUE self, VALUE key, VALUE value) 
{
  Check_Type(key, T_STRING);
  
  int result = tbd_update(tbd, RSTRING(key)->ptr, RSTRING(value)->ptr, RSTRING(value)->len + 1);
  
	return INT2NUM(result);
}




VALUE method_delete(VALUE self, VALUE key) 
{
  Check_Type(key, T_STRING);
  
  int result = tbd_delete(tbd, RSTRING(key)->ptr);
  
  return INT2NUM(result);
}
