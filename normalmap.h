#ifndef __NORMALMAP_H__
#define __NORMALMAP_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    Prewitt3x3,
    Prewitt5x5,
    Sobel3x3,
    Sobel5x5,
    Scharr3x3,
    Scharr5x5,
    NormalMapFilterCount
} NormalMapFilter;

const char * NormalMapFilterToString( NormalMapFilter filter );

/**
 * Calculate an RGB normal map from a height map.
 *
 * @param heightMap
 * Is expected being an array with width*height elements.
 *
 * @param normalMap
 * Is expected being an array with width*height*3 elements.
 *
 * @param wrap
 * Filter will wrap around the image - useful when image will be tiled.
 *
 * @param invertY
 * Flip the normals Y axis.
 */
void GenerateNormalMap( int width,
                        int height,
                        const float * heightMap,
                        float * normalMap,
                        NormalMapFilter filter,
                        bool wrap,
                        bool invertY );

#ifdef __cplusplus
}
#endif

#endif
