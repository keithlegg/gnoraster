
/*
   - "Battleship" computer vision fiducial locator algorithm - 
   Author     - Keith Legg 
   Created    - July 30, 2014 - August 2. 2014.
   Modified   - October 18, 2014
   Modified   - December 2, 2014

   ###############
     Beware the effect of diagnostic actually affecting the outcome!
     If you change a pointer to a pixel value before you are done scanning - it can change the results!
     I have attempted to fix this by splitting the diagnostic framebuffer into another area of memory. 
   ###############

   There are 4 steps of filtering to determine a fiducial anchor.

   1 - the blob is large enough on X  (about 1/90th of long edge, width)
   2 - the blob is large enough on Y  (about 1/90th of long edge, width)
     - ( the blog is symetrical )
   3 - the blob is under a maximum size (set high if you are unsure!)
     - the x and y hits are concentrated near each other (more of a hack than a feature)
   4 - the blob is surrounded by a white and black concentric ring 
    - the ratios of the white and black ring are width proportionate 

*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

//#include <string.h>  //strcmp
#include <ctype.h>  //

#include <cmath>

#include "point_op.h" 
#include "framebuffer.h"   
#include "scan_core.h"

/*****************/
//changed these to variables so we can set dynamically
bool show_diag_msg         = false;   
bool do_diagnose           = false; // output a diagnostic png image 

bool diagnose_final_fid    = false; // show the fiducial anchors verified           - magenta 
bool diagnose_anchors      = false; // show the possible hit anchors located        - orange
bool diagnose_verify       = false; // show the pixels N S E W                      - blue 
bool diagnose_confidence   = false; // draw a squre at the end of N E S W  if pass  - red  
bool diagnose_verif_angles = false; // show the verified fiducial angles            - red
bool diagnose_verif_rings  = false; // show the verified rings that pass check      - red

bool diagnose_firsthit     = false; // white to black transition of a blob          - red/green
bool diagnose_lasthit      = false; // black to white transition of a blob          - red/green
bool diagnose_midpoints    = false; // midpoint of a blob                           - yellow/off yellow
bool diagnose_battleship   = false; // show every cached hit and the sqaure it looks for hits in (determined by fine setting) 

bool dump_cache_data       = false; // dump a text file of the scanned fiducial data 

//a bit convoluted - this dynamically sums up the TOTAL confidence possible for ANY SINGLE fiducial so we can avergae it later 
int spokes                 = 20; //number of linear scans in verify (each spoke adds 1 unit of confidence)
int angle_check_confidence = 2;
int rings_confidence       = 2; //value of each ring (multiplied by 3 below for actual result)
int max_confidence         = spokes + angle_check_confidence + (rings_confidence*3); // The maximum possible fiducial confidence

/*****************/
//cache array variables go here 

// this is the cache for potential fiducials 
short scanCache[cachesize][cachesize];        
short *ptrScCache  = &scanCache[0][0];
int scan_cache_count = 0;

// this is the cache for fiducial confidence 
fiducial fiducials[confidence_checksize];
fiducial *ptFiducials = &fiducials[0];

// this is the cache for confirmed fiducials 
pix_coord cache_hits[anchor_checksize];
pix_coord *ptChitz = &cache_hits[0];

// this is the cache for the final pass fiducials 
//fiducial third_pass[anchor_checksize];
//fiducial *ptLastPass = &third_pass[0];

/*****************/
void log_verify(char* msg){
    if ( debug_verify ){
        printf(msg);
    }
}

/*****************/
/*
  //DEBUG - NOT DONE !!
  experiment to load a cache from a text file into memory to debug and test things 
*/

void read_cache( char *filename )
{
    FILE *f = fopen(filename, "r");
    int c;

    if (f == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }else{

        int found_word = 0;
        while ((c =fgetc(f)) != EOF )
        {
            if (!isalpha(c))
            {
                if (found_word) {
                    putchar('\n');
                    found_word = 0;
                }
            }
            else {
                found_word = 1;
                c = tolower(c);

                //printf("%i \n", c);
                //putchar(0x42);

                /*
                char str1[15];
                int ret;
                strcpy(str1, "abc");
                ret = strcmp(str1, (char)c);

                if (ret){
                printf("match found!");
                }
                */

            }

        }

    }
    fclose(f);
}

/*
   Dump out all the cached fiducial data to a text file.
   pictures are pretty, but sometimes you need real data in searchable text form.
*/

