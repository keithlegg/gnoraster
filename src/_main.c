


#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <cstring> //osx wants this for strcmp 
#include <cmath>
#include <unistd.h> //for getcwd 

#include <png.h>

#include "point_op.h"    // vector operations
#include "framebuffer.h" // raster operations
#include "scan_core.h"   // the scanner code 
#include "image_util.h"  // experimental features, etc 

/***************************************************************/

extern bool show_diag_msg;
extern bool do_diagnose;

extern bool diagnose_final_fid;
extern bool diagnose_anchors; 
extern bool diagnose_verify;
extern bool diagnose_confidence;
extern bool diagnose_verif_angles;
extern bool diagnose_firsthit;
extern bool diagnose_lasthit;
extern bool diagnose_midpoints; 
extern bool diagnose_battleship;
extern bool dump_cache_data;
extern bool diagnose_verif_rings;
extern int max_confidence;


#define fine_distance 1 //used to always be one and passed as an arg- upped the value due to improved cache distance sorting 
#define enable_prescaling true //override to disable the "speed up scan" feature

/***************************************************************/

// LIBPNG global variables 
int number_of_passes;

png_byte color_type;
png_byte bit_depth;
png_bytep *row_pointers;
png_structp png_ptr;
png_infop info_ptr;


char version[] = "1.311";


/*
void splitname( char* s, char *out )
{
 
    char* token = strtok(s, ".");
    while (token) {
        //printf("token: %s\n", token);
        token = strtok(NULL, ".");
        if (token)
        {
            strcpy(out, token);
            //out = token;
        }   
        //printf("toke out: %s\n", out);
    }

}
*/

/***************************************************************/
void read_png_file(const char* file_name, int *w, int *h)
{
    char header[8];    // 8 is the maximum size that can be checked
    int x=0;int y =0;
    // open file and test for it being a png  
    FILE *fp = fopen(file_name, "rb");
    if (!fp)
        abort_("[read_png_file] File %s could not be opened for reading", file_name);
    fread(header, 1, 8, fp);
        
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
        abort_("[read_png_file] png_create_read_struct failed");

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        abort_("[read_png_file] png_create_info_struct failed");

    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[read_png_file] Error during init_io");

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    *w = png_get_image_width(png_ptr, info_ptr);
    *h = png_get_image_height(png_ptr, info_ptr);

    color_type = png_get_color_type(png_ptr, info_ptr);
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[read_png_file] Error during read_image");

    row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * *h);
    for (y=0; y< *h; y++)
        row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));

    png_read_image(png_ptr, row_pointers);

    fclose(fp);
}


/***************************************************************/
/* 
  DEBUG - the auto setting of globals "width" and "height" need tweaking.

  This function works, but "read_png_file" and "write_png_file" sets width and height automagically.
  If you use another function that depends on width and height you must call them first.

*/ 

RGBAType *read_png_create_buffer32( const char* file_name, int *w, int *h)
{
    int x=0;int y =0;

    //read_png_file(row_pointers, file_name, w, h, png_ptr, info_ptr, bit_depth, color_type, number_of_passes);
    read_png_file(file_name, w, h);

    RGBAType *pixels = createBuffer32(*w, *h);

    int pixItr = 0;

    for (int y = 0; y < *h; y++)
    {     
        png_byte* row  = row_pointers[y];

         for (int x = 0; x < *w; x++)
        {  
            pixItr = (y * *w) + x;  
            png_byte* ptr = &(row[x*4]);

            pixels[pixItr].r = ptr[0];       
            pixels[pixItr].g = ptr[1];
            pixels[pixItr].b = ptr[2];
            pixels[pixItr].a = ptr[3];           
        }
    }
    return pixels;
}

/***************************************************************/


