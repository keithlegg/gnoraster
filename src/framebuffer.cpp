
#include <iostream>
#include <fstream>
#include <cstring> //memset 

#include <stdio.h>
#include <cstdint>

#include <cmath>

#include "framebuffer.h"
#include "bmp.h"


using namespace std; 



/*
    BMP bmp9("t1_24.bmp");
    bmp9.fill_region(0, 0, 50, 50, 0, 0, 255, 255);
    bmp9.fill_region(150, 0, 100, 150, 0, 255, 0, 255);
    bmp9.write("t1_24_copy.bmp");

   
    BMP bmp10("t2_24.bmp");
    bmp10.fill_region(0, 0, 50, 50, 0, 0, 255, 255);
    bmp10.fill_region(150, 0, 100, 150, 0, 255, 0, 255);
    bmp10.write("t2_24_copy.bmp");

    BMP bmp5("Shapes_24.bmp");
    bmp5.fill_region(0, 0, 100, 200, 0, 0, 255, 255);
    bmp5.fill_region(150, 0, 209, 203, 0, 255, 0, 255);
    bmp5.write("Shapes_24_copy.bmp");

    // Read an image from disk and write it back:
    BMP bmp("Shapes.bmp");
    bmp.fill_region(0, 0, 100, 200, 0, 0, 255, 255);
    bmp.write("Shapes_copy.bmp");

    // Create a BMP image in memory, modify it, save it on disk
    BMP bmp2(800, 600);
    bmp2.fill_region(50, 20, 100, 200, 0, 0, 255, 255);
    bmp2.write("img_test.bmp");

    // Create a 24 bits/pixel BMP image in memory, modify it, save it on disk
    BMP bmp3(200, 200, false);
    bmp3.fill_region(50, 20, 100, 100, 255, 0, 255, 255);
    bmp3.write("img_test_24bits.bmp");

    BMP bmp4("img_test_24bits.bmp");
    bmp4.write("img_test_24bits_2.bmp");

    BMP bmp6(403, 305, false);
    bmp6.fill_region(0, 0, 50, 50, 0, 0, 255, 0);
    bmp6.write("test6.bmp");

    BMP bmp7("test6.bmp");
    bmp7.fill_region(0, 0, 40, 40, 255, 0, 0, 0);
    bmp7.write("test6_2.bmp");

    BMP bmp8(200, 200, false);
    bmp8.fill_region(0, 0, 100, 100, 255, 0, 255, 255);
    bmp8.write("img_test_24bits_3.bmp");

    BMP bmp11("test_pnet.bmp");
    bmp11.fill_region(0, 0, 100, 100, 255, 0, 255, 255);
    bmp11.write("test_pnet_copy.bmp");

*/



void test_BMP(void)
{

    BMP bmpxx(300 );
    BMP * p_bmpxx = &bmpxx;

    p_bmpxx->show();

    //BMP bmpxx( uint32_t x = 100, uint32_t y = 100 );
    //bmpxx.fill_region(0, 0, 50, 50, 0, 0, 255, 255);

    //bmpxx.write("foobartest.bmp");
}


//     void set_pixel(uint32_t x0, uint32_t y0, uint8_t B, uint8_t G, uint8_t R, uint8_t A) {
//         if (x0 > (uint32_t)bmp_info_header.width || y0 > (uint32_t)bmp_info_header.height) {


/********************************************/

/*
 //RGB color struct
 struct pixelrgb {
     double r;
     double g;
     double b;
 };
*/

   
/*

//        unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0 ,0,0,0,0 , 54,0,0,0};

0h  2   42 4D   "BM"  ID field (42h, 4Dh)
2h  4   46 00 00 00   70 Bytes  Size of the BMP file
6h  2   00 00   Unused  Application specific
8h  2   00 00   Unused  Application specific
Ah  4   36 00 00 00   54 bytes  Offset where the pixel array (bitmap data) can be found  DIB Header

//        unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0,24,0};

Eh  4   28 00 00 00   40 bytes  Number of bytes in the DIB header (from this point)
12h   4   02 00 00 00   2 pixels (left to right order)  Width of the bitmap in pixels
16h   4   02 00 00 00   2 pixels (bottom to top order)  Height of the bitmap in pixels. Positive for bottom to top pixel order. Negative for top to bottom pixel order.
1Ah   2   01 00   1 plane   Number of color planes being used
1Ch   2   18 00   24 bits   Number of bits per pixel
1Eh   4   00 00 00 00   0   BI_RGB, no pixel array compression used
22h   4   10 00 00 00   16 bytes  Size of the raw data in the pixel array (including padding)
26h   4   13 0B 00 00   2,835 pixels/meter  Horizontal resolution of the image
2Ah   4   13 0B 00 00   2,835 pixels/meter  Vertical resolution of the image
2Eh   4   00 00 00 00   0 colors  Number of colors in the palette
32h   4   00 00 00 00   0 important colors  0 means all colors are important Start of pixel array (bitmap data)

//

36h   3   00 00 FF  0 0 255   Red, Pixel (0,1)
39h   3   FF FF FF  255 255 255   White, Pixel (1,1)
3Ch   2   00 00   0 0   Padding for 4 byte alignment (could be a value other than zero)
3Eh   3   FF 00 00  255 0 0   Blue, Pixel (0,0)
41h   3   00 FF 00  0 255 0   Green, Pixel (1,0)
44h   2   00 00   0 0   Padding for 4 byte alignment (could be a value other than zero)

*/

