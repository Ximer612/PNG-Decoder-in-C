#include "ximer_pngrecostruct.h"

#include <stdlib.h>
#define DEBUG_PRINT
#include <console_utilities.h>

unsigned char ximer_paeth_predictor(unsigned char a, unsigned char b, unsigned char c)
{
    unsigned char Pr;
    unsigned char p = a + b - c;
    unsigned char pa = abs(p - a);
    unsigned char pb = abs(p - b);
    unsigned char pc = abs(p - c);
    if(pa <= pb && pa <= pc)
    {
        Pr = a;
    }
    else if(pb <= pc)
    {
        Pr = b;
    }
    else
    {
        Pr = c;
    }
    return Pr;
}

unsigned char ximer_recon_a(unsigned char* pixels_data,const int stride, const int bytesPerPixel, unsigned char r, unsigned char c)
{
    if(c >= bytesPerPixel)
    {
        return pixels_data[r * stride + c - bytesPerPixel];
    }

    return 0;
}

unsigned char ximer_recon_b(unsigned char* pixels_data,const int stride, const int bytesPerPixel, unsigned char r, unsigned char c)
{
    return r > 0 ? pixels_data[(r-1) * stride + c] : 0;
}

unsigned char ximer_recon_c(unsigned char* pixels_data,const int stride, const int bytesPerPixel, unsigned char r, unsigned char c)
{
    return r > 0 && c >= bytesPerPixel ? pixels_data[(r-1) * stride + c - bytesPerPixel] : 0;
}

unsigned char* ximer_reconstruct_pixels_data(const int bytesPerPixel, const int image_width, const int image_height, unsigned char* IDAT_data)
{
    const int stride = image_width * bytesPerPixel;
    unsigned char* pixels_data = (unsigned char*)malloc(image_height * stride);

    int i = 0;

    for (int r = 0; r < image_height; r++) //for each scanline
    {
        unsigned char filter_type = IDAT_data[i]; // first byte of scanline is filter type
        i++;

        for (int c = 0; c < stride; c++) //for each byte in scanline
        {
            unsigned char filt_x = IDAT_data[i];
            unsigned char ftmp_recon;
            i++;
            unsigned char recon_x;

            switch (filter_type)
            {
            case 0:// None
                ftmp_recon = 0;
                recon_x = filt_x;
                //RED_PRINT("None filter type: %02x - %02x", filter_type, recon_x);
                break;
            case 1:// Sub
                ftmp_recon = ximer_recon_a(pixels_data,stride,bytesPerPixel,r, c);
                //RED_PRINT("Sub filter type: %02x - %02x", filter_type, recon_x);
                break;
            case 2:// Up
                ftmp_recon = ximer_recon_b(pixels_data,stride,bytesPerPixel,r,c);
                //RED_PRINT("Up filter type: %02x - %02x", filter_type, recon_x);
                break;
            case 3:// Average
                ftmp_recon = ximer_recon_a(pixels_data,stride,bytesPerPixel,r, c) + ximer_recon_b(pixels_data,stride,bytesPerPixel,r,c); // 2
                //RED_PRINT("Average filter type: %02x - %02x", filter_type, recon_x);
                break;
            case 4:// Paeth
                ftmp_recon = ximer_paeth_predictor(ximer_recon_a(pixels_data,stride,bytesPerPixel,r, c), ximer_recon_b(pixels_data,stride,bytesPerPixel,r,c), ximer_recon_c(pixels_data,stride,bytesPerPixel,r,c));
                //RED_PRINT("Paeth filter type: %02x - %02x", filter_type, recon_x);
                break;
            default:
                RED_PRINT("unknown filter type: %02x", filter_type);
                exit(-1);
                continue;
            }

            recon_x = filt_x + ftmp_recon;
            pixels_data[r * stride + c] = recon_x;
        }
    }

    return pixels_data;
}