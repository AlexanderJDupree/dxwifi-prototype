/**
 *  decoder.h - API to decode FEC data
 * 
 */

#ifndef LIBDXWIFI_DECODER_H
#define LIBDXWIFI_DECODER_H

/************************
 *  Includes
 ***********************/

#include <stdlib.h>
#include <stdint.h>


/************************
 *  Data structures
 ***********************/

typedef struct __dxwifi_decoder dxwifi_decoder;

/************************
 *  Functions
 ***********************/

/**
 *  DESCRIPTION:        Initializes the FEC decoder
 * 
 *  ARGUMENTS:
 * 
 *      msglen:         Length, in bytes, of the raw message to encode
 * 
 *      symbol_size:    Desired size of each FEC block
 * 
 *      coderate:       Rate at which to encode repair symbols
 * 
 *  RETURNS:
 *      
 *      dxwifi_decoder*: Allocated and initialized decoder object. Do not attempt
 *      to free this decoder. Please use the close_decoder() method instead. 
 * 
 */
dxwifi_decoder* init_decoder();


/**
 *  DESCRIPTION:        Tearsdown any resources associated with the FEC decoder
 * 
 *  ARGUMENTS:
 *      
 *      encoder:        Pointer to an initialized dxwifi decoder
 * 
 */
void close_decoder(dxwifi_decoder* decoder);


/**
 *  DESCRIPTION:
 * 
 */
size_t dxwifi_decode(dxwifi_decoder* decoder, void* encoded_message);

#endif // LIBDXWIFI_DECODER_H