void dump_cache( char *filename, int width, int height, int coarse , int max , int fine)
{

    FILE *f = fopen(filename, "w");
    if (f == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }

    const char *divider = "##################################";
    /********/
    fprintf(f, "Cache size: %i (x*y) out of %i possible.\n", scan_cache_count, ((int)cachesize*cachesize)); 
    fprintf(f, "Scanned image resolution: width %i height %i\n", width, height );
    fprintf(f, "Scan settings: coarse %i max %i fine %i \n", coarse, max, fine );
    fprintf(f, "%s\n", divider);  

    for (int x=0;x<confidence_checksize;x++)
    {
        if (fiducials[x].x !=0 && fiducials[x].y !=0 ){
            fprintf(f, "id: %i confidence: x %i y %i conf %i\n", fiducials[x].id, fiducials[x].x, fiducials[x].y, fiducials[x].confidence );  
        }
    }

    /********/
    fprintf(f, "%s\n", divider); 
    //ALL THE "ORANGES" confirmed X and Y overlaps  
    for (int x=0;x<anchor_checksize;x++)
    { 
        if (cache_hits[x].x !=0 && cache_hits[x].y != 0){
            fprintf(f, "anchor: x %i y %i\n", cache_hits[x].x, cache_hits[x].y );                
        }
    }


    /********/
    /*
    fprintf(f, "%s\n", divider); 

    //ALL THE "YELLOWS" X and Y midpoints 
    for (int x=0;x<cachesize;x++)
    { 
         for (int y=0;y<cachesize;y++)
         { 
             if (scanCache[x][y] !=0){
                fprintf(f, "midpt: x %i y %i\n", x , y);                
             }

         }
    }
    */
    /********/
    fclose(f);
}

int check_cache_distance( int smpl[2], pix_coord out_coords[], int num, int distance)
    /* check in existing memory cache for a canidate fiducial's distance to existing one 
       the idea is to filter out redundant points that are too close to each other (or the same)
       Return a 1 if it is OKAY to cache a new point. 
    */
{
    distance = int(distance / 3);
    for(int k=0;k<num;k++)
    {
        int pt[2]= {out_coords[k].x, out_coords[k].y };

        //check if it is an exact match
        if (out_coords[k].x == smpl[0] && out_coords[k].y == smpl[1]){
            return 0; //points are the same - dont cache the new one
        }

        //check if it is close (under specified distance) 
        if (fcalc_distance( pt, smpl ) < distance){
            return 0;//points are spatailly too close - dont cache the new one
        }
    }//iterate all cached hits to check distances 
    return 1;
}

/*****************************/
int fiducial_compare(const fiducial *f1, const fiducial *f2)
// evaluate two fiducial's confidence level.
{
   if ( f1->confidence > f2->confidence)
      return -1;
   else if (f1->confidence < f2->confidence)
      return 1;
   else
      return 0;
}

/*
   This scans the whole point cache and evaluates which 3 fiducials have a 90 degree relationship to each other.
   Computationally very heavy and I am not conviced it is even a usefull too since alomost all the fiducials end up having a 90 degree connection.

   In order to really be usefull it neeeds to be 90 degrees AND orinted to the page , but we cant calculate that since we done deal with angle here.
   (I.E. theta of right triangle in python code)
 
   Because incoming page is arbitrarily rotated (not rotationally adjusted), we have no basis to evaluate the 90 degree alignment.


   TODO:
       would be useful to add a feature to read in disk caches and visualize the angles between them 
*/

