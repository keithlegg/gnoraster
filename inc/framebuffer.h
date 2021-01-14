
#pragma once

#ifndef framebuffer_H
#define framebuffer_H


void test_BMP(void);



class framebuffer {

   int pix_iterator;

   public:

     int bwidth;
     int bheight;
     
     int center_x; //center of screen
     int center_y; //center of screen
     int n;        //number of pixels

     double DEG_TO_RAD;
     double RAD_TO_DEG;

   
   //RGB color struct
   struct RGBType {
       int r;
       int g;
       int b;
   };

   /*
   //RGBA color struct
   struct RGBAType {
       uint8_t r;
       uint8_t g;
       uint8_t b;
       uint8_t a;
   };*/


   RGBType *rgbdata;  //pointer to our pixel array

   RGBType plotcolor; //color for the pretty lines and dots

   framebuffer( int w, int h);

   ~framebuffer(void);

   static void loadbmp (const char *filename, framebuffer::RGBType *data);
   static void savebmp (const char *filename, int w, int h, int dpi, framebuffer::RGBType *data);

   void draw_point ( int xcoord, int ycoord );
   void draw_point ( int xcoord, int ycoord, RGBType pcol );
   void draw_circle ( int x_orig, int y_orig, int dia);
   void draw_circle ( int x_orig, int y_orig, int dia, RGBType pcol);

   double deg_to_rad ( double deg);
   double rad_to_deg ( double rad);

   void draw_line(int x1, int y1, int const x2, int const y2);
   void draw_line(int x1, int y1, int const x2, int const y2, RGBType pcol);

};


#endif
