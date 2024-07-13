#ifndef XIMER_PNGRECONSTRUCT
#define XIMER_PNGRECONSTRUCT

unsigned char* ximer_reconstruct_pixels_data(const int bytesPerPixel, const int image_width, const int image_height, unsigned char* IDAT_data);

/// @brief x is the byte being filtered
/// @param a  is the byte corresponding to x in the pixel immediately before the pixel containing x (or 0 if such a pixel is out of bounds of the image)
/// @param b is the byte corresponding to x in the previous scanline (or 0 if such a scanline is out of bounds of the image)
/// @param c is the byte corresponding to b in the pixel immediately before the pixel containing b (or 0 if such a pixel is out of bounds of the image)
/// @return 
unsigned char ximer_paeth_predictor(unsigned char a, unsigned char b, unsigned char c);
unsigned char ximer_recon_a(unsigned char* pixels_data,const int stride, const int bytesPerPixel, unsigned char r, unsigned char c);
unsigned char ximer_recon_b(unsigned char* pixels_data,const int stride, const int bytesPerPixel, unsigned char r, unsigned char c);
unsigned char ximer_recon_c(unsigned char* pixels_data,const int stride, const int bytesPerPixel, unsigned char r, unsigned char c);

#endif //XIMER_PNGRECONSTRUCT