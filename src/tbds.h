/** tbds.h
 *  
 * Tiny Basic Datastore Server
 * 
 * A simple server for Tiny Basic Datastore
 *
 * Author: Joshua Petitt
 * Available at: https://github.com/jpmec/tbd
 *
 */


#ifndef _TBDS_H_
#define _TBDS_H_




#include <stddef.h>




struct tbds_start_params
{
  size_t tbd_size;
};



/** Start the server.
 */
void tbds_start(const struct tbds_start_params*);




#endif//_TBDS_H_
