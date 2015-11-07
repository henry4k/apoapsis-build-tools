#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    int width;
    int height;
    int channels;
    float * data;
} Image;

Image * CreateImage( int width, int height, int channels );
void FreeImage( Image * image );
Image * ReadImage( const char * fileName );
bool WriteImage( const Image * image, const char * fileName );

#ifdef __cplusplus
}
#endif

#endif
