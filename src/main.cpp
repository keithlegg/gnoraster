#include <iostream>
#include <stdio.h>
#include <string.h>



#include "point_op.h"
#include "BMP.h"

#include "framebuffer.h"
#include "sceneloader.h"

// #include "scan_core.h"
// #include "image_util.h"

using namespace std;


void test_load_save_bmp(char *infile, char *outfile)
{
    /* someday I will be able to load BMP files as well as save them ... sigh */  
    framebuffer::RGBType* imagedata;
    framebuffer loaded_bmp( 512, 512 );
    framebuffer::loadbmp(infile , imagedata);
    framebuffer::savebmp(outfile, 512, 512, 72, imagedata);
}

/***********************************/
/*
   self contained example of creating a BMP file 
   detached from the renderer, just to show how to driectly 
   create a framebuffer and dump it to disk.

   The colors are a total mess. 
   OSX compiles the BMP exporter function differently.
   See framebuffer.cpp line 184 for a better explanation.
*/
 
void test_image_draw( int width, int height, char *outfile)
{

    framebuffer test_draw( width, height );
    framebuffer *ptest_draw = &test_draw;

    int pix_iterator;

    short flat_color = 128; //background grey color 

    //fill each pixel with a color  
    for (int x = 0; x < width; x++)
    {   
        for (int y = 0; y < height; y++)
        {  
            pix_iterator = y * width + x;     

            test_draw.rgbdata[pix_iterator].r = 254;  //= 0x254;      
            test_draw.rgbdata[pix_iterator].g = 0;  //= 0x0
            test_draw.rgbdata[pix_iterator].b = 0;  //= 0x0

        }
   }

   framebuffer::RGBType drawcolor;
   drawcolor.r = 0x255;
   drawcolor.g = 0x0;
   drawcolor.b = 0x0;

   BMP new_outfile( width, height);
   new_outfile.dump_rgba_data(0, 0, width, height, test_draw.rgbdata);
   new_outfile.write( outfile) ;
} 



/***********************************/


void test_sceneloader(char* filename)
{


    sceneloader RS;
    //char* filename = "eee";
    RS.read_file( filename );

    //test_image_draw( atoi(argv[1]), atoi(argv[2]), argv[3], argv[4]);

}


/***********************************/


int main(int argc, char *argv[])
{

	//test_load_save_bmp
    //test_image_draw(512, 512, "abc.bmp");
    // std::cout << "Hello World!";
    test_sceneloader( argv[1] ); //atoi(argv[1])




    return 0;
}