float check_fiducial_angles( RGBAType *pt_fbInput, RGBAType *pt_fbDiag, int imagewidth, fiducial *ptFidlist, int num )
{
    float error_angle = .2;// number of degrees +/-(90) 
    bool showDebug = false;

    RGBType c1 = newRgb(255, 0  , 0);
    RGBType c2 = newRgb(0  , 255, 0);
    RGBType c3 = newRgb(0  , 0, 0);

    if (showDebug){
        for (int a=0;a<num;a++ )
        {
            printf(" Dump coords %i id %i x %i, y %i \n" , a, ptFidlist[a].id, ptFidlist[a].x, ptFidlist[a].y );
        }
    }

    float angle = 0;

    for (int a=0;a<num;a++ )
    {
        for (int b=0;b<num;b++ )
        {     
            for (int c=0;c<num;c++ )
            {                       
                if (a!=b && a!=c)
                {
                    if (b!=c)
                    {
                        //printf(" index %i %i  %i \n", a, b, c );
                        vector2d v1 = line2vect( ptFidlist[a].x, ptFidlist[a].y, ptFidlist[b].x  , ptFidlist[b].y   );
                        vector2d v2 = line2vect( ptFidlist[a].x, ptFidlist[a].y, ptFidlist[c].x  , ptFidlist[c].y );
                        angle = angle_between( v1, v2);

                        //printf(" index %i %i %i  angle is %f \n", a, b, c, angle);

                        if (angle>(float)(90-error_angle) && angle < (float)(90+error_angle) )
                        {
                            //printf("ANGLE IS CLOSE!!! \n");
                            if (diagnose_verif_angles){
                                draw_line( pt_fbDiag, imagewidth, ptFidlist[a].x, ptFidlist[a].y, ptFidlist[b].x  , ptFidlist[b].y , &c2 );                      
                                draw_line( pt_fbDiag, imagewidth, ptFidlist[a].x, ptFidlist[a].y, ptFidlist[c].x  , ptFidlist[c].y , &c2 );   
                            }

                            //increse the confidence of all three if they are on a 90 degree alignment
                            //only increase confidence for a single fiducial ONE TIME !!
                            if (ptFidlist[a].is_angled_checked==false){
                                ptFidlist[a].confidence = ptFidlist[a].confidence + angle_check_confidence;
                                ptFidlist[a].is_angled_checked = true;
                            }
                            if (ptFidlist[b].is_angled_checked==false){                             
                                ptFidlist[b].confidence = ptFidlist[b].confidence + angle_check_confidence;
                                ptFidlist[b].is_angled_checked = true;
                            }
                            if (ptFidlist[c].is_angled_checked==false){                             
                                ptFidlist[c].confidence = ptFidlist[c].confidence + angle_check_confidence;
                                ptFidlist[c].is_angled_checked = true;  
                            }

                        }else{
                            // these work , but they get really visually busy and will draw over the desired green lines.

                            //draw_line( pt_fbDiag, imagewidth, ptFidlist[a].x, ptFidlist[a].y, ptFidlist[b].x  , ptFidlist[b].y , &c1 );                      
                            //draw_line( pt_fbDiag, imagewidth, ptFidlist[a].x, ptFidlist[a].y, ptFidlist[c].x  , ptFidlist[c].y , &c1 );                           
                        }
                    }
                }
            }  
        }//second loop
    } //first loop

} 

int scan_darkpt (RGBAType *pt_fbInput, RGBAType *pt_fbDiag, pix_coord pt, int width, int height )
{
    /*
    a form of thresholding - determine if a pixel is considered dark or not.
    used in linear_scan, which is used as a "spoke" in the verification wheel.

    */

    int out = 0;

    //keep from running off page 
    if (pt.x >= width ||  pt.x < 0 || pt.y >= height ||  pt.y < 0){        
        return -1;
    }

    // internal structure 
    RGBAType* pixPtr  = &( pt_fbInput [(pt.y*width) + pt.x]);
    RGBAType* diagPtr = &( pt_fbDiag  [(pt.y*width) + pt.x]);    

    //check if pixel is black or white by averaging   
    if( scanner_darkly(pixPtr) )
    {
        if (diagnose_verify)
        {
            diagPtr->r = 0;diagPtr->g = 255;diagPtr->b =255;diagPtr->a =255;
        }      
        return 1;//pixel is dark 
    }else{
        if (diagnose_verify)
        {
            diagPtr->r = 128;diagPtr->g = 0;diagPtr->b =128;diagPtr->a =255;
        }       
        return 0;//pixel is NOT dark 
    }
    return 0;
}

/* 
   Input:
       list of interger values (0 or 1) representing pixels that are dark or not 

   Output:
       bool "true" if line averages to 1, "false" if it averages to 0. 

   // example usage 

   int testlum[4000];   
   testlum[0] = 1;
   testlum[1] = 1;
   average_pixels(&testlum[0], 4);

*/

bool average_pixels(int *values, int num){
    int sum = 0;
    for (int x=0;x<num;x++){sum = sum + values[x];}

    int mean = (round)((float)(sum)/(float)(num)); //0 or 1 

    if (mean<1){
        return false;
    }
    else{
        return true;   
    }

    return false;
}

/*
  take a list of coordinates and "connect-the-dot" scan them for a true or false averaged result
  it takes a list of vertices , scans the lines of pixels between them and averages it to a 1 or 0 
  if they are black or white.
*/