/* 
  show output of the guassian blur 
  
  // void show_guassian(char* imagepath, char* output, int blur_rad, int clamp_darkval){

*/
void show_guassian(char* imagepath, int coarse_arg, int max_arg  )
{
    int w = 0;int h = 0;
    char* output = "diagnostic.png";
    int clamp_darkval =  160;
 
    RGBAType *pxl_input = read_png_create_buffer32( imagepath, &w, &h); //main image
    RGBAType *pxl_gauss = read_png_create_buffer32( imagepath, &w, &h); //work buffer for guassian
 
    int longedge              = 0;
    int enable_multipass_blur = 1; //1 or 0 - bool represented as int
    int guassian_passes       = 0; //number of guassian oversamples - can be very slow
 
    if (w<h){
        longedge = h;
    }else{
     longedge = w;
    }
  
    //determine optimum scan settings based on image resolution
    float* ptr = 0;
    ptr =  autoscan_settings(w,h);
    guassian_passes       = (int)*(ptr+3); //number of guassian oversamples - can be very slow
    
    printf("gaussian settings : passes %i \n", guassian_passes );


    //experiment - 1 time vs multipass
    //single pass blurring
    if (!enable_multipass_blur)
    {
        printf("# single pass blurring enabled\n");
        gaussBlur ( pxl_input, pxl_gauss , w, h, guassian_passes, true , clamp_darkval);
        threshold ( pxl_gauss, w, h, clamp_darkval) ;
    }
     
    //multipass blurring works better!!
    if (enable_multipass_blur)
    {
        printf("# multi pass blurring enabled\n"); 
        int xx = 0;
        for (xx=1;xx<=guassian_passes;xx++)
        {
             printf("gauss pass %i \n", xx );
             gaussBlur( pxl_input, pxl_gauss , w, h, xx, true , 150);   
             pxl_input = copyBuffer32(pxl_gauss, w, h); 
        }
        threshold ( pxl_gauss, w, h, clamp_darkval) ;
    }
    
    char title[] = "generated_by_scanfids";  
    writePng32(pxl_gauss, output, w, h, title);
 
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL){
        printf("\nSaving image %s/%s\n", cwd, output);     
    }
 
    //memory housekeeping
    cleanup_heap(row_pointers, h);   
    free(pxl_input);  
    free(pxl_gauss);  
}

/***************************************************************/

void show_threshold(char* input, char* output, int blur_rad, int clamp_darkval)
{
   /* show output of the threshold operation 

       This is basically a "full on" contrast function. It considers a pixel to be black or white only based on 
       a luminance threshold. 

   */


   int w = 0;int h = 0;

   RGBAType *pxl_input = read_png_create_buffer32( input, &w, &h); //main image
   RGBAType *pxl_gauss = read_png_create_buffer32( input, &w, &h); //work buffer for guassian

   //if blur_rad is zero than autoset value 
   if (blur_rad==0){
       blur_rad = calc_blur_radius(w, h);
   }

   //int clamp_darkval = 160; // Too high ruins dark images. Too low ruins light images. 160 is just right.

   //gaussBlur( pxl_input, pxl_gauss , w, h, blur_rad, false , clamp_darkval);
   threshold ( pxl_gauss, w, h, clamp_darkval) ;

   char title[] = "generated_by_scanfids";  
   writePng32(pxl_gauss, output, w, h, title);

   char cwd[1024];
   if (getcwd(cwd, sizeof(cwd)) != NULL){
       printf("\nSaving image %s/%s\n", cwd, output);     
   }

   //memory housekeeping
   cleanup_heap(row_pointers, h);   
   free(pxl_input);  
   free(pxl_gauss);  
}


/***************************************************************/

