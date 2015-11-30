#include <assert.h>
#include <stdio.h> // fprintf
#include <setjmp.h> // setjmp
#include <string.h> // memset
#include <stdlib.h> // malloc, free, abort
#include <png.h>
#include "image.h"


Image * ReadImage( const char * fileName )
{
    FILE * file = fopen(fileName, "rb");
    if(!file)
    {
        fprintf(stderr, "Could not open '%s' for reading.\n", fileName);
        return NULL;
    }

    png_byte header[8];
    const int bytesRead = fread(header, 1, 8, file);
    if(bytesRead != 8 || png_sig_cmp(header, 0, 8) != 0)
    {
        fprintf(stderr, "'%s' is not a valid PNG file.\n", fileName);
        fclose(file);
        return NULL;
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    assert(png);

    png_infop info = png_create_info_struct(png);
    assert(info);

    if(setjmp(png_jmpbuf(png)))
        abort();

    png_init_io(png, file);

    // Let libpng know you already read the first 8 bytes:
    png_set_sig_bytes(png, 8);

    png_read_info(png, info);

    const int width     = png_get_image_width(png, info);
    const int height    = png_get_image_height(png, info);
    const int colorType = png_get_color_type(png, info);
    const int bitDepth  = png_get_bit_depth(png, info);

    // Decode palette images:
    if(colorType == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);

    // Ensure that the image has 8 bit:
    if(bitDepth == 16)
        png_set_strip_16(png);
    if(colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
        png_set_expand_gray_1_2_4_to_8(png);

    if(png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);

    png_read_update_info(png, info);

    png_bytep * rowPointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
    for(int y = 0; y < height; y++)
        rowPointers[y] = (png_byte*)malloc(png_get_rowbytes(png, info));

    png_read_image(png, rowPointers);

    fclose(file);

    // ---------------------------------------------------------------------

    Image * image = (Image *)malloc(sizeof(Image));
    memset(image, 0, sizeof(Image));
    image->width  = width;
    image->height = height;
    switch(colorType)
    {
        case PNG_COLOR_TYPE_GRAY:
            image->channels = 1;
            break;

        case PNG_COLOR_TYPE_GRAY_ALPHA:
            image->channels = 2;
            break;

        case PNG_COLOR_TYPE_RGB:
            image->channels = 3;
            break;

        case PNG_COLOR_TYPE_RGB_ALPHA:
            image->channels = 4;
            break;

        case PNG_COLOR_TYPE_PALETTE:
            image->channels = 3;
            int transparency = 0;
            png_get_tRNS(png, info, NULL, &transparency, NULL);
            if(transparency > 0)
                image->channels++;
            break;

        default:
            assert(!"Unknown color type.");
    }

    const int channels = image->channels;
    assert(png_get_rowbytes(png, info) == width*channels);
    image->data = (float *)malloc(sizeof(float)*width*height*channels);

    for(int y = 0; y < height; y++)
    {
        const png_bytep row = rowPointers[y];
        for(int x = 0; x < width;  x++)
        {
            float * dstPixel = &image->data[y*width*channels + x*channels];
            const png_bytep srcPixel = &row[x*channels];
            for(int i = 0; i < channels; i++)
            {
                dstPixel[i] = (float)srcPixel[i] / 255.f;
            }
        }
    }

    for(int y = 0; y < height; y++)
        free(rowPointers[y]);
    free(rowPointers);

    return image;
}


bool WriteImage( const Image * image, const char * fileName )
{
    int y;

    FILE * file = fopen(fileName, "wb");
    if(!file)
    {
        fprintf(stderr, "Could not open '%s' for writing.\n", fileName);
        return NULL;
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    assert(png);

    png_infop info = png_create_info_struct(png);
    assert(info);

    if(setjmp(png_jmpbuf(png)))
        abort();

    png_init_io(png, file);

    const int width    = image->width;
    const int height   = image->height;
    const int channels = image->channels;

    int colorType;
    switch(channels)
    {
        case 1:
            colorType = PNG_COLOR_TYPE_GRAY;
            break;

        case 2:
            colorType = PNG_COLOR_TYPE_GRAY_ALPHA;
            break;

        case 3:
            colorType = PNG_COLOR_TYPE_RGB;
            break;

        case 4:
            colorType = PNG_COLOR_TYPE_RGB_ALPHA;
            break;

        default:
            assert(!"Unsupported channel count.");
    }

    png_bytep * rowPointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
    for(int y = 0; y < height; y++)
    {
        rowPointers[y] = (png_bytep)malloc(sizeof(png_byte)*width*height*channels);
        png_bytep row = rowPointers[y];
        for(int x = 0; x < width; x++)
        {
            const png_bytep dstPixel = &row[x*channels];
            const float * srcPixel = &image->data[y*width*channels + x*channels];
            for(int i = 0; i < channels; i++)
            {
                int value = srcPixel[i] * 255.f;
                if(value > 255)    value = 255;
                else if(value < 0) value = 0;
                dstPixel[i] = value;
            }
        }
    }

    png_set_IHDR(png,
                 info,
                 width,
                 height,
                 8,
                 colorType,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);

    png_write_image(png, rowPointers);
    png_write_end(png, NULL);

    for(int y = 0; y < height; y++)
        free(rowPointers[y]);
    free(rowPointers);

    fclose(file);
    return true;
}
