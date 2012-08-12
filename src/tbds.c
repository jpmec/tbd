/** tbds.c
 *  
 * Tiny Basic Datastore Server
 * 
 * A simple server implementation for Tiny Basic Datastore Server.
 *
 * Author: Joshua Petitt
 * Available at: https://github.com/jpmec/tbd
 *
 */




#include "tbds.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "tbd.h"




static size_t tbds_read_key(char* key_buffer, size_t key_buffer_size, FILE* file)
{
  assert(key_buffer);
  assert(file);

  --key_buffer_size;  // decrement for NULL terminator
  
  int c = EOF;
  size_t count = 0;

  do
  {
    c = fgetc(file);
    ++count;
    
    if (isalnum(c))
    {
      *key_buffer = c & 0xFF;
      ++key_buffer;
      --key_buffer_size;
    }
    else
    {
      break;
    }
    
  } while ((c != EOF) && (key_buffer_size));
  
  key_buffer[count] = '\0';
  
  return count;
}




static size_t tbds_read_value(char* value_buffer, size_t value_buffer_size, FILE* file)
{
  assert(value_buffer);
  assert(file);
  
  --value_buffer_size;  // decrement for NULL terminator
  
  int c = EOF;
  size_t count = 0;
  
  do
  {
    c = fgetc(file);
    ++count;
    
    if (isalnum(c))
    {
      switch (c)
      {
        default:
        {
          *value_buffer = c & 0xFF;
          ++value_buffer;
          --value_buffer_size;
        }
      }
    }
    else
    {
      break;
    }
    
  } while ((c != EOF) && (value_buffer_size));
  
  value_buffer[count] = '\0';
  
  return count;
}








static void tbds_create(tbd_t* tbd)
{
  char key_buffer[TBD_MAX_KEY_LENGTH] = {0};
  char value_buffer[256] = {0};
  
  
  /* read the key */
  size_t key_size = tbds_read_key(key_buffer, sizeof(key_buffer), stdin);
  size_t value_size = tbds_read_value(value_buffer, sizeof(value_buffer), stdin);
  
  if (!key_size || !value_size)
  {
    return;
  }
  
  fprintf(stdout, "key:'%s'\n", key_buffer);      
  fprintf(stdout, "value:'%s'\n", value_buffer);
  
  int result = tbd_create(tbd, key_buffer, value_buffer, value_size);
  
  if (result != 0)
  {
    fprintf(stderr, "error: %d", result);         
  }  
  
}




static void tbds_read(tbd_t* tbd)
{
  char key_buffer[TBD_MAX_KEY_LENGTH] = {0};
  char value_buffer[256] = {0};
  
  size_t key_size = tbds_read_key(key_buffer, sizeof(key_buffer), stdin);
  
  if (!key_size)
  {
    return;
  }
  
  int result = tbd_read(tbd, key_buffer, value_buffer, sizeof(value_buffer));
  
  if (result != 0)
  {
    fprintf(stderr, "error: %d", result);         
  } 
  else
  {
    fprintf(stdout, "%s", value_buffer);
  }  
}




static void tbds_update(tbd_t* tbd)
{
  char fin_buffer[256] = {0}; 
  
  fprintf(stdout, "%s", fin_buffer);
  
  int result = -1;
  
  if (result != 0)
  {
    fprintf(stderr, "error: %d", result);         
  }     
}




static void tbds_delete(tbd_t* tbd)
{
  char key_buffer[TBD_MAX_KEY_LENGTH] = {0};
  
  size_t key_size = tbds_read_key(key_buffer, sizeof(key_buffer), stdin);
  
  if (!key_size)
  {
    return;
  }

  int result = tbd_delete(tbd, key_buffer);
  
  if (result != 0)
  {
    fprintf(stderr, "error: %d", result);         
  }   
}



void tbds_start(const struct tbds_start_params* params)
{

  char cmd_buffer[9] = {0};
  
  unsigned tbd_buffer[TBD_MAX_SIZE];
  
  
  const tbd_init_t tbd_params = {
  
    .start = tbd_buffer,
    .size = sizeof(tbd_buffer) & TBD_MAX_SIZE,
    .hunk_size = sizeof(unsigned),
  };
  
  
  
  tbd_t* tbd = tbd_init(&tbd_params);
  
  
  do
  {
    
    fgets(cmd_buffer, sizeof(cmd_buffer) - 1, stdin);
    
    
    if (strncmp(cmd_buffer, "select ", 7) == 0)
    {
      tbds_read(tbd);
      printf("\n");
    }
    
    else if (strncmp(cmd_buffer, "update ", 7) == 0)
    {
      tbds_update(tbd);
      printf("\n");

    }    
    
    else if (strncmp(cmd_buffer, "insert ", 7) == 0)
    {
      tbds_create(tbd);
      printf("\n");
    }
    
    else if (strncmp(cmd_buffer, "delete ", 7) == 0)
    {
      tbds_delete(tbd);
      printf("\n");      
    }
    else
    {
      fprintf(stderr, "invalid: %s", cmd_buffer); 
    }
    
  } while(1);
}