/********************************************/


 
framebuffer::~framebuffer(void) {
   delete rgbdata;  
}

framebuffer::framebuffer( int w, int h) {
       bwidth = w;
       bheight = h;
       n = bwidth * bheight;
       rgbdata = new RGBType[n];

       // degree = radian * (180 / PI) // PI = 3.14159265
       // radian = degree * (PI/180) 
       DEG_TO_RAD = 0.0174532925;
       RAD_TO_DEG = 57.29577951;

       center_x = w/2;
       center_y = h/2;

       plotcolor.r = 1;
       plotcolor.g = 1;
       plotcolor.b = 1;
}

 

void framebuffer::loadbmp (const char *filename, framebuffer::RGBType *data)
{

   //http://www.cplusplus.com/doc/tutorial/files/ 

   // output_image[pix_iterator].b = flat_color;
   //framebuffer::RGBType poly_color; 
   //framebuffer::RGBType vtx_color; 
 
   streampos size;
   char * memblock;

   ifstream file (filename, ios::in|ios::binary|ios::ate);
  
   if (file.is_open())
   {
       size = file.tellg();
       memblock = new char [size];
       file.seekg (0, ios::beg);
       file.read (memblock, size);
       file.close();
   
       cout << "the entire file content is in memory" << endl; 
       delete[] memblock;
   }

   else cout << "Unable to open file";
   //return 0;

}
      


/********************************************/
/*
void framebuffer::savebmp (const char *filename, int w, int h, int dpi, framebuffer::RGBType *data) {
    FILE *f;
    unsigned char *img = NULL;
    int filesize = 54 + 3*w*h;  //w is your image width, h is image height, both int

    img = (unsigned char *)malloc(3*w*h);
    std::memset(img,0,3*w*h);

    int x,y = 0;
    double r,g,b = 0;

    for(int i=0; i<w; i++)
    {
        for(int j=0; j<h; j++)
        {
            x=i; y=(h-1)-j;
            
            r  = (data[(h*y+x)].b)*255;
            g  = (data[(h*y+x)].g)*255;
            b  = (data[(h*y+x)].r)*255;

            if (r > 255) r=255;
            if (g > 255) g=255;
            if (b > 255) b=255;
            img[(x+y*w)*3+2] = (unsigned char)(r);
            img[(x+y*w)*3+1] = (unsigned char)(g);
            img[(x+y*w)*3+0] = (unsigned char)(b);
        }
    }

    unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
    unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
    unsigned char bmppad[3] = {0,0,0};

    bmpfileheader[ 2] = (unsigned char)(filesize    );
    bmpfileheader[ 3] = (unsigned char)(filesize>> 8);
    bmpfileheader[ 4] = (unsigned char)(filesize>>16);
    bmpfileheader[ 5] = (unsigned char)(filesize>>24);

    bmpinfoheader[ 4] = (unsigned char)(       w    );
    bmpinfoheader[ 5] = (unsigned char)(       w>> 8);
    bmpinfoheader[ 6] = (unsigned char)(       w>>16);
    bmpinfoheader[ 7] = (unsigned char)(       w>>24);
    bmpinfoheader[ 8] = (unsigned char)(       h    );
    bmpinfoheader[ 9] = (unsigned char)(       h>> 8);
    bmpinfoheader[10] = (unsigned char)(       h>>16);
    bmpinfoheader[11] = (unsigned char)(       h>>24);

    f = fopen(filename,"wb");
    fwrite(bmpfileheader,1,14,f);
    fwrite(bmpinfoheader,1,40,f);
    for(int i=0; i<h; i++)
    {
        fwrite(img+(w*(h-i-1)*3),3,w,f);
        fwrite(bmppad,1,(4-(w*3)%4)%4,f);
    }

    free(img);
    fclose(f);

}
*/


/********************************************/

