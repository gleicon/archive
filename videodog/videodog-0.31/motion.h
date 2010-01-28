/* Motion detection package
 *  (c) Gleicon S. Moraes
 */



#include <unistd.h>
#include <time.h>
#include <malloc.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

/* default tolerance reference */
#define _TOLERANCE      10

/* 
 * Basic elements we are dealing:
 * the image itself, its size and depth and a time mark
 */
struct image_motion {
	unsigned char   *image;
	int             x, y, depth;
	int		left, right; // side-o
	time_t          stamp;  
	};


/* methods & functions */

int detect_motion (struct image_motion *_reference_image, struct image_motion *_local_img, char tolerance);
int subtract_motion (struct image_motion *_reference_image, struct image_motion *_local_img, char _tolerance);

int load_pnm(char *wot, struct image_motion *bufo);
int detect_motion_mask (struct image_motion *_reference_image, struct image_motion *_local_img, struct image_motion *mask, char _tolerance);