void runscan(char* imagepath, int coarse_arg, int max_arg  )
{
    /*
        Improvement ideas:
            arbitrary scaling / precaling 

    */


    /***************************/
    // setup diagnostic output image names 

    char basename[256];
    char* p_basename = strtok(strcpy(basename, imagepath), ".");

    char    gausfname[20] = "";
    char    diagfname[20] = "";
    char    gausext[20] = "_gauss.png";
    char    diagext[20] = "_diag.png";

    strncat(gausfname, p_basename, 20);
    strncat(gausfname, gausext, 20);
    
    strncat(diagfname, p_basename, 20);
    strncat(diagfname, diagext, 20);


    /***************************/

    int w = 0;int h = 0;

    int longedge = 0;

    int do_prescaler          = 1; //1 or 0 - bool represented as int 
    int enable_multipass_blur = 1; //1 or 0 - bool represented as int
    int guassian_passes       = 0; //number of guassian oversamples - can be very slow
    int shrinkage             = 0; //scale factor 1= full size, 2 = half , 3 = quarter
 
    int clamp_darkval         = 160; // Too high ruins dark images. Too low ruins light images. 160 is just right.
    int candidate_count       = 0;


    RGBAType *pxl_input = read_png_create_buffer32( imagepath, &w, &h); //main image
    
    printf("scanning image %s \n", imagepath );

    //seems to crash if it goes higher 
    if (!enable_prescaling)
    {
        if(coarse_arg>=35){coarse_arg=35;}
        if(max_arg>=70)   {max_arg=70;}
    }
    
    //determine optimum scan settings based on image resolution
    float* ptr =  autoscan_settings(w,h);
    shrinkage             = *(ptr+2);      // scale factor 1= full size, 2 = half , 3 = quarter
    guassian_passes       = (int)*(ptr+3); // number of guassian oversamples - can be very slow
    do_prescaler          = (int)*(ptr+4); // 1 or 0 - bool represented as int 

    //shrink the image first to speed up the scanning 
    if (do_prescaler && enable_prescaling)
    {
        //printf("\n\n # DEBUG # coarse %i max %i scale_factor %i gausspasses %i do_prescale %i \n", coarse_arg, max_arg, shrinkage, guassian_passes, do_prescaler  );
        printf(" prescaling is active \n");

        pxl_input  = copyBufferEveryOther32(pxl_input, &w, &h, shrinkage);
        coarse_arg = coarse_arg/shrinkage;
        max_arg    = max_arg/shrinkage;
    }

    if (show_diag_msg)
    {
        printf(" --> DEBUG scan settings are:\n");
        printf(" --> coarse %i max %i shrinkage %i gaussian %i  \n", coarse_arg, max_arg, shrinkage, guassian_passes);
    }

    /**********************************/


    RGBAType *pxl_gauss = copyBuffer32(pxl_input, w, h); 
    RGBAType *pxl_diag  = copyBuffer32(pxl_input, w, h);  


    fiducial *fiducials;
 
    //single pass blurring
    if (!enable_multipass_blur)
    {
        printf(" running  single pass gaussian \n");  
        gaussBlur ( pxl_input, pxl_gauss , w, h, 1, true , clamp_darkval);
        threshold ( pxl_gauss, w, h, clamp_darkval) ;

    }

    //multipass blurring works better!!
    if (enable_multipass_blur)
    {
        int xx = 0;
        for (xx=1;xx<=guassian_passes;xx++)
        {
            printf("running gaussian pass %i \n", xx);
            gaussBlur( pxl_input, pxl_gauss , w, h, xx, true , clamp_darkval);   
            pxl_input = copyBuffer32(pxl_gauss, w, h); 
            threshold ( pxl_gauss, w, h, clamp_darkval) ;
        }
    }
    
    
    //normal - pass orignial file as diagnostic 
    //fiducials = process_file( pxl_gauss, pxl_diag, w, h, coarse_arg, max_arg, fine_distance, &candidate_count);

   if (do_diagnose){
       write_buffer_png(pxl_gauss, row_pointers, gausfname, w, h, png_ptr, info_ptr, bit_depth, color_type ) ;
    }

    //pass gaussian as diagnostic 
    fiducials = process_file( pxl_gauss, pxl_gauss, w, h, coarse_arg, max_arg, fine_distance, &candidate_count);

   if (do_diagnose){

       /*
       char cwd[1024];
       if (getcwd(cwd, sizeof(cwd)) != NULL){
           printf("Saving image %s/%s\n\n", cwd, diagfname);     
       }
       */
       write_buffer_png(pxl_gauss, row_pointers, diagfname, w, h, png_ptr, info_ptr, bit_depth, color_type ) ;

   }

   for (int iterator=0; iterator < candidate_count; iterator++)
   {
       if (iterator > 2){break;}//stop after 3 
       if (iterator==0){
           if (do_prescaler){
               printf(  "%i %i" , fiducials[iterator].x*shrinkage, fiducials[iterator].y*shrinkage);
           }else{
               printf(  "%i %i" , fiducials[iterator].x, fiducials[iterator].y);            
           }
       }else{
           if (do_prescaler){
               printf(  ",%i %i" , fiducials[iterator].x*shrinkage, fiducials[iterator].y*shrinkage);
           }else{
               printf(  ",%i %i" , fiducials[iterator].x, fiducials[iterator].y);            
           }
       }
   }

   // printf(  ",this will not be parsed! ");  

   //memory housekeeping
   cleanup_heap(row_pointers, h);   
   free(pxl_input);  
   free(pxl_gauss);  
   free(pxl_diag);    
}


