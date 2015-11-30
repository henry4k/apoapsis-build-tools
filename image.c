#include <assert.h>
#include <string.h> // memset
#include <stdlib.h> // malloc, free
#include "image.h"


Image * CreateImage( int width, int height, int channels )
{
    Image * image = (Image *)malloc(sizeof(Image));
    image->width    = width;
    image->height   = height;
    image->channels = channels;
    image->data     = (float *)malloc(sizeof(float)*width*height*channels);
    return image;
}

void FreeImage( Image * image )
{
    assert(image->data != NULL);
    free(image->data);
    memset(image, 0, sizeof(Image));
    free(image);
}
