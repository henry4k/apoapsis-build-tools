#include <assert.h>
#include <math.h> // sqrtf
#include <stdlib.h> // malloc, free
#include <string.h> // memset
#include "normalmap.h"
#include <stdio.h> // DEBUG


typedef struct
{
    int xOffset;
    int yOffset;
    float weight;
} KernelElement;

typedef struct
{
    KernelElement * elements;
    int elementCount;
} Kernel;



const char * NormalMapFilterToString( NormalMapFilter filter )
{
    switch(filter)
    {
        case Prewitt3x3: return "Prewitt3x3";
        case Prewitt5x5: return "Prewitt5x5";
        case Sobel3x3:   return "Sobel3x3";
        case Sobel5x5:   return "Sobel5x5";
        case Scharr3x3:  return "Scharr3x3";
        case Scharr5x5:  return "Scharr5x5";
        case NormalMapFilterCount: ; // fallthrough
    }
    assert(!"Unknown normal map filter.");
    return NULL;
}

static float * CreateRotatedKernelWeights( int size, const float * source )
{
    float * destination = (float *)malloc(sizeof(float)*size*size);
    for(int y = 0; y < size; y++)
    for(int x = 0; x < size; x++)
    {
        destination[x*size + y] = source[y*size + x];
    }
    return destination;
}

static Kernel * CreateKernel( int size, const float * weights )
{
    int elementCount = 0;
    for(int i = 0; i < size*size; i++)
        if(weights[i] != 0)
            elementCount++;
    assert(elementCount > 0);

    KernelElement * elements =
        (KernelElement *)malloc(sizeof(KernelElement)*elementCount);

    int elementIndex = 0;
    for(int i = 0; i < size*size; i++)
    {
        if(weights[i] != 0)
        {
            const int x = i % size;
            const int y = (i-x) / size;
            KernelElement * element = &elements[elementIndex];
            element->xOffset = x - (size / 2);
            element->yOffset = y - (size / 2);
            element->weight = weights[i];
            elementIndex++;
        }
    }
    assert(elementIndex == elementCount);

    Kernel * kernel = (Kernel *)malloc(sizeof(Kernel));
    kernel->elements = elements;
    kernel->elementCount = elementCount;
    return kernel;
}

static void FreeKernel( Kernel * kernel )
{
    assert(kernel->elements != NULL);
    free(kernel->elements);
    memset(kernel, 0, sizeof(Kernel));
    free(kernel);
}

static const float Prewitt3x3XWeights[] =
{
    -1, 0, +1,
    -1, 0, +1,
    -1, 0, +1
};

static const float Prewitt5x5XWeights[] =
{
    -2, -1, 0, +1, +2,
    -2, -1, 0, +1, +2,
    -2, -1, 0, +1, +2,
    -2, -1, 0, +1, +2,
    -2, -1, 0, +1, +2
};

static const float Sobel3x3XWeights[] =
{
    -1, 0, +1,
    -2, 0, +2,
    -1, 0, +1
};

static const float Sobel5x5XWeights[] =
{
    -1,  -2, 0,  +2, +1,
    -4,  -8, 0,  +8, +4,
    -6, -12, 0, +12, +6,
    -4,  -8, 0,  +8, +4,
    -1,  -2, 0,  +2, +1
};

static const float Scharr3x3XWeights[] =
{
     -3, 0,  +3,
    -10, 0, +10,
     -3, 0,  +3
};

static const float Scharr5x5XWeights[] =
{
    -1, -1, 0, +1, +1,
    -2, -2, 0, +2, +2,
    -3, -6, 0, +6, +3,
    -2, -2, 0, +2, +2,
    -1, -1, 0, +1, +1
};

static void CreateXandYKernels( int size,
                                const float * xWeights,
                                Kernel * * xKernel,
                                Kernel * * yKernel )
{
    float * yWeights = CreateRotatedKernelWeights(size, xWeights);
    *xKernel = CreateKernel(size, xWeights);
    *yKernel = CreateKernel(size, yWeights);
    free(yWeights);
}

