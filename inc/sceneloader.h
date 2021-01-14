#ifndef SCENELOADER_H    
#define SCENELOADER_H


#include "framebuffer.h"
//#include "Vectors.h"



class sceneloader
{

    public: 
        sceneloader(){
            object_path[0]      = 0; //script_path 
        };

        ~sceneloader(){};


        // framebuffer::RGBType bg_color; 
        // framebuffer::RGBType vtx_color; 
        // framebuffer::RGBType line_color; 
        // framebuffer::RGBType fill_color; 
                
        char object_path[100];
        
        void show( void );
        void read_file( char* filepath );
        
        void write_file( char* filepath );

        // Vector3 campos; 
        // float lightintensity;
        // Matrix4 camera_matrix;

        //----------------------
        



};




#endif