/***************************************************************/
/*
void run_multiscan(char* imagepath, int coarse_arg, int max_arg  ){
     
    // fancier version of run_scan - run it and look at confidence levels, if they are deemed unsatisfactory
    // then run it again with different settings and take the "best" results from either scan   
     

    int w = 0; int h = 0;
    int nw = 0; int nh = 0; //width and height, will be normalized so width is > height
    
    bool SHOW_DEBUG_MSGS = false;
    // the ratio of max_confidence/current_confidence at which we say that these fiducials are good enough.
    float CONFIDENCE_THRESHOLD = 1.3;

    int longedge = 0;

    int do_prescaler          = 1; //1 or 0 - bool represented as int 
    int enable_multipass_blur = 1; //1 or 0 - bool represented as int
    int guassian_passes       = 0; //number of guassian oversamples - can be very slow
    float shrinkage; //scale factor 1=full size, 2=half, 3=quarter...calculated automatically below
    
    // Slots for things find on each pass through the scan
    // RGBAType* diagnostic_bufs[10];     // Hold low-res copies of diagnostic bu
    // int* candidate_counts[10] = { 0 }; // How many fiducials for each pass
    // int* avg_confidences[10] = { 0 };  // Avg confidence of best fiducials on each pass
    
    // Pixel threshold that we might want to check; the most comonly useful are at 
    // the start since we short-circuit as soon as we find high quality fiducials.
    int thresholds[5] = {135, 165, 185, 200, 210};
    
    //seems to crash if it goes higher 
    if (!enable_prescaling)
    {
        if(coarse_arg>=35){ coarse_arg=35; }
        if(max_arg>=70){ max_arg=70; }
    }
    
    RGBAType *pxl_input  = read_png_create_buffer32(imagepath , &w, &h); //main image
    
    //determine optimum scan settings based on image resolution
    float* ptr = 0;
    ptr      =  autoscan_settings(w,h);
    
    shrinkage       = *(ptr+2); //scale factor 1= full size, 2 = half , 3 = quarter (floating point values allowed)
    guassian_passes = (int)*(ptr+3); //number of guassian oversamples - can be very slow
    do_prescaler    = (int)*(ptr+4); //1 or 0 - bool represented as int 
        
    if (w < h){
        nw = h;
        nh = w;
    } else {
        nw = w;
        nh = h;
    }
    
    //shrink the image first to speed up the scanning
    if (do_prescaler && enable_prescaling)
    {
        //printf("\n\n # DEBUG # coarse %i max %i scale_factor %i gausspasses %i do_prescale %i \n", coarse_arg, max_arg, shrinkage, guassian_passes, do_prescaler  );
        pxl_input  = copyBufferEveryOther32(pxl_input, &w, &h, shrinkage);
        //printf("--> w=%i, h=%i\n\n", w, h);
        coarse_arg = coarse_arg/shrinkage;
        max_arg    = max_arg/shrinkage;
    }
    
    RGBAType *pxl_gauss         = copyBuffer32(pxl_input, w, h); 
    RGBAType *best_pxl_diag     = copyBuffer32(pxl_gauss, w, h); 
    RGBAType *current_pxl_diag  = copyBuffer32(pxl_gauss, w, h); 
    
    // Values discovered by trial and error with a large number of scans.
    // Too high ruins dark images. Too low ruins light images. 160 is first pass.
    //int clamp_darkval     = 165; 
    // Too high ruins dark images. Too low ruins light images. 128 is second pass.    
    //int alt_clamp_darkval = 135;
    int best_candidate_count    = 0;
    int current_candidate_count = 0;
    
    fiducial best_fiducials[confidence_checksize];
    fiducial *current_fiducials;
    
    float best_avg_confidence    = 0; // This is the confidence value we'll end up using.
    float current_avg_confidence = 0;
    
    int best_pass = 0; // The pass upon which the maximum confidence was observed
    
    //single pass blurring - faster but low quality (larger amount of pixel noise) 
    if (!enable_multipass_blur)
    {
        int blur_rad = calc_blur_radius(w, h);
        gaussBlur ( pxl_input, pxl_gauss , w, h, blur_rad, false , 0);
    }

    //multipass blurring -  slower but highly effective for eliminating pixel noise 
    if (enable_multipass_blur)
    {
        int xx = 0;
        for (xx=1;xx<=guassian_passes;xx++){
            gaussBlur( pxl_input, pxl_gauss , w, h, xx, false , 0); //increase blur radius with each iteration    
            pxl_input = copyBuffer32(pxl_gauss, w, h); //feed the output back into the input and run it again 
        }
    }
    
    if (SHOW_DEBUG_MSGS){
        printf(" \nMax confidence is %i\n", max_confidence);
    }
    
    for (int i=0; i < (sizeof(thresholds) / sizeof(int)); i++)
    {   
        current_avg_confidence = 0;
        current_candidate_count = 0;
        
        RGBAType *pixels = copyBuffer32(pxl_gauss, w, h); 
        threshold(pixels, w, h, thresholds[i]);
        current_fiducials = process_file(pixels, current_pxl_diag, w, h, coarse_arg, max_arg, fine_distance, &current_candidate_count);
    
        // Make a copy of the first set of fiducial candidates we found using the initial clamp_darkval setting
        // so that if we decide to scan a second time, but get worse results, we can fall back to these.
        //for (int iterator=0; iterator < current_candidate_count; iterator++){altfids[iterator] = fiducials[iterator];}
    
        // Calculate a measure of confidence for the fiducials we found
        for (int iterator=0; iterator < current_candidate_count; iterator++)
        {
            if (iterator > 2){break;} //stop after 3 
            //printf("DEBUG fiducial %i confidence %i \n" , iterator, fiducials[iterator].confidence );
            current_avg_confidence = current_avg_confidence + current_fiducials[iterator].confidence;
        }
        if (current_candidate_count > 2){
            current_avg_confidence = current_avg_confidence / (float) (min(3, (int) current_candidate_count));
        } else {
            current_avg_confidence = 0;
        }


        if (SHOW_DEBUG_MSGS){
            printf("  --> Pass #%i current avg confidence = %f, candidate_count = %i\n", i+1, current_avg_confidence, current_candidate_count);
            printf("  --> Prior best pass avg confidence = %i \n\n",  (int)best_avg_confidence); 
        }
    
        // If the values we got with these settings are better than the ones we got previously,
        // update the 'best' values from the current values so that the 'best' are in fact always
        // the best at the end of the loop.
        if (current_avg_confidence > best_avg_confidence)
        {
            best_pass = i;
            best_candidate_count = current_candidate_count;
            best_avg_confidence = current_avg_confidence;
            best_pxl_diag = copyBuffer32(current_pxl_diag, w, h);
            // Copy the current fiducials in to the best fiducials
            for (int iterator=0; iterator < current_candidate_count; iterator++)
            {
                best_fiducials[iterator] = current_fiducials[iterator];
            }
        
        }
    
        // If we have good confidence levels on these fiducials, don't bother looking for better ones.
        if ((current_avg_confidence > max_confidence / CONFIDENCE_THRESHOLD))
        {
            if (SHOW_DEBUG_MSGS){
                printf("Breaking on pass #%i with avg confidence = %f\n", i+1, current_avg_confidence);
            }
            break;
        }
    }
    
    if (SHOW_DEBUG_MSGS){
        printf("Average confidence for final fiducials = %f\n\n", best_avg_confidence);
    }

    for (int iterator=0; iterator < best_candidate_count; iterator++)
    {
        if (iterator > 2){ break; }//stop after 3 
        if (iterator==0){
            if (do_prescaler){
                printf("%i %i" , (int)(best_fiducials[iterator].x * shrinkage), (int)(best_fiducials[iterator].y * shrinkage));
            }else{
                printf("%i %i" , (int)best_fiducials[iterator].x, (int)best_fiducials[iterator].y);            
            }
        }else{
            if (do_prescaler){
                printf(",%i %i" , (int)(best_fiducials[iterator].x*shrinkage), (int)(best_fiducials[iterator].y*shrinkage));
            }else{
                printf(",%i %i" , (int)best_fiducials[iterator].x, (int)best_fiducials[iterator].y);            
            }
        }
    }

    if (do_diagnose){
        char path[] = "diagnostic.png";
        char cwd[1024];
        if ((getcwd(cwd, sizeof(cwd)) != NULL) && SHOW_DEBUG_MSGS){
            printf("\nSaving image %s/%s\n", cwd, path);
        }
        write_buffer_png(best_pxl_diag, row_pointers, path, w, h, png_ptr, info_ptr, bit_depth, color_type);     
   }

   cleanup_heap(row_pointers, h);   
   free(pxl_input);  
   free(pxl_gauss); 
   free(current_pxl_diag); 
   free(best_pxl_diag);    
}

*/

