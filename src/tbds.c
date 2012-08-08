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




#include <stdio.h>
#include <string.h>
#include "tbd.h"




void tbds_start(struct tbds_start_params* params)
{
  char fin_buffer[256] = {0};
  char cmd_buffer[9] = {0};
  
  unsigned tbd_buffer[1024];
  
  
  const tbd_init_t tbd_params = {
  
    .start = tbd_buffer,
    .size = sizeof(tbd_buffer),
    .hunk_size = sizeof(unsigned),
  };
  
  
  
  tbd_t* tbd = tbd_init(&tbd_params);
  
  
  do
  {
    
    fgets(fin_buffer, sizeof(fin_buffer) - 1, stdin);
    
    
    memcpy(cmd_buffer, fin_buffer, sizeof(cmd_buffer) - 1);
    
    
    if (strstr(cmd_buffer, "select "))
    {
      fprintf(stdout, "%s", fin_buffer);
      
      int result = -1;
      
      if (result != 0)
      {
        fprintf(stderr, "error: %d", result);         
      } 
    }
    
    else if (strstr(cmd_buffer, "update "))
    {
      fprintf(stdout, "%s", fin_buffer);
      
      int result = -1;
      
      if (result != 0)
      {
        fprintf(stderr, "error: %d", result);         
      }       
    }    
    
    else if (strstr(cmd_buffer, "insert "))
    {
      fprintf(stdout, "%s", fin_buffer);
      
      int result = tbd_create(tbd, "key", "value", sizeof("value"));
      
      if (result != 0)
      {
        fprintf(stderr, "error: %d", result);         
      }
    }
    
    else if (strstr(cmd_buffer, "delete "))
    {
      fprintf(stdout, "%s", fin_buffer);
      
      int result = -1;
      
      if (result != 0)
      {
        fprintf(stderr, "error: %d", result);         
      }      
    }
    else
    {
      fprintf(stderr, "invalid: %s", fin_buffer); 
    }
  } while(1);
}