int check_polyline_luminance(RGBAType *pt_fbInput, RGBAType *pt_fbDiag, int imagewidth, int imageheight, 
pix_coord *ptPoints, int numpts, int scan_coarse )
{
    int num = 0;
    int *ptNum = &num;
    int p1[2] = {0};
    int p2[2] = {0};

    pix_coord workbuffer[1000] = {0};  // storage for pixel coordinates  
    pix_coord *ptBuff = &workbuffer[0];   

    int luminances[4000] = {0};       // storage for pixel luminace values   
    int lumenIdx = 0;           // luminance index 

    //int *ptLumens = &luminances[0];   

    int i = 0; 
    //really should be an even number - pretty much use 4 for now (square=circle/90)   
    for(i=1;i<=numpts;i++)
    {
        if (i<numpts)
        { 
            p1[0] = ptPoints[i-1].x; 
            p1[1] = ptPoints[i-1].y;
            p2[0] = ptPoints[i].x; 
            p2[1] = ptPoints[i].y;
        }

        //close the (3) points so they become a connected circle to the 
        if (i==numpts)
        { 
            p1[0] = ptPoints[i-1].x; 
            p1[1] = ptPoints[i-1].y;
            p2[0] = ptPoints[0].x; 
            p2[1] = ptPoints[0].y;  
        }

        num = 0; //reset the count each cycle
        calc_line(ptBuff, p1, p2, ptNum);
        //printf("###NEW ROW %i - num pixels is %i \n", i, num);

        for (int x=0; x<num ;x++ )
        {
            //fill buffer 
            luminances[lumenIdx] = scan_darkpt(pt_fbInput, pt_fbDiag, ptBuff[x], imagewidth, imageheight);
            lumenIdx++; //keep a count of all pixels sampled 
        }

    }//iterate each point

    int mean_lum = average_pixels(&luminances[0], lumenIdx); //rounded to 0 or 1 

    //draw debugging info 
    if(diagnose_verif_rings){
        RGBType c3 = newRgb(255, 0, 0);

        if(mean_lum){
            for(i=1;i<=numpts;i++)
            {
                if (i<numpts){
                    draw_line( pt_fbDiag, imagewidth, ptPoints[i-1].x,  ptPoints[i-1].y, ptPoints[i].x  , ptPoints[i].y , &c3 );      
                }
                if (i==numpts){
                    draw_line( pt_fbDiag, imagewidth, ptPoints[i-1].x,  ptPoints[i-1].y, ptPoints[0].x  , ptPoints[0].y , &c3 ); 
                }
            }     
        }

        if(! mean_lum){
            RGBType c3 = newRgb(0, 255, 0);
            for(i=1;i<=numpts;i++)
            {
                if (i<numpts){
                    draw_line( pt_fbDiag, imagewidth, ptPoints[i-1].x,  ptPoints[i-1].y, ptPoints[i].x  , ptPoints[i].y , &c3 );      
                }
                if (i==numpts){
                    draw_line( pt_fbDiag, imagewidth, ptPoints[i-1].x,  ptPoints[i-1].y, ptPoints[0].x  , ptPoints[0].y , &c3 ); 
                }
            }     
        }

    }

    return mean_lum;
}

/*****************************/
/*
  specialized tool for evaluating radiating lines (pixels) from the center of a fiducial 
  looks for specific things:
   the red dot in the diagnostic image indicates the line passed this check 
   
   - "black-white-black" pattern
   - ratios of black to white are proportional to the coarse setting, etc
   
*/