/*

// setup header structs bmpfile_header and bmp_dib_v3_header before this (see wiki)
// * note for a windows bitmap you want a negative height if you're starting from the top *
// * otherwise the image data is expected to go from bottom to top *

FILE * fp = fopen ("file.bmp", "wb");
fwrite(bmpfile_header, sizeof(bmpfile_header), 1, fp);
fwrite(bmp_dib_v3_header, sizeof(bmp_dib_v3_header_t), 1, fp);

for (int i = 0; i < 200; i++)  {
 for (int j = 0; j < 200; j++) {
  fwrite(&image[j][i][2], 1, 1, fp);
  fwrite(&image[j][i][1], 1, 1, fp);
  fwrite(&image[j][i][0], 1, 1, fp);
 }
}

fclose(fp);

*/

/********************************************/

void framebuffer::savebmp (const char *filename, int w, int h, int dpi, framebuffer::RGBType *data) {

    FILE *f;
    int k = w*h;
    int s = 4*k;
    int filesize = 54 +s;


    double factor = 39.375;
    int m = static_cast<int>(factor);

    int ppm = dpi*m;
    unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0 ,0,0,0,0 , 54,0,0,0};
    unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0,24,0};

    bmpfileheader[2] = (unsigned char) (filesize);
    bmpfileheader[3] = (unsigned char) (filesize>>8);
    bmpfileheader[4] = (unsigned char) (filesize>>16);
    bmpfileheader[5] = (unsigned char) (filesize>>24);

    bmpinfoheader[4] = (unsigned char) (w);
    bmpinfoheader[5] = (unsigned char) (w>>8);
    bmpinfoheader[6] = (unsigned char) (w>>16);
    bmpinfoheader[7] = (unsigned char) (w>>24);

    bmpinfoheader[8]  = (unsigned char) (h);
    bmpinfoheader[9]  = (unsigned char) (h>>8);
    bmpinfoheader[10] = (unsigned char) (h>>16);
    bmpinfoheader[11] = (unsigned char) (h>>24);

    bmpinfoheader[21] = (unsigned char) (s);
    bmpinfoheader[22] = (unsigned char) (s>>8);
    bmpinfoheader[23] = (unsigned char) (s>>16);
    bmpinfoheader[24] = (unsigned char) (s>>24);

    bmpinfoheader[25] = (unsigned char) (ppm);
    bmpinfoheader[26] = (unsigned char) (ppm>>8);
    bmpinfoheader[27] = (unsigned char) (ppm>>16);
    bmpinfoheader[28] = (unsigned char) (ppm>>24);

    bmpinfoheader[29] = (unsigned char) (ppm);
    bmpinfoheader[30] = (unsigned char) (ppm>>8);
    bmpinfoheader[31] = (unsigned char) (ppm>>16);
    bmpinfoheader[32] = (unsigned char) (ppm>>24);

    f = fopen( filename,"wb");
    fwrite( bmpfileheader, 1, 14, f);
    fwrite( bmpinfoheader, 1, 40, f);

    for (int i = 0; i < k;i++)
    {
        framebuffer::RGBType rgb = data[i];

        double red   = (data[i].b)*255;
        double green = (data[i].g)*255;
        double blue  = (data[i].r)*255;
        
        //uint8_t red   = (data[i].b)*255;
        //uint8_t green = (data[i].g)*255;
        //uint8_t blue  = (data[i].r)*255;

        // -------------------------

        // OSX complains when compiling with -std-c++11 option  
        // error: non-constant-expression cannot be narrowed from type 'int' to 'unsigned char' in 
        //unsigned char color[3] = { (int)floor(blue),(int)floor(green),(int)floor(red) };
        
        //this is modified to compile on OSX - not sure its right (or the other one, for that matter!)
        int color[3] = { (int)(blue),(int)(green),(int)(red) };
        
        //-------------------------

        fwrite (color, 1,3,f);
    }
    fclose(f);

}
 

/********************************************/

void framebuffer::draw_point ( int xcoord, int ycoord ){
   if (xcoord >bwidth){return;}
   if (ycoord >bheight){return;}
   
   if (xcoord > 0 && xcoord > 1){
       if (xcoord < framebuffer::bwidth && xcoord < framebuffer::bheight){
           pix_iterator = (ycoord * framebuffer::bwidth) + xcoord;
           
           rgbdata[pix_iterator].r = plotcolor.r;       
           rgbdata[pix_iterator].g = plotcolor.g;
           rgbdata[pix_iterator].b = plotcolor.b;
       }
   }
}

/********************************************/

