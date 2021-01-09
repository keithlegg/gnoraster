#ifndef IMAGEUTIL_H    
#define IMAGEUTIL_H

#include <png.h>


int writePng24(RGBType  *pixels, char* filename, int w, int h, char* title);
int writePng24(RGBAType *pixels, char* filename, int w, int h, char* title);
int writePng32(RGBType *pixels, char* filename, int w, int h, char* title);
int writePng32(RGBAType *pixels, char* filename, int w, int h, char* title);

/*
void read_png_file(png_bytep *row_pointers, const char* file_name, int *w, int *h, png_structp png_ptr,
                    png_infop info_ptr, png_byte bit_depth, png_byte color_type, int number_of_passes);
*/

int calc_blur_radius(int w, int h);

void cleanup_heap( png_bytep* rowpt, int height);


void write_png_file(png_bytep *row_pointers, const char* file_name, int w, int h, png_structp png_ptr,
                    png_infop info_ptr, png_byte bit_depth, png_byte color_type);

void write_buffer_png(RGBType *pixels, png_bytep *row_pointers, const char* file_name, int w, int h,
                      png_structp png_ptr, png_infop info_ptr, png_byte bit_depth, png_byte color_type);

void write_buffer_png(RGBAType *pixels, png_bytep *row_pointers, const char* file_name, int w, int h,
                      png_structp png_ptr, png_infop info_ptr, png_byte bit_depth, png_byte color_type);

float* autoscan_settings (int width, int height);

int* read_png_fileinfo( const char* file_name, int width, int height, png_structp png_ptr, png_infop info_ptr, 
                        png_byte color_type, png_byte bit_depth );

void saveBMP_24bit ( RGBType *data, const char *filename, int w, int h);
void loadBMP_24bit( RGBType *data, const char *filename, int *w, int *h);

inline void setRGB(RGBAType *ptr, float val);

//static void *
void createTestImage(RGBAType *buffer, int width, int height, float xS, float yS, float rad, int maxIteration);


#endif