static void CreateFilterKernels( NormalMapFilter filter,
                                 Kernel * * xKernel,
                                 Kernel * * yKernel )
{
    switch(filter)
    {
        case Prewitt3x3:
            CreateXandYKernels(3, Prewitt3x3XWeights, xKernel, yKernel);
            return;

        case Prewitt5x5:
            CreateXandYKernels(5, Prewitt5x5XWeights, xKernel, yKernel);
            return;

        case Sobel3x3:
            CreateXandYKernels(3, Sobel3x3XWeights, xKernel, yKernel);
            return;

        case Sobel5x5:
            CreateXandYKernels(5, Sobel5x5XWeights, xKernel, yKernel);
            return;

        case Scharr3x3:
            CreateXandYKernels(3, Scharr3x3XWeights, xKernel, yKernel);
            return;

        case Scharr5x5:
            CreateXandYKernels(5, Scharr5x5XWeights, xKernel, yKernel);
            return;

        case NormalMapFilterCount: // fallthrough
            ;
    }
    assert(!"Unknown normal map filter.");
}

static int GetCroppedMapIndex( int width,
                               int height,
                               int x,
                               int y )
{
    if(x < 0)
        x = 0;
    else if(x >= width)
        x = width-1;

    if(y < 0)
        y = 0;
    else if(y >= height)
        y = height-1;

    return y*width + x;
}

static int GetWrappedMapIndex( int width,
                               int height,
                               int x,
                               int y )
{
    if(x < 0)
        x = x+width;
    else if(x >= width)
        x = x-width;

    if(y < 0)
        y = y+height;
    else if(y >= height)
        y = y-height;

    return y*width + x;
}

static float ApplyKernel( const Kernel * kernel,
                          const float * map,
                          int width,
                          int height,
                          bool wrap,
                          int x,
                          int y )
{
    float result = 0;
    for(int i = 0; i < kernel->elementCount; i++)
    {
        const KernelElement * kernelElement = &kernel->elements[i];
        int mapIndex;
        if(wrap)
            mapIndex = GetWrappedMapIndex(width,
                                          height,
                                          x + kernelElement->xOffset,
                                          y + kernelElement->yOffset);
        else
            mapIndex = GetCroppedMapIndex(width,
                                          height,
                                          x + kernelElement->xOffset,
                                          y + kernelElement->yOffset);
        assert(mapIndex >= 0 && mapIndex < width*height);
        result += map[mapIndex] * kernelElement->weight;
    }
    return -result; // TODO: Must be inverted because otherwise the normalmap doesn't looks right. But why?
}

static void Normalize( float * v )
{
    const float length = sqrtf(v[0]*v[0] +
                               v[1]*v[1] +
                               v[2]*v[2]);
    if(length > 1e-04f)
    {
        const float invLength = 1.0f / length;
        v[0] *= invLength;
        v[1] *= invLength;
        v[2] *= invLength;
    }
    else
    {
        v[0] = 0;
        v[1] = 0;
        v[2] = 0;
    }
}

static void NormalToRGB( float * v )
{
    v[0] = v[0]*0.5f + 0.5f;
    v[1] = v[1]*0.5f + 0.5f;
    v[2] = v[2]*0.5f + 0.5f;
    assert(v[0] >= 0 && v[0] <= 1 &&
           v[1] >= 0 && v[1] <= 1 &&
           v[2] >= 0 && v[2] <= 1);
}

void GenerateNormalMap( int width,
                        int height,
                        const float * heightMap,
                        float * normalMap,
                        NormalMapFilter filter,
                        bool wrap )
{
    Kernel * xKernel;
    Kernel * yKernel;
    CreateFilterKernels(filter, &xKernel, &yKernel);

    #pragma omp parallel for collapse(2)
    for(int y = 0; y < height; y++)
    for(int x = 0; x < width;  x++)
    {
        float * normal = &normalMap[(y*width + x)*3];
        normal[0] = ApplyKernel(xKernel, heightMap, width, height, wrap, x, y);
        normal[1] = ApplyKernel(yKernel, heightMap, width, height, wrap, x, y);
        normal[2] = 1.0f;
        Normalize(normal);
        NormalToRGB(normal);
    }

    FreeKernel(xKernel);
    FreeKernel(yKernel);
}