void framebuffer::draw_point ( int xcoord, int ycoord, RGBType pcol ){
   if (xcoord >bwidth){return;}
   if (ycoord >bheight){return;}
   
   if (xcoord > 0 && xcoord > 1){  
       if (xcoord < framebuffer::bwidth && xcoord < framebuffer::bheight){  
           pix_iterator = (ycoord * framebuffer::bwidth) + xcoord;    
           rgbdata[pix_iterator].r = pcol.r;       
           rgbdata[pix_iterator].g = pcol.g;
           rgbdata[pix_iterator].b = pcol.b;
       }
   }
}

/********************************************/

void framebuffer::draw_circle ( int x_orig, int y_orig, int dia){

   if (x_orig> bwidth){return;}
   if (y_orig> bheight){return;}

   int plot_x = 0;
   int plot_y = 0;

   for (int i = 0; i <=360; i++)
   {  
       plot_x = x_orig + (sin(deg_to_rad(i))*dia) ;
       plot_y = y_orig + (cos(deg_to_rad(i))*dia) ;
       draw_point(plot_x, plot_y); 
   }
}

/********************************************/

void framebuffer::draw_circle ( int x_orig, int y_orig, int dia, RGBType pcol){
   if (x_orig> bwidth){return;}
   if (y_orig> bheight){return;}

   int plot_x = 0;
   int plot_y = 0;

   for (int i = 0; i <=360; i++)
   {  
       plot_x = x_orig + (sin(deg_to_rad(i))*dia) ;
       plot_y = y_orig + (cos(deg_to_rad(i))*dia) ;
       draw_point(plot_x, plot_y, pcol); 
   }
}

/********************************************/

// degree to radian 
double framebuffer::deg_to_rad ( double deg){
    return deg * DEG_TO_RAD;
}

// radian to degree
double framebuffer::rad_to_deg ( double rad){
    return rad * RAD_TO_DEG;
}

/********************************************/

/* 
   draw_line:

   args:
       x1 - x coord of line start
       y1 - y coord of line start 
       x2 - x coord of line end 
       y2 - y coord of line end 

   use bresenhams algorithm to dram a line using the default color 

*/

void framebuffer::draw_line(int x1, int y1, int const x2, int const y2)
{
   if (x1> bwidth){return;}
   if (y1> bheight){return;}

    int delta_x(x2 - x1);
    // if x1 == x2, then it does not matter what we set here
    signed char const ix((delta_x > 0) - (delta_x < 0));
    delta_x = std::abs(delta_x) << 1;
    int delta_y(y2 - y1);
    // if y1 == y2, then it does not matter what we set here
    signed char const iy((delta_y > 0) - (delta_y < 0));
    delta_y = std::abs(delta_y) << 1;
    draw_point( x1, y1 );
    if (delta_x >= delta_y)
    {
        // error may go below zero
        int error(delta_y - (delta_x >> 1));
        while (x1 != x2)
        {
            if ((error >= 0) && (error || (ix > 0)))
            {
                error -= delta_x;
                y1 += iy;
            }
            // else do nothing
            error += delta_y;
            x1 += ix;
            draw_point(x1, y1);
        }
    }
    else
    {
        // error may go below zero
        int error(delta_x - (delta_y >> 1));
        while (y1 != y2)
        {
            if ((error >= 0) && (error || (iy > 0)))
            {
                error -= delta_y;
                x1 += ix;
            }
            // else do nothing
            error += delta_x;
            y1 += iy;
            draw_point(x1, y1 );
        }
    }
}

/********************************************/

void framebuffer::draw_line(int x1, int y1, int const x2, int const y2, RGBType pcol)
{
    if (x1> bwidth){return;}
    if (y1> bheight){return;}

    int delta_x(x2 - x1);
    // if x1 == x2, then it does not matter what we set here
    signed char const ix((delta_x > 0) - (delta_x < 0));
    delta_x = std::abs(delta_x) << 1;
    int delta_y(y2 - y1);
    // if y1 == y2, then it does not matter what we set here
    signed char const iy((delta_y > 0) - (delta_y < 0));
    delta_y = std::abs(delta_y) << 1;
    draw_point( x1, y1, pcol );
    if (delta_x >= delta_y)
    {
        // error may go below zero
        int error(delta_y - (delta_x >> 1));
        while (x1 != x2)
        {
            if ((error >= 0) && (error || (ix > 0)))
            {
                error -= delta_x;
                y1 += iy;
            }
            // else do nothing
            error += delta_y;
            x1 += ix;
            draw_point(x1, y1, pcol);
        }
    }
    else
    {
        // error may go below zero
        int error(delta_x - (delta_y >> 1));
        while (y1 != y2)
        {
            if ((error >= 0) && (error || (iy > 0)))
            {
                error -= delta_y;
                x1 += ix;
            }
            // else do nothing
            error += delta_x;
            y1 += iy;
            draw_point(x1, y1, pcol);
        }
    }
}