/***************************************************************/


/* this is a testing tool for verify_rings/verify_wheel */ 
void test_verify_point(char *imagepath, int x, int y, float verif_dist, int spokes, int coarse_threshold, int max_threshold)
{
    int c =0;
    int w = 0;int h = 0;
    
    int clamp_darkval = 160; 

    char title[]               = "generated_by_scanfids";    
    char diagnose_image_name[] = "diagnostic.png"; 

    RGBAType *pxl_input = read_png_create_buffer32( imagepath, &w, &h); //main image
    RGBAType *pxl_gauss = read_png_create_buffer32( imagepath, &w, &h); //work buffer for guassian
    RGBAType *pxl_diag  = read_png_create_buffer32( imagepath, &w, &h); //work buffer for guassian

    int blur_rad = calc_blur_radius(w, h);

    int candidate_count = 0;
    fiducial *fiducials;

    gaussBlur ( pxl_input, pxl_gauss , w, h, blur_rad, true , clamp_darkval);
    
    c = verify_wheel(pxl_gauss, pxl_diag, w, h, spokes, verif_dist, x, y, coarse_threshold);
    c = c + verify_rings(pxl_gauss, pxl_diag, w, h, verif_dist, x, y, coarse_threshold, max_threshold);
    
    writePng24(pxl_diag, diagnose_image_name, w, h, title);

    printf("confidence is %i \n", c);

   //memory housekeeping
   cleanup_heap(row_pointers, h);   
   free(pxl_input);  
   free(pxl_gauss); 
   free(pxl_diag); 

}

/***************************************************************/