int linear_scan(RGBAType *pt_fbInput, RGBAType *pt_fbDiag, int width, int height, int p1[2], int p2[2], int scan_coarse )
{
    /* combination of PYCV linear scan and verify fiducial  */

    pix_coord pointz[100];
    pix_coord *ptFoo = &pointz[0];

    int num = 0;
    int *ptNum = &num;

    calc_line(ptFoo, p1, p2, ptNum);

    int analyze[50]  = {0}; //use to walk and cache changes in color 

    int patterns[20] = {0}; //use to store sequence of changes 
    int framesize = 0;
    int changecount = 0;
    int last_pix  = 0;

    int first,second,third = 0;

    //color of indicator 
    int verifycolor[3] = {0};

    for (int x=0; x<num ;x++ )
    {
        //fill buffer 
        analyze[x] = scan_darkpt(pt_fbInput, pt_fbDiag, pointz[x], width, height); 

        if (analyze[x]==-1){return 0;} //run off edge optimization

        if(analyze[x]!=last_pix)
        {
            if (changecount==0){
                first = analyze[x]; 
                patterns[changecount] = framesize;
            }
            if (changecount==1){
                second = analyze[x]; 
                patterns[changecount] = framesize;           
            }
            if (changecount==2){
                third = analyze[x];
                patterns[changecount] = framesize; 

                //break out if not black white black 
                if(first != 1){
                    log_verify("verify: second segment not white\n");    
                    return 0;
                }
                else if (second!=0){
                    log_verify("verify: second segment not white\n");                
                    return 0;
                }
                else if (third !=1){
                    log_verify("verify: third segment not black\n");                    
                    return 0;
                }
            }

            if (changecount==3){
                patterns[changecount] = framesize;          
            }

            if (changecount==4){
                patterns[changecount] = framesize;          
            }

            changecount++;   

            framesize =0;
        }//if change detected 

        framesize++;       
        last_pix = analyze[x];     
    }//
    if (debug_verify){
        printf("####\n");
        for (int i=0;i<changecount;i++){
            printf("patterns %i \n", patterns[i]);

        }
    }
    //if pattern is BLACK - WHITE - BLACK 
    if(first == 1 && second==0 && third ==1)
    {
        //check that the first black is at least half of coarse 
        if ( patterns[1]>= ((int)(scan_coarse/2) ) )
        {
            log_verify("verify: passes - 1st (black) is bigger than half of coarse setting.\n");  
            //if the 1st (black) is larger than half of the second (white) 
            if ( patterns[1]>=(patterns[2]*1.2)  )  //if ( patterns[1]>=(patterns[2]*1.2)-error_px )       
            {
                log_verify("verify: passes - 1st (black) is bigger than half of second (white).\n");  
                // if the outer band (black) is larger than half of the white band 
                // also check that the white band is bigger than 1/5 of coarse
                if ( (patterns[3] * 1.2) >= patterns[2] && (patterns[2]>scan_coarse/5) )         
                {
                    log_verify("verify: passes - 2nd (white) is about equal to third (black).\n"); 
                    //show the end of the spoke in green if it passes check 
                    if (diagnose_confidence){
                        verifycolor[0]=0;verifycolor[1]=255; 
                        draw_fill_square(pt_fbDiag, width, p2[0], p2[1], 2, verifycolor);
                    }

                    //check that outer black ring is larger than half of the second white
                    //also check that the outer black band is less than half of coarse setting               
                    if ( patterns[3]>(int)(patterns[1]*.4) && patterns[3]<(scan_coarse/2) ){
                        //show the end of the spoke in red if it passes check 
                        if (diagnose_confidence){
                            verifycolor[0]=255;verifycolor[1]=0;                         
                            draw_fill_square(pt_fbDiag, width, p2[0], p2[1], 3, verifycolor);
                            //printf("a=%i, b=%i, c=%i at %ix%i\n", patterns[1], patterns[2], patterns[3], p2[0], p2[1]);
                        }
                        return 1;
                    }
                }
                return 0;
            }
            return 0;
        }//first black is half of coarse setting
    }else{
        log_verify("verify: fails - black white black check.\n");  
        return 0;
    } 
} 

/*
  another tool for fiducial verification - 3 square concentric rtings that average to black or white.
*/
int verify_rings(RGBAType *pt_fbInput, RGBAType *pt_fbDiag, int imagewidth, int imageheight,
                 float diameter, int xpos, int ypos, int scan_coarse, int scan_max)
{

    pix_coord circlePts[10];  
    pix_coord *ptCirc = &circlePts[0];

    int confidence = 0;
    int sample = 0;
    //RGBType red; red.g=255; //actually green at the moment 
    
    int i = 0;
    int step = 0;
    float step_tweak = 1; // ratio to shift different boxes towards or away from center
    int step_size = (int)scan_coarse/1.89; //polar distance between rings from center- (diameter)
    int ringcount = 0;

    for(i=0; i<3; i=i+1)
    {
        int numgen = 0;

        if (i==1){
            step_tweak = 1.08; // The second ring needs to be pushed out a tiny bit
        } else {
            step_tweak = 1;
        }
        step = (i+1) * step_size *  step_tweak;

        calc_circle ( ptCirc, 4, xpos, ypos, step, &numgen); 
        //draw_poly_line(pt_fbDiag, imagewidth, ptCirc, numgen, &red);  

        //this will return a 0 if the line averages into white , 1 if it is black 
        //pattern we want is black white black, or 1-0-1 
        sample = check_polyline_luminance( pt_fbInput, pt_fbDiag, imagewidth, imageheight, ptCirc, numgen, scan_coarse ); 

        //break out if the pattern is not what we are looking for 
        if (i==0){
            if (sample!=1){
                return 0;
            }
            confidence = confidence + rings_confidence;
        }
        if (i==1){
            if (sample!=0){
                return 0;
            }
            confidence = confidence + rings_confidence;            
        }
        if (i==2){
            if (sample!=1){
                return 0;
            }
            confidence = confidence + rings_confidence;             
        }
    }
  
    //DEBUG    
    //printf("DEBUG RING CONFIDENCE IS %i \n", confidence);

    return confidence; //this is worth more than the "wheel spokes" verif check - double it for good measure
}

