#include <iostream>
#include <stdio.h>
#include <cstring>
#include <algorithm>


#include "framebuffer.h"



int MAX_CHARS_PER_LINE = 100;
int MAX_TOKENS_PER_LINE = 100;

/*
    there is a stupid bug that the line needs multiple breaks for this to work 
    Obviously I need to learn a lot more about strtok(), and strings in C 
*/

void sceneloader::load_file( char* filepath )
{
    cout << "sceneloader loading file "<< filepath << "\n";

    ifstream fin;
    fin.open(filepath); // open a file
    if (!fin.good()){ 
        cout << "scene file \""<< filepath <<"\" appears to be missing." << endl;
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

            //----------------------
            if (!strcmp(token[0],"cam_matrix_path"))
            {            
                strcpy( cam_matrix_path, token[1]);
            }

            //----------------------
            if (!strcmp(token[0],"cam_pos"))
            {        
                campos = Vector3( atof(token[1]), atof(token[2]), atof(token[3]) );
            }

            //----------------------
            if (!strcmp(token[0],"light_pos"))
            {            
                //cout << " light position is " <<  atof(token[1]) << " " <<  atof(token[2]) << " " <<  atof(token[3]) << "\n";
                lightpos = Vector3( atof(token[1]), atof(token[2]), atof(token[3]) );

            }

            //----------------------
            if (!strcmp(token[0],"light_intensity"))
            {            
                lightintensity = atof(token[1]);
            }

            //----------------------
            if (!strcmp(token[0],"bg_color"))
            {            
                bg_color.r = atoi(token[1]);
                bg_color.g = atoi(token[2]);
                bg_color.b = atoi(token[3]);                                
            }

            //----------------------
            if (!strcmp(token[0],"line_color"))
            {            
 
                //cout << " line_color is " <<  atof(token[1]) << " " <<  atof(token[2]) << " " <<  atof(token[3]) << "\n";
                line_color.r = atoi(token[1]);
                line_color.g = atoi(token[2]);
                line_color.b = atoi(token[3]);
            }

            //----------------------
            if (!strcmp(token[0],"fill_color"))
            {            
                //cout << " fill_color is " <<  atof(token[1]) << " " <<  atof(token[2]) << " " <<  atof(token[3]) << "\n";
                fill_color.r = atoi(token[1]);
                fill_color.g = atoi(token[2]);
                fill_color.b = atoi(token[3]);
            }

            //----------------------
            if (!strcmp(token[0],"vtx_color"))
            {   
                //cout << " vtx_color is " <<  atof(token[1]) << " " <<  atof(token[2]) << " " <<  atof(token[3]) << "\n";
                vtx_color.r = atoi(token[1]);
                vtx_color.g = atoi(token[2]);
                vtx_color.b = atoi(token[3]);                
            }
           
            //----------------------
            if (!strcmp(token[0],"show_vtx"))
            {   
                if (!strcmp(token[1],"true"))
                {
                    show_vtx = true;
                }
            }

            //----------------------
            if (!strcmp(token[0],"show_lines"))
            {   
                if (!strcmp(token[1],"true"))
                {
                    show_lines = true;
                }
                cout << "show lines "<< show_lines << "\n";
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

   cout << " 3D object       " << object_path      << "\n";
   cout << " camera matrix   " << cam_matrix_path  << "\n";
   cout << " proj matrix     " << proj_matrix_path << "\n";      

}



