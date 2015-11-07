#include <stdio.h> // printf
#include <string.h> // strcmp
#include <stdlib.h> // atof
#include "image.h"
#include "third-party/edtaa3/edtaa3.h"

static const float DefaultMaxDistance = 16;

static void PrintHelp( const char * programName )
{
    printf("%s [options] <input> <output>\n", programName);

    printf("\t-d <max distance>\n");
}

static bool ParseArguments( int argc,
                            char * * argv,
                            float * maxDistance,
                            const char * * inputFileName,
                            const char * * outputFileName )
{
    for(int i = 1; i < argc; i++)
    {
        if(argv[i][0] == '-')
        {
            if(strcmp(argv[i], "-d") == 0)
            {
                if(i+1 < argc)
                {
                    i++;
                    *maxDistance = atof(argv[i]);
                }
                else
                {
                    printf("Option needs a value.\n");
                    return false;
                }
            }
            else
            {
                printf("Unknown option %s\n", argv[i]);
                return false;
            }
        }
        else if(*inputFileName == NULL)
        {
            *inputFileName = argv[i];
        }
        else if(*outputFileName == NULL)
        {
            *outputFileName = argv[i];
        }
        else
        {
            printf("Too many arguments.\n");
            return false;
        }
    }

    if(*inputFileName  == NULL ||
       *outputFileName == NULL)
    {
        printf("File parameter(s) are missing.\n");
        return false;
    }

    return true;
}

static void GenDistanceField( const char * inputFileName,
                              const char * outputFileName,
                              float maxDistance )
{
    Image * input   = ReadImage(inputFileName);
    Image * outside = CreateImage(input->width, input->height, 1);
    Image * inside  = CreateImage(input->width, input->height, 1);
    Image * output  = CreateImage(input->width, input->height, 1);

    const int pixels = input->width * input->height;

    edtaa3(input->width, input->height, input->data, outside->data);

    // Invert input image:
    for(int i = 0; i < pixels; i++)
        input->data[i] = 1.0f - input->data[i];

    edtaa3(input->width, input->height, input->data, inside->data);

    // Merge inside and outside:
    for(int i = 0; i < pixels; i++)
    {
        float d = -(outside->data[i] - inside->data[i]);

        d = (d / maxDistance) * 0.5f + 0.5;

        // Clamp to 0-1:
        if(d > 1.0f)
            d = 1.0f;
        else if(d < 0.0f)
            d = 0.0f;

        output->data[i] = d;
    }

    WriteImage(output, outputFileName);

    FreeImage(input);
    FreeImage(outside);
    FreeImage(inside);
    FreeImage(output);
}

int main( int argc, char * * argv )
{
    if(argc == 1)
    {
        PrintHelp(argv[0]);
    }
    else
    {
        float maxDistance = DefaultMaxDistance;
        const char * inputFileName = NULL;
        const char * outputFileName = NULL;
        if(!ParseArguments(argc, argv, &maxDistance, &inputFileName, &outputFileName))
            return 1;

        GenDistanceField(inputFileName, outputFileName, maxDistance);
    }
    return 0;
}