/********************************************/
/*
   this simply generates a "wheel" of radiating spokes.
   The real work is done inside linear_scan, this just sets it up to be run dynamically.

*/

int verify_wheel(RGBAType *pt_fbInput, RGBAType *pt_fbDiag, int imagewidth, int height,
                 int spokes, float diameter, int xpos, int ypos, int scan_coarse)
{

    pix_coord circlePts[360];//360 or less 
    pix_coord *ptCirc = &circlePts[0];

    int numgen = 0;
    
    int p1[2]={0};
    int p2[2]={0};

    calc_circle ( ptCirc, spokes, xpos, ypos, diameter, &numgen);    

    int scan_line  = 0;
    int confidence = 0;

    for(int i=0;i<numgen;i++)
    {    
        p1[0]=xpos;
        p1[1]=ypos;
        p2[0]=circlePts[i].x;
        p2[1]=circlePts[i].y;

        if (linear_scan(pt_fbInput, pt_fbDiag, imagewidth, height, p1, p2, scan_coarse))
        {
            confidence++;
        }

     }
     //printf("DEBUG WHEEL CONFIDENCE IS %i \n", confidence);
     return confidence;
}
              
/********************************************/
fiducial * process_file(RGBAType *pt_fbInput, RGBAType *pt_fbDiag, int width, int height, 
                        int coarse_threshold, int max_threshold, int fine_threshold, int *candidate_count )
{
    int x = 0;
    int y = 0;

    int x_threshold     = coarse_threshold; //coarse_threshold; //minimun trigger size - 15
    int y_threshold     = coarse_threshold; //coarse_threshold;
        
    int x_thresh_max    = max_threshold;    //max_threshold; //maximum trigger size  - 25
    int y_thresh_max    = max_threshold;    //max_threshold;

    int second_pass     = fine_threshold;   //square size of x and y match - 1
    int margins         = fine_threshold*2; // not so clever "work around" for segfault off edges

    float verif_dist    =  (float)(coarse_threshold+max_threshold); //(float)(coarse_threshold*2); //distance to walk N E S W  //

    int hit_count_x     = 0;
    int hit_first_x     = 0;
    int hit_last_x      = 0;

    int hit_count_y     = 0;
    int hit_first_y     = 0;
    int hit_last_y      = 0;

    int last_isblackX   = 0;
    int last_isblackY   = 0;

    int last_pix_xy[2]  = {0,0};
    int midpoint        = 0;
    int ch_id           = 0; //count the hits 
    
    int confidence      = 0;
    
    RGBAType* ptnxt     = 0;
    RGBAType* ptrDiag   = 0;
    RGBAType* ptr       = 0;

    //init cache variable         
    int i, j;
    for (i = 0; i < cachesize; i++)
    {
        for (j = 0; j < cachesize; j++)
        {
            scanCache[i][j] = 0;
        }
    }
    /**************************/
      
    //scan the height 
    for (x=0; x<width-margins; x++) 
    {        
        for (y=0; y<height-margins; y++) 
        {
            ptr      = &(pt_fbInput[(y*width)+x]);
            ptrDiag  = &(pt_fbDiag[(y*width)+x]); 
            //one pixel down on y axis 
            if(y<height && x< width)
            {
                ptnxt  = &( pt_fbInput[ ((y+1)*width) + (x+1)] );
            }else{
                ptnxt   = 0;                 
            }
       
            /********************/

            //check if pixel is "dark" 
            if ( scanner_darkly(ptr) )
            {
                          
                //store the first x hit in sequence
                if ( hit_count_y == 0)
                {
                    hit_first_y = y;
                        
                    if(diagnose_firsthit){
                        ptrDiag->r= 255;   //first hit y
                    }
                } 

                if ( hit_count_y >= y_threshold)
                {
                            //store the hits that exceed threshold X                         
                            if (hit_count_y <=y_thresh_max)
                            {
                                if(y<height-margins && x < width-margins)
                                { 
                                    //check if next pixel is NOT "dark" 
                                    if ( scanner_darkly(ptnxt)==0 )
                                    {                    
                                        hit_last_y = y+1;

                                        midpoint =  hit_first_y + (hit_last_y - hit_first_y)/2;
 
                                        RGBAType* ptMid_v  = &(pt_fbDiag[(midpoint*width)+x]);  

                                        //cache the "hits" in a 2d array
                                        scanCache[x][midpoint] = 23;//the value 23 means nothing - anything but zero will work  
                                        scan_cache_count++;

                                        //debug middle hit y 
                                        if (diagnose_midpoints){ 
                                             ptMid_v->r = 255;ptMid_v->g  = 255; ptMid_v->b = 0;ptMid_v->a = 255;//middle hit y                                     
                                        }

                                        //debug last hit y   
                                        if (diagnose_lasthit){                            
                                            ptrDiag->r = 255; ptrDiag->g  = 0; ptrDiag->b = 0;ptrDiag->a = 255;//last hit y
                                        }

                                    }//not dark 
                                }//dont segfault me Bro!
                            
                          }// <max threshold Y
                      }// > min threshold Y
                 hit_count_y++;
                }
                else
                {
                     hit_count_y = 0;
                     hit_first_y = 0;
                     hit_last_y  = 0;
                }

            }
        }//height scan 

        //scan the width 
        for (y=0; y<height-margins; y++) {
            for (x=0; x<width-margins; x++) {
                
                ptr      = &( pt_fbInput[(y*width)+x] );
                ptrDiag  = &(pt_fbDiag[(y*width)+x]);                 
                ptnxt    = &(pt_fbInput[(y*width)+(x+1)] );

                //check if pixel is "dark" 
                if ( scanner_darkly(ptr) ){
                   //printf(" %d", hit_count_x );
                   last_pix_xy[0] = x;last_pix_xy[1] = y;
                   
                   //store the first x hit in sequence
                   if ( hit_count_x == 0){
                       hit_first_x = x; 

                       //debug first hit x
                       if(diagnose_firsthit){
                           ptrDiag->g = 255;ptrDiag->a = 255;//first hit x
                       }
                   } 

                   //store the hits that exceed threshold X 
                   if ( hit_count_x >= x_threshold){

                      if ( hit_count_x <= x_thresh_max){
                       //check if next pixel is NOT "dark" 
                       if(x<=width ){ 
                         if ( scanner_darkly(ptnxt)==0 ){                    
                            hit_last_x = x+1;

                            midpoint = hit_first_x + (hit_last_x - hit_first_x)/2;
                            RGBAType* ptMid_h = &(pt_fbDiag[(y*width)+midpoint]);

                            if (debug_pointcache){
                                printf("#cache midpoint %d %d,%d \n", hit_first_x, midpoint, hit_last_x );
                            }

                            //THIS IS TO DEBUG THE MIDDLE X 
                            if (diagnose_midpoints){ 
                                ptMid_h->r = 128;ptMid_h->g = 128; //middle pixel x
                            }
                            
                            //show the squares that sample each other
                            if (diagnose_battleship){
                                int color[3]={0};
                                color[1]=255;color[2]=255;
                                
                                int pt1[2] = {0};
                                pt1[0]=midpoint-second_pass; 
                                pt1[1]=y-second_pass;

                                int pt2[2] = {0};
                                pt2[0]=midpoint+second_pass;
                                pt2[1]=y+second_pass;

                                draw_square(pt_fbDiag, width, pt1, pt2, color);
                            }

                            for(int bs = midpoint-second_pass; bs < midpoint+second_pass; bs++ ){
                                for(int bt = y-second_pass; bt < y+second_pass; bt++ ){
                                    ptrScCache = &scanCache[bs][bt];
                                    confidence = 0;
                                    if (ptrScCache[0] != 0 ){
                                        int p1[2] = {0,0};int p2[2] ={0,0};
                                        /************/
                                        // verification stages - check image pixels for a fiducial like pattern and increase "confidence" if it passes checks 

                                        int pt1[2]= {midpoint, y};

                                        //optimization - only verify if it exists coarse_threshold distance from a currently cached point 
                                        if ( check_cache_distance( pt1, cache_hits, ch_id , max_threshold )  )
                                        {
                                            if (disable_wheelverif==false){
                                                confidence = verify_wheel(pt_fbInput, pt_fbDiag, width, height, spokes, verif_dist, midpoint, y, coarse_threshold);
                                            }
                                            if (disable_ringsverif==false){
                                                confidence = confidence + verify_rings(pt_fbInput, pt_fbDiag, width, height, verif_dist, midpoint, y, coarse_threshold, max_threshold);
                                            }
                                        
                                        }//cache distance check 
                                        /************/

                                        //THIS IS TO DEBUG A POSSIBLE FIDUCIAL ANCHOR
                                        if (diagnose_anchors){
                                          int color[3] = {255, 128, 0};
                                          draw_fill_square(pt_fbDiag, width, midpoint, y, 2, color);
                                          
                                          int color2[3] = {230, 190, 50};
                                          draw_point(pt_fbDiag, width, midpoint, y, color2);
                                        }

                                        /************/
                                        if (check_cache_distance( pt1, cache_hits, ch_id , max_threshold ) ) 
                                        {
                                            if (*candidate_count < (sizeof(fiducials) / sizeof(fiducial)))
                                            {
                                                fiducial this_candidate;                                                        
                                                this_candidate.confidence = confidence;
                                                this_candidate.x = midpoint;
                                                this_candidate.y = y;
                                                this_candidate.id = *candidate_count;  
  
                                                fiducials[*candidate_count] = this_candidate; 
                                                *candidate_count = *candidate_count + 1;
                                            } else {
                                                goto enough_fiducials; 
                                            }
                                            if (ch_id < (sizeof(cache_hits) / sizeof(pix_coord)))
                                            {
                                                //cache the hits
                                                cache_hits[ch_id].x = midpoint;
                                                cache_hits[ch_id].y = y;                                                  
                                                ch_id++; //iterate hit count
                                            } 
                                        }//cache distance check 

                                    }//if we hit a "battleship" a match on X and Y in the cache 
                                }//y battleship square
                            }//x battleship square
                            //debug last hit x
                            if (diagnose_lasthit)
                            {
                                ptrDiag->r = 0;ptrDiag->g = 255;ptrDiag->b = 0; //last hit
                            }
                         }//not dark
                       }//dont segfault me Bro!
                      }//< max thresh x
                   }//> min thresh x
                   hit_count_x++;
                }//if dark
                else{
                   hit_count_x = 0;
                   hit_first_x = 0;
                   hit_last_x  = 0;
                }//reset and keep going
            }//width scan x
        }//width scan y
        
enough_fiducials:
        confidence = 0;
        // Find and print the the three most likely fiducials -- those with the higest confidence.
        // qsort(...) sorts our list of candidates according to the the fiducial's confidence member
        // and we print out no more than the first three.
        //printf("Found %i candidate fiducials.\n", candidate_count);
        qsort ((void *) &fiducials, *candidate_count, sizeof(fiducial), (compfn)fiducial_compare);

        //verify the angles of the fiducials 
        //kind of thrown together - but it does work as far as I can tell
        if (disable_angleverif == false){
            check_fiducial_angles( pt_fbInput, pt_fbDiag, width, &fiducials[0], *candidate_count );
        }
        
        // check_fiducial_angles may have changed the fiducials array
        qsort ((void *) &fiducials, *candidate_count, sizeof(fiducial), (compfn)fiducial_compare);
        
        //optional feature to write out a dump of scanned fiducials               
        if (dump_cache_data){
            char filename[] = "cachedump.txt";
            dump_cache(filename, width, height, coarse_threshold, max_threshold, fine_threshold );
        }
        
        for (int iterator=0; iterator<*candidate_count; iterator++)
        {
            if (iterator > 2){
                break;
            }

            if(diagnose_final_fid){
                int color[3] = {255,0,255};
                draw_fill_square(pt_fbDiag, width, fiducials[iterator].x, fiducials[iterator].y, 4, color);
            }
        }

        // Debug fiducial candidate confidence
        if ( debug_confidence)
        {
            for (int i=0; i<*candidate_count; i++){
                printf("\nFound fiducial: id = %i confidence = %i at x %i y %i.",  fiducials[i].id, fiducials[i].confidence, fiducials[i].x, fiducials[i].y);
                if (i == (*candidate_count - 1)){
                    printf("\n");
                }
            }
        }
    return fiducials;
}//end process

