/* 
 * part of this function was shamelesly stole from Gerd K., and part stole from A. Fiori.
 * Anyway, its not supposed to stay here that long, `cause I want to use custom fonts...

 *

 */
//#define SCALE_W 4 // scale the font size (width)
//#define SCALE_H 4 // scale the font size (height)

#include "global.h"
#include "font_8x8.h"

//#define DEBUG

/* usage: rgb24_printf(X, Y, SCALE_W, SCALE_H, vd, "hello my %s", "ui"); */
/* if u dont want to scale, use it 0 */


void rgb24_printf (int x, int y, int SCALE_H, int SCALE_W, struct vd_video *vd, const char *fmt, ...)	{
  
	va_list ap;
	char temp[128];
	unsigned char *ptr, *image;
	int i, j, n, len, fnt, a, c, lines=0, offset=0;
	
	
	if (SCALE_H == 0) SCALE_H=1;
	if (SCALE_W == 0) SCALE_W=1;
	
	if ((vd->y - y) < SCALE_H * 8) {
		fprintf (stderr, "rgb24_printf: y out of range\n");
		return;
	}
	
	image=vd->image; // current image


        va_start (ap, fmt);
        memset (temp, 0, sizeof (temp));
        vsnprintf (temp, sizeof (temp), fmt, ap);
        va_end (ap);

        x = x % vd->x;
        y = y % vd->y;
        len = strlen (temp);
	
        for (i = 0; i < 8 ; i++) {
	    //lines=0;
	    offset=(vd->w * (vd->x  * (y + i )) + x);
	    for (c=0; c < SCALE_H ; c++) {
		ptr = image + offset + lines;
	  	#ifdef DEBUG	
		fprintf(stderr,"Image end: %p - current pointer: %p (lines: %d offset: %d c: %d)", vd->image + vd->grab_size, ptr, lines, offset, c);
		#endif
	  	if (image + vd->grab_size < ptr) {
			#ifdef DEBUG
			fprintf(stderr," [out]\n");
			#endif
			return;	
	  	} 
		  #ifdef DEBUG
		   else fprintf(stderr,"\n");
		  #endif
		
		for (j = 0; j < len; j++) {
			fnt = font_database[(temp[j] * 8) + i];
			for (n = 7; n >= 0; n--) {
				if (fnt & (1 << n)) {
					for (a=0; a < vd->w  + SCALE_W  ;a++) {
						//if (ptr[a] < 200) ptr[a] = 255;
						//else ptr[a]=0;
						ptr[a]=255;
					}
				}
				ptr += vd->w  + SCALE_W;
			
			} // n
		} // j
	      lines+=vd->x * vd->w ;
	  } // c
	} // i
}
			
