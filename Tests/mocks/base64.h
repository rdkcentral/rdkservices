#pragma once
#ifndef __BASE64_LIB__
#define __BASE64_LIB__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
size_t b64_get_encoded_buffer_size( const size_t decoded_size ){return 0;}
size_t b64url_get_encoded_buffer_size( const size_t decoded_size ){return 0;}
void b64_encode( const uint8_t *input, const size_t input_size, uint8_t *output ){return;}
size_t b64_get_decoded_buffer_size( const size_t encoded_size ){return 0;}
size_t b64url_get_decoded_buffer_size( const size_t encoded_size ){return 0;}
size_t b64_decode( const uint8_t *input, const size_t input_size, uint8_t *output ){return 0;}
size_t b64url_decode( const uint8_t *input, const size_t input_size, uint8_t *output ){return 0;}


#ifdef __cplusplus
}
#endif

#endif /* __BASE64_LIB__ */
