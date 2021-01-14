#include <iostream>
#include <stdio.h>
#include <cstring>
#include <fstream>

#include <algorithm>


#include "framebuffer.h"
#include "sceneloader.h"



int MAX_CHARS_PER_LINE = 100;
int MAX_TOKENS_PER_LINE = 100;

/*
    there is a stupid bug that the line needs multiple breaks for this to work 
    Obviously I need to learn a lot more about strtok(), and strings in C 
*/

void sceneloader::write_file( char* filepath )
{

}

/*************************************/

void sceneloader::read_file( char* filepath )
{


    std::cout << "sceneloader loading file "<< filepath << "\n";

    std::ifstream fin;
    fin.open(filepath); // open a file
    if (!fin.good()){ 
        std::cout << "scene file \""<< filepath <<"\" appears to be missing." << std::endl;
        exit (EXIT_FAILURE); // exit if file not found
    }

    int n = 0;

    int line_ct = 0;
    while (!fin.eof())
    {
        char buf[MAX_CHARS_PER_LINE];
        fin.getline(buf, MAX_CHARS_PER_LINE);

        const char* token[MAX_TOKENS_PER_LINE];
        token[0] = strtok(buf, " ");
         
        //if line has data on it ...  
        if (token[0]) 
        {
            // walk the space delineated tokens 
            for (n=1; n < MAX_TOKENS_PER_LINE; n++)
            {
                token[n] = strtok(NULL, " \t\n");
                if (!token[n]) break;  
            }



            //----------------------
            if (!strcmp(token[0],"obj_path"))
            {        
                strcpy( object_path, token[1]);
            }

            //////
            line_ct ++; 

        } 


    }

    // ################# 
    // obj_path 3d_obj/monkey.obj
    // cam_matrix_path camera_matrix.olm
    // # THIS IS A TEST OF A COMMENT  

    // object_path;
    // cam_matrix_path;
    // proj_matrix_path;

}



void sceneloader::show()
{

   // cout << " 3D object       " << object_path      << "\n";
   // cout << " camera matrix   " << cam_matrix_path  << "\n";
   // cout << " proj matrix     " << proj_matrix_path << "\n";      

}



