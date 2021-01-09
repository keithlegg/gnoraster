#ifndef SCANCORE_H    
#define SCANCORE_H


/* use these to optimize speed/memory usage */ 
#define cachesize 10000               // coresponds to image X/Y res - we should never have an image 10K square.... hopefully
#define confidence_checksize 200      // If we've found more than 200 fiducial candidates something is wrong. 
#define anchor_checksize 200          // number of potential hits before it stops looking  

//use these to disable the verification stages 
#define disable_wheelverif false       // turn off wheel verification stage for debugging 
#define disable_ringsverif false       // turn off rings verification stage for debugging 
#define disable_angleverif false       // turn off angle verification stage for debugging 

/* text output debugging */
#define debug_pointcache false        // dump the cached points    
#define debug_verify false            // info verify 
#define debug_verify_rings false      // info verify axis aligned
#define debug_confidence false        // print out fiducial confidence info 


typedef int (*compfn)(const void*, const void*); //used in the qsort function 

float check_fiducial_angles( RGBAType *pt_fbInput, RGBAType *pt_fbDiag, int imagewidth, 
	                         fiducial *ptFidlist, int num );

bool average_pixels(int *values, int num);

void read_cache( char *filename );

void log_verify(char* msg);

void dump_cache( char *filename, int width, int height, int coarse, int max, int fine);

fiducial *process_file(RGBAType *pt_fbInput,  RGBAType *pt_fbDiag, int width, int height, 
					    int coarse_threshold, int max_threshold, int fine_threshold, int *candidate_count );

int scan_darkpt (RGBAType *pt_fbInput, RGBAType *pt_fbDiag, pix_coord pt, int width, int height );
int check_cache_distance( int smpl[2], pix_coord out_coords[] , int num, int distance);

int linear_scan(RGBAType *pt_fbInput, RGBAType *pt_fbDiag, int width, int height, int p1[2], int p2[2], int scan_coarse);

int check_polyline_luminance(RGBAType *pt_fbInput, RGBAType *pt_fbDiag, int imagewidth, int imageheight, 
                             pix_coord *ptPoints, int numpts, int scan_coarse );

int verify_rings(RGBAType *pt_fbInput, RGBAType *pt_fbDiag, int imagewidth, int imageheight
                 ,float diameter, int xpos, int ypos, int scan_coarse, int scan_max);

int verify_wheel(RGBAType *pt_fbInput, RGBAType *pt_fbDiag, int imagewidth, int height,
                 int spokes, float diameter, int xpos, int ypos, int scan_coarse);

int verify_old(RGBAType *pt_fbInput, RGBAType *pt_fbDiag, int width, int height,
               float verif_dist, int midpoint, int y, int scan_coarse);

int fiducial_compare(const fiducial *f1, const fiducial *f2);



#endif
