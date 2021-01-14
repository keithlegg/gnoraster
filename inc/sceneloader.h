#ifndef SCENELOADER_H    
#define SCENELOADER_H

#include "framebuffer.h"



class sceneloader
{

    public: 
        sceneloader(){
            bmp_path[0]      = 0; //path to a bmp file  
        };

        ~sceneloader(){};


        // framebuffer::RGBType bg_color; 
        // framebuffer::RGBType vtx_color; 
        // framebuffer::RGBType line_color; 
        // framebuffer::RGBType fill_color; 
                
        char bmp_path[100];
        
        void show( void );

        /**************************/
        // file operations 
        void read_file( char* filepath );
        void write_file( char* filepath );

        /**************************/
        // convert_colors

        // translate 
        // rotate 
        // scale 
        // crop 
        // blitter

        // apply bit mask 
        // gaussian blur  
        // contrast  

        // scan 

        
        /**************************/         
        // geom 
 
        // draw line

        // draw triangle 
        // draw fill_triangle 
        
        // draw circle 
        // draw fill_cirlce 
    
        // draw rect
        // draw fill_rect 

        /**************************/



        // Vector3 campos; 
        // float lightintensity;
        // Matrix4 camera_matrix;


        



};




#endif