/* this is a testing tool for check_fiducial_angles */
void test_verify_angles(char *imagepath)
{
    int w = 512;
    int h = 512;
    
    RGBAType *pxl_input  = createBuffer32( w, h); //main image
    RGBType linecol = newRgb(120,128,0);

    fillbuffer32(pxl_input, w, h, &linecol);
    RGBAType *pxl_diag  = copyBuffer32( pxl_input, w, h); //main image

    char title[] = "generated_by_scanfids"; 
    char diagnose_image_name[] = "diagnostic.png"; 
    
    fiducial fiducials[100];
    fiducials[0] = newfid(0,   0  , 0  ,  9);
    fiducials[1] = newfid(1,   10 , 0  ,  9);
    fiducials[2] = newfid(3,   0  , 10 ,  9);
    fiducials[2] = newfid(4,   0  , 10 ,  9);

    //pass in the number of fiducials to check as last arg
    check_fiducial_angles( pxl_input, pxl_diag, w, &fiducials[0], 4 ); 
    
    
    /*
    RGBType c2;
    c2.r=255;
    vector2d v1 = line2vect( 40, 40, 20, 20 );
    draw_vector ( pxl_diag, w, v1, 40, 40, &c2 );
    */

    writePng24(pxl_diag, diagnose_image_name, w, h, title);
    
   //memory housekeeping
   cleanup_heap(row_pointers, h);   
   free(pxl_input);  
   free(pxl_diag); 

}

/***************************************************************/

