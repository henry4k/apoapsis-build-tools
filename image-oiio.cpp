#include <stdio.h> // fprintf
#include <OpenImageIO/imageio.h>
#include "image.h"



OIIO_NAMESPACE_USING


Image * ReadImage( const char * fileName )
{
    ImageInput * input = ImageInput::open(fileName);
    if(!input)
    {
        fprintf(stderr,
                "Could not open '%s' for reading: %s\n",
                fileName,
                OpenImageIO::geterror().c_str());
        return NULL;
    }

    const ImageSpec & spec = input->spec();
    Image * image = CreateImage(spec.width, spec.height, spec.nchannels);
    if(!input->read_image(image->data))
    {
        fprintf(stderr,
                "'%s': %s",
                fileName,
                input->geterror().c_str());
        return NULL;
    }

    input->close();

    return image;
}

bool WriteImage( const Image * image, const char * fileName )
{
    ImageOutput * output = ImageOutput::create(fileName);
    if(!output)
    {
        fprintf(stderr,
                "Could not create output '%s': %s\n",
                fileName,
                OpenImageIO::geterror().c_str());
        return false;
    }

    ImageSpec spec(image->width, image->height, image->channels);
    if(!output->open(fileName, spec))
    {
        fprintf(stderr,
                "Could not open '%s' for writing: %s\n",
                fileName,
                output->geterror().c_str());
        output->close();
        return false;
    }

    if(!output->write_image(TypeDesc::FLOAT, image->data))
    {
        fprintf(stderr,
                "Can't write material map: %s\n",
                output->geterror().c_str());
        return false;
    }

    output->close();

    return true;
}
