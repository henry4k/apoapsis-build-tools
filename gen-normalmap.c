#include <stdio.h> // printf
#include <string.h> // strcmp
#include "image.h"
#include "normalmap.h"

static const NormalMapFilter DefaultFilter = Sobel3x3;

static void PrintHelp( const char * programName )
{
    printf("%s [options] <input> <output>\n", programName);

    printf("\t-f <filter> (");
    for(int i = 0; i < NormalMapFilterCount; i++)
    {
        printf("%s", NormalMapFilterToString((NormalMapFilter)i));
        if(i != NormalMapFilterCount-1)
            printf(", ");
    }
    printf(")\n");

    printf("\t-w (enable wrapping)\n");
}

static NormalMapFilter GetFilterByName( const char * name )
{
    for(int i = 0; i < NormalMapFilterCount; i++)
    {
        const NormalMapFilter filter = (NormalMapFilter)i;
        if(strcmp(name, NormalMapFilterToString(filter)) == 0)
            return filter;
    }
    return DefaultFilter;
}

static bool ParseArguments( int argc,
                            char * * argv,
                            NormalMapFilter * filter,
                            bool * wrap,
                            const char * * inputFileName,
                            const char * * outputFileName )
{
    for(int i = 1; i < argc; i++)
    {
        if(argv[i][0] == '-')
        {
            if(strcmp(argv[i], "-f") == 0)
            {
                if(i+1 < argc)
                {
                    i++;
                    *filter = GetFilterByName(argv[i]);
                }
                else
                {
                }
            }
            else if(strcmp(argv[i], "-w") == 0)
            {
                *wrap = true;
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

int main( int argc, char * * argv )
{
    if(argc == 1)
    {
        PrintHelp(argv[0]);
    }
    else
    {
        NormalMapFilter filter = DefaultFilter;
        bool wrap = false;
        const char * inputFileName = NULL;
        const char * outputFileName = NULL;
        if(!ParseArguments(argc, argv, &filter, &wrap, &inputFileName, &outputFileName))
            return 1;

        Image * input = ReadImage(inputFileName);
        Image * output = CreateImage(input->width, input->height, 3);

        GenerateNormalMap(input->width,
                          input->height,
                          input->data,
                          output->data,
                          filter,
                          wrap);

        WriteImage(output, outputFileName);

        FreeImage(input);
        FreeImage(output);
    }
    return 0;
}
