#pragma once
class base64Impl {
public:
    base64Impl()
    {
    }
    virtual ~base64Impl() = default;
    virtual size_t b64_get_encoded_buffer_size( const size_t decoded_size ) = 0;
    virtual void b64_encode( const uint8_t *input, const size_t input_size, uint8_t *output ) = 0;
};

class base64 {
public:

    static base64& getInstance()
    {
        static base64 instance;
        return instance;
    }

    base64Impl* impl;

    static size_t b64_get_encoded_buffer_size( const size_t decoded_size )
    {
	    return getInstance().impl->b64_get_encoded_buffer_size(decoded_size);
    }
    
    static void b64_encode( const uint8_t *input, const size_t input_size, uint8_t *output )
    {
            return getInstance().impl->b64_encode(input, input_size, output);
    }