void parseArgs(int argc, char **argv)
{
    if (argc < 2){
        abort_("Usage: scan_fids <mode> <file in> <coarse> <max> ");
    }
    
    //changed these from global to local
    int w = 0;int h = 0;
    char scanmode[10];
    char diagnose_image_name[] = "diagnostic.png";
    
    /***************************/

    //defualt scan mode
    strcpy(scanmode, "scan");
    if( strcmp(argv[1], scanmode) == 0)
    {
        if (argc !=5){
             abort_("\nUsage: scan_fids scan <file in> <coarse> <max> ");
        }
        int coarse = atoi(argv[3]);   
        int max    = atoi(argv[4]); 
       
        runscan(argv[2], coarse, max );
        //run_multiscan(argv[2], coarse, max);
    }
    
    /***************************/

    strcpy(scanmode, "auto");
    if( strcmp(argv[1], scanmode) == 0)
    {
        if (argc !=3){
             abort_("\nUsage: scan_fids auto <file in> ");
        }

        int* ptr = 0;
        // format is : [width, height, coarse, max , 24/32 , bits per channel]
        ptr = read_png_fileinfo( argv[2], w, h, png_ptr, info_ptr, color_type, bit_depth );

        printf("********************\n" );
        printf("settings          coarse %i max %i \n", *(ptr+2), *(ptr+3) );
        printf("width             %i \n", *(ptr  ) );    
        printf("height            %i \n", *(ptr+1) );  
        printf("image bit depth   %i \n", *(ptr+4) );  
        printf("channel depth     %i \n", *(ptr+5) );  

        do_diagnose            = true;
        diagnose_verify        = true;
        diagnose_confidence    = true;
        diagnose_final_fid     = true;  
        diagnose_verif_angles  = true;        
        diagnose_verif_rings   = true;

        runscan( argv[2], *(ptr+2) , *(ptr+3));
        //run_multiscan(argv[2], *(ptr+2), *(ptr+3));

    }    
    
    /***************************/

    strcpy(scanmode, "anchor");
    if( strcmp(argv[1], scanmode) == 0)
    {
        if (argc !=5){
            abort_("\nUsage: scan_fids anchor <file in> <coarse> <max>  ");
        }
      
        do_diagnose            = true;
        diagnose_verify        = true;
        diagnose_confidence    = true;      
        diagnose_anchors       = true;
        //diagnose_verif_angles  = true;    
        //diagnose_verif_rings   = true;

        runscan(argv[2], atoi(argv[3]), atoi(argv[4]) );
        //run_multiscan(argv[2], atoi(argv[3]), atoi(argv[4]) );
    }    

    /***************************/

    strcpy(scanmode, "final");
    if( strcmp(argv[1], scanmode) == 0)
    {
      if (argc !=5){
        abort_("\nUsage: scan_fids final <file in> <coarse> <max>  ");
      }

      do_diagnose            = true;
      diagnose_verify        = true;
      diagnose_confidence    = true;
      diagnose_final_fid     = true;  
      diagnose_verif_angles  = true;        
      diagnose_verif_rings   = true;

      int coarse = atoi(argv[3]);   
      int max    = atoi(argv[4]); 

      runscan(argv[2], coarse, max );
      //run_multiscan(argv[2], coarse, max );
    }    

    /***************************/

    strcpy(scanmode, "midpoint");
    if( strcmp(argv[1], scanmode) == 0)
    {
      if (argc !=5){
        abort_("\nUsage: scan_fids midpoint <file in> <coarse> <max>   ");
      }

      do_diagnose         = true;
      diagnose_midpoints  = true;  
      
      runscan(argv[2], atoi(argv[3]), atoi(argv[4]) );
      //run_multiscan(argv[2], atoi(argv[3]), atoi(argv[4]) );
    }  

     /***************************/

    strcpy(scanmode, "firsthit");
    if( strcmp(argv[1], scanmode) == 0)
    {
        if (argc !=5){
          abort_("\nUsage: scan_fids firsthit <file in> <coarse> <max> ");
        }
        do_diagnose         = true;
        diagnose_firsthit   = true; 
        
        runscan(argv[2], atoi(argv[3]), atoi(argv[4]) );
        //run_multiscan(argv[2], atoi(argv[3]), atoi(argv[4]) );
    }  
    
    /***************************/

    strcpy(scanmode, "dumpcache");
    if( strcmp(argv[1], scanmode) == 0){;
        //write out a text file of all fiducial data cached 
        dump_cache_data     = true;
        do_diagnose         = true;
        diagnose_verify     = true;
        diagnose_confidence = true;
        diagnose_final_fid  = true; 
        
        runscan(argv[2], atoi(argv[3]), atoi(argv[4]) );
        //run_multiscan(argv[2], atoi(argv[3]), atoi(argv[4]) );      
    }  



    /***************************/

    strcpy(scanmode, "gaussian");
    if( strcmp(argv[1], scanmode) == 0)
    {
        //show_guassian - <IMAGE> <SIZE> <THRESH_VALUE>
        //show_guassian( argv[2], argv[3], atoi(argv[4]), atoi(argv[5]) ); //node style i/o 
        show_guassian( argv[2], atoi(argv[3]), atoi(argv[4]) );
    }

    /***************************/    
    strcpy(scanmode, "threshold");
    if( strcmp(argv[1], scanmode) == 0)
    {
        show_threshold( argv[2], diagnose_image_name, atoi(argv[3]), atoi(argv[4]) );
    }

    /***************************/
    /***************************/
    strcpy(scanmode, "help");
    if( strcmp(argv[1], scanmode) == 0){;
        printf("\nUsage: \n");
        printf("    scanfids <mode> <imagename.png> <coarse> <max> \n");
        printf("      modes are :  scan, final, anchor, midpoint, firsthit, info, auto, dumpcache, testverify \n");
        printf("  \n");
        printf("    mode args: \n");
        printf("      info       : scanfids info IMAGE \n");  
        printf("      auto       : scanfids auto IMAGE \n"); 

        printf("      scan       : scanfids scan IMAGE COARSE MAX\n");
        printf("      final      : scanfids final IMAGE COARSE MAX\n");
        printf("      anchor     : scanfids anchor IMAGE COARSE MAX\n");
        printf("      midpoint   : scanfids midpoint IMAGE COARSE MAX\n");                                                     
        printf("      firsthit   : scanfids firsthit IMAGE COARSE MAX\n");
       
        printf("      gaussian   : scanfids guassian IMAGE COARSE MAX  \n"); 
        printf("      threshold  : scanfids threshold IMAGE BLURAD CLAMPTHRESH  \n"); 
        
        printf("      dumpcache  : scanfids dumpcache IMAGE COARSE MAX\n");
        printf("      testverify : scanfids testverify IMAGE X Y COARSE MAX\n");

        printf("  \n");
    }  

    /***************************/

    // strcpy(scanmode, "about");
    // if( strcmp(argv[1], scanmode) == 0){;
    //     printf("***********************************\n" );
    //     printf("\"scanfids\" version %s \n", version );
    //     printf("modified : December 16th, 2014 \n" );
    //     printf("***********************************\n" );      
    // }  
    

    /***************************/
    strcpy(scanmode, "info");
    if( strcmp(argv[1], scanmode) == 0)
    {
        int* ptr = 0;
        //  format is : [width, height, coarse, max , 24/32 , bits per channel]
        ptr = read_png_fileinfo( argv[2], w, h, png_ptr, info_ptr, color_type, bit_depth );
        printf("********************\n" );
        printf("scan settings:    coarse %i max %i \n", *(ptr+2), *(ptr+3) );
        printf("width             %i \n", *(ptr  ) );    
        printf("height            %i \n", *(ptr+1) );  
        printf("image bit depth   %i \n", *(ptr+4) );  
        printf("channel depth     %i \n", *(ptr+5) );  
        printf("scale factor      %i \n", *(ptr+6) ); 
        printf("gaussian passes   %i \n", *(ptr+7) ); 
        printf("enable prescaling %i \n", *(ptr+8) ); 
        printf("********************\n" );
    }

    /***************************/
    strcpy(scanmode, "scale");
    if( strcmp(argv[1], scanmode) == 0)
    {
    
    }

    /***************************/
    strcpy(scanmode, "rotate");
    if( strcmp(argv[1], scanmode) == 0)
    {
    
    }


    /***************************/


    /***************************/
            
    //output a test image 
    strcpy(scanmode, "test");
    if( strcmp(argv[1], scanmode) == 0)
    {
        if (argc <3){
            abort_("\nUsage: scan_fids <blah> <outfile>");
        }
        // TEST BUILD A NEW FRAMEBUFFER AND DRAW ON IT WITH VECTORS
        RGBType bgcolor;
        bgcolor.r=20;bgcolor.g=45;bgcolor.b=30;
        RGBType linecol;
        linecol.r=0;linecol.g=0;linecol.b=0;
        int locwidth  = 0;
        int locheight = 0;
        //RGBAType *pixels  = createBuffer32(locwidth, locheight);
        //fillbuffer32( pixels, locwidth, locheight, &bgcolor);
        RGBAType *pixels = read_png_create_buffer32( argv[2], &locwidth, &locheight); //main image
        RGBAType *pixels2 = createBuffer32(locwidth, locheight);        
        fillbuffer32( pixels2, locwidth, locheight, &linecol);
        pixels2 = copyBufferEveryOther32(pixels, &locwidth, &locheight, 4);
        //ScaleRect(pixels2, pixels, locwidth, locheight, locwidth/3 , locheight/3 );
        //pixels2 = blitBuffer32( pixels, &locwidth, &locheight, 100, 100,  800, 800 );
        // vector2d v1 = line2vect( 10, 10, 35, 2 );    
        // vector2d v2 = line2vect( 10, 10, -22, 22 );              
        // draw_vector ( pixels, locwidth, v1 , 120, 120, &linecol );
        // draw_vector ( pixels, locwidth, v2 , 120, 120, &linecol );
        //printf( "final out is  %i %i ", locwidth, locheight );
        char title[] = "scanfidstest";
        writePng32(pixels2, diagnose_image_name, locwidth, locheight, title);
        //saveBMP_24bit(copyBuffer_24bit(pixels, locwidth, locheight) , diagnose_image_name, locwidth, locheight );
        //cleanup_heap(row_pointers, h);   
        free(pixels); 
        //test_verify_angles(argv[2]);
        /* 
        // TEST BMP LOAD AND SAVE 
        int locwidth  = 0;
        int locheight = 0;
        RGBType *pixels  = createBuffer24(locwidth, locheight);     
        loadBMP_24bit(pixels, argv[2], &locwidth, &locheight);
        char * outfile = "foo.bmp";
        saveBMP_24bit(pixels, outfile, locwidth, locheight);
        */
        /*
        // TEST BUILD A NEW FRAMEBUFFER AND DRAW ON IT WITH VECTORS
        RGBType bgcolor;
        bgcolor.r=20;bgcolor.g=45;bgcolor.b=30;
        RGBType linecol;
        linecol.r=0;linecol.g=255;linecol.b=0;
        int locwidth  = 400;
        int locheight = 400;
        RGBAType *pixels  = createBuffer_32bit(locwidth, locheight);
        RGBAType *pixels2 = createBuffer_32bit(locwidth, locheight);        
        fill_buffer( pixels, locwidth, locheight, &bgcolor);
        //verify_wheel(pixels, pixels, locwidth, locheight, 16, 25.0, 40, 40);
        //verify_rings(pixels, pixels, locwidth, locheight, 5, 20, 20);
        // vector2d v1 = line2vect( 10, 10, 35, 2 );    
        // vector2d v2 = line2vect( 10, 10, -22, 22 );              
        // draw_vector ( pixels, locwidth, v1 , 120, 120, &linecol );
        // draw_vector ( pixels, locwidth, v2 , 120, 120, &linecol );
        char title[] = "scanfidstest";
        writeImage(pixels, diagnose_image_name, locwidth, locheight, title);
        //saveBMP_24bit(copyBuffer_24bit(pixels, locwidth, locheight) , diagnose_image_name, locwidth, locheight );
        //cleanup_heap(row_pointers, h);   
        free(pixels); 
        */
    }    
    
    /***************************/

    // // ARGS are IMAGEPATH Xcoord Ycoord Coarse
    // strcpy(scanmode, "testverify");
    // if( strcmp(argv[1], scanmode) == 0){
    //     do_diagnose            = true;
    //     diagnose_verify        = true;
    //     diagnose_confidence    = true;      
    //     diagnose_verif_rings   = true;
    //     float verif_dist    =  (float) (atoi(argv[5])*2) ; 
    //     test_verify_point( argv[2], atoi(argv[3]), atoi(argv[4]), verif_dist, 15, atoi(argv[5]), atoi(argv[6]) ) ; 
    // }     
    // //test_verify_point
}


int main(int argc, char **argv)
{

    do_diagnose         = true;
    show_diag_msg       = true;

    parseArgs(argc, argv); //new args with mode added 
    return 1;
}

