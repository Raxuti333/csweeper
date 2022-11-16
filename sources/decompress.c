#include <zlib.h>
#include <stdlib.h>

unsigned char* decompress(const unsigned char* compressed, const unsigned int d_size, const unsigned int i_size)
{
    unsigned char* data = malloc(i_size);

    z_stream infstream;
    infstream.zalloc = Z_NULL;
    infstream.zfree = Z_NULL;
    infstream.opaque = Z_NULL;
    infstream.avail_in = d_size;
    infstream.next_in = (Bytef*)compressed;
    infstream.avail_out = i_size;
    infstream.next_out = data;

    inflateInit(&infstream);
    inflate(&infstream, Z_NO_FLUSH);
    inflateEnd(&infstream);

    return data;
}