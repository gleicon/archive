/* 
 * main transformations and effects functions.
 * for now they only work with RGB frames, not YUV or any other format
 */

/* DISCLAIM-O-RAMA
 * Note that, some functions are ok to apply to the entire image 
 * ( as RGB24 or 8bit), like threshold ( well, the correct way is to 
 * handle each component as independent image, but this is only a demo), 
 * but some functions should know if the image depth is 8 or 24 bits , 
 * an others should convert even an rgb24 image to a grayscale or bw image.
 * I got these concepts from the net, never had a formal education on image 
 * processing, mainly from the improv source code 
 * (http://www.ee.uwa.edu.au/~braunl/improv). 
 * From now some effects are only real for 8 bits images.
 */

#include "global.h"



/* Gamma correction
 * this transformation has two functions: 
 * 1- the lookup table construction
 * 2- the transformation itself. as it uses some calculus, 
 	I did the lookup table to improve the speed when applying 
	to each frame. The lookup table is nothing more than a matrix 
	with the correct gamma values for the given factor.
 */

/* 
should receive the gamma factor, and a pointer to the table (a 256 u_char matrix) */




void gamma_lookup( float factor, unsigned char *lookup_table) {
	unsigned int a;

	for (a=0; a<256; a++)
		lookup_table[a]=(unsigned char)(pow((double)a/255.0f, factor) *255.0f);
}


/* should receive the vd struct and the lookuptable */

void gamma_apply(struct vd_video *vd, char *lookuptable) {
	unsigned long int a=0;

	for (a=0; a < vd->grab_size; a++)
		*(vd->image  + a) = lookuptable[(*(vd->image + a))];

}

/* simple rough threshold function, working in all elements */

void threshold(struct vd_video *vd, unsigned char limit) {
	unsigned long int a=0, b=0;

	for (a=0; a < vd->grab_size; a=a+vd->w) {
		
		if (*(vd->image  + a) > limit) {
			for (b=0; b< vd->w; b++) *(vd->image +a +b)=255;
		} else {
			for (b=0; b< vd->w; b++) *(vd->image +a +b)=0;
		}
	}

}


void sobel(struct vd_video *vd) {
	int i;
        int grad;
	int deltaX, deltaY;
	int width=vd->x;
	int height=vd->y;
	
	unsigned char *imageIn = vd->grab_data, *imageOut = NULL;
	imageOut=malloc(vd->grab_size * sizeof(unsigned char));
	if (!imageOut) {
		fprintf(stderr,"imageOut alloc err\n");
		return;
		}
	
	for (i = width; i < (height-1)*width*vd->w; i++) {
	        deltaX = 2*imageIn[i+1] + imageIn[i-width+1] + imageIn[i+width+1] - 2*imageIn[i-1] - imageIn[i-width-1] - imageIn[i+width-1];

		deltaY = imageIn[i-width-1] + 2*imageIn[i-width] + imageIn[i-width+1] - imageIn[i+width-1] - 2*imageIn[i+width] - imageIn[i+width+1];
                grad = (abs(deltaX) + abs(deltaY)) / 3;
                
		if (grad > 255) grad = 255;
                
		imageOut[i] = (unsigned char) grad;
	}

	memcpy(vd->grab_data, imageOut, vd->grab_size);
	free(imageOut);
}


void laplace(struct vd_video *vd) {
    	int i;
    	int delta;
	int width=vd->x;
	int height=vd->y;
	
	unsigned char *imageIn = vd->grab_data, *imageOut =NULL;
	imageOut=malloc(vd->grab_size * sizeof(unsigned char));
	if (!imageOut) {
		fprintf(stderr,"imageOut alloc err\n");
		return;
		}
	
	

    	for (i = width; i < (height-1)*width; i++) {
		delta  = abs(4 * imageIn[i] - imageIn[i-1] - imageIn[i+1] - imageIn[i-width] - imageIn[i+width]);
        	if (delta > 255) imageOut[i] = 255;
		else  imageOut[i] = (unsigned char)delta;
    	}
	memcpy(vd->grab_data, imageOut, vd->grab_size);
	free(imageOut);
}


void noise(struct vd_video *vd, double qtd) {
    	int i;
	int width=vd->x;
	int height=vd->y;
	int imageSize= width * height;
	int numberOfPixels= (int) (imageSize * qtd);
	
	unsigned char *imageIn = vd->grab_data, *imageOut =NULL;
	
	imageOut=malloc(vd->grab_size * sizeof(unsigned char));
	
	
	if (!imageOut) {
		fprintf(stderr,"imageOut alloc err\n");
		return;
		}
	
	memcpy(imageOut, imageIn, vd->grab_size);
	
	for (i=0; i < numberOfPixels ; i+=2) {
		imageOut[rand() % imageSize]= 255;
		imageOut[rand() % imageSize]= 0;
	}
		
	

	memcpy(vd->grab_data, imageOut, vd->grab_size);
	free(imageOut);
}
																					

void negative(struct vd_video *vd) {
    	int i;
	
	unsigned char *imageIn = vd->grab_data, *imageOut =NULL;
	
	imageOut=malloc(vd->grab_size * sizeof(unsigned char));
	
	
	if (!imageOut) {
		fprintf(stderr,"imageOut alloc err\n");
		return;
		}
	
	memcpy(imageOut, imageIn, vd->grab_size);
	
	for (i=0; i < vd->grab_size ; i++) {
		imageOut[i]= 255-imageIn[i];
	}
		
	

	memcpy(vd->grab_data, imageOut, vd->grab_size);
	free(imageOut);
}
																					

void mirror(struct vd_video *vd) {
	int r, c;
	int width=vd->x;
	
	unsigned char *imageIn = vd->grab_data, *imageOut = NULL;
	imageOut=malloc(vd->grab_size * sizeof(unsigned char));
	if (!imageOut) {
		fprintf(stderr,"imageOut alloc err\n");
		return;
		}
	for (r=0; r < vd->grab_size; r+=width)
		for (c=0; c < width; c++) {
			imageOut[c + r]=imageIn[(width - c) + r];
		}
	

	memcpy(vd->grab_data, imageOut, vd->grab_size);
	free(imageOut);
}


void lowpass(struct vd_video *vd) {
	int r, c;
	int width=vd->x;
	
	unsigned char *imageIn = vd->grab_data, *imageOut = NULL;
	imageOut=malloc(vd->grab_size * sizeof(unsigned char));
	if (!imageOut) {
		fprintf(stderr,"imageOut alloc err\n");
		return;
		}
	for (r=0; r < vd->grab_size; r+=width)
		for (c=0; c < width; c++) {
			imageOut[c + r]= ( imageIn[r-1 + c-1] + imageIn[r -1 + c] + imageIn[r - 1 + c + 1] 
					 + imageIn[r + c -1] + imageIn[r + c] + imageIn[r + c +1] +
					   imageIn[r + 1 + c - 1] + imageIn[ r + 1 + c] + imageIn[r + 1 + c + 1]) /9;
					   
		}
	

	memcpy(vd->grab_data, imageOut, vd->grab_size);
	free(imageOut);
}


void highpass(struct vd_video *vd) {
	int r, c;
	int width=vd->x;
	
	unsigned char *imageIn = vd->grab_data, *imageOut = NULL;
	imageOut=malloc(vd->grab_size * sizeof(unsigned char));
	if (!imageOut) {
		fprintf(stderr,"imageOut alloc err\n");
		return;
		}
	for (r=0; r < vd->grab_size; r+=width)
		for (c=0; c < width; c++) {
			imageOut[c + r]= ( - imageIn[r-1 + c-1] - imageIn[r -1 + c] - imageIn[r - 1 + c + 1] 
			                   - imageIn[r + c -1] + 8 *( imageIn[r + c]) - imageIn[r + c +1]  
					   - imageIn[r + 1 + c - 1] - imageIn[ r + 1 + c] - imageIn[r + 1 + c + 1]) /9;
					   
		}
	

	memcpy(vd->grab_data, imageOut, vd->grab_size);
	free(imageOut);
}


/* inspiration taken from motion ! */

void mark_areas(struct vd_video *vd) { 
        int height=vd->y, width=vd->x;
	int x, y, maxx=0, maxy=0, minx=width, miny=height, a=0;	
	unsigned char *imageIn = vd->image;
	unsigned int oi=0;

	/* Locate the vertex */
	
	for (x=0; x < width; x++) {
		for (y=0; y < height; y ++){
			if( vd->w == 1) {
				oi=(imageIn[(x+y*width)*vd->w+0]);
			} else {
				oi=(imageIn[(x+y*width)*vd->w+0]+ imageIn[(x+y*width)*vd->w+1] + imageIn[(x+y*width)*vd->w+2]);
			}
			if (oi) {
				if (x < minx) minx=x;
				if (x > maxx) maxx=x;
				if (y < miny) miny=y;
                                if (y > maxy) maxy=y;
			}
		}
	}
#ifdef DEBUG
	fprintf(stderr, "mark_areas: minx: %d miny: %d maxx: %d maxy: %d\n", minx, miny, maxx, maxy);
#endif

	/* draw */
	for (x=minx; x<=maxx; x++) {
		for (a=0; a< vd->w; a++)
			imageIn[(x+width*miny)*vd->w+a]=255;
		}
								
	for (x=minx; x<=maxx; x++) {
		for (a=0; a< vd->w; a++)
               		imageIn[(x+width*maxy)*vd->w+a]=255;
	       }
	
        for (y=miny; y<=maxy; y++) {
		for (a=0; a< vd->w; a++)
			imageIn[(minx+y*width)*vd->w+a]=255;
		}
	for (y=miny; y<=maxy; y++) {
		for (a=0; a< vd->w; a++)
	        	imageIn[(maxx+y*width)*vd->w+a]=255;
	        }

}

/* channel threshold (for RGB24 images)
 * it walks by each pixel, and, zeroes the channel w/ minor value
 */
 
void channel_threshold(struct vd_video *vd) {
	unsigned char *a;
//	if (vd->w < 3) return; /* return quietly */
	a=vd->grab_data;
	while ((unsigned int)a < (unsigned int) vd->grab_data + (unsigned int) vd->grab_size) {
		
		if (*a > *(a+1)) {
			if (*a > *(a+2)) *(a+1) = *(a+2) = 0;
		} else if (*(a+1) > *(a+2)) {
			if (*(a+1) > *a) *a = *(a+2) = 0;
		} else if (*(a+2) > *a) {
			if (*(a+2) > *(a+1)) *(a+1) = *a = 0;
		} else {
			*(a) = *(a+1) = *(a+2)=0;
		}


		a+=vd->w;
	}

}

 
void predator(struct vd_video *vd) {
	unsigned char *a;
//	if (vd->w < 3) return; /* return quietly */
	a=vd->grab_data;
	while ((unsigned int)a < (unsigned int) vd->grab_data + (unsigned int) vd->grab_size) {
		
		if (*a > *(a+1)) {
			if (*a > *(a+2)) *(a+1) = *(a+2) = 0;
			*a=255;
		} else if (*(a+1) > *(a+2)) {
			if (*(a+1) > *a) *a = *(a+2) = 0;
			*(a+1)=255;
		} else if (*(a+2) > *a) {
			if (*(a+2) > *(a+1)) *(a+1) = *a = 0;
			*(a+2)=255;
		} else {
			*(a) = *(a+1) = *(a+2)=255;
		}


		a+=vd->w;
	}

}


void posterize(struct vd_video *vd, unsigned char factor) {
	unsigned char *a;
	unsigned int b;
//	if (vd->w < 3) return; /* return quietly */
	a=vd->grab_data;
	while ((unsigned int)a < (unsigned int) vd->grab_data + (unsigned int) vd->grab_size) {
		for (b=0; b< vd->w; b++) *(a+b) = *(a+b) - (*(a+b) % (256/factor)); //if (*(a+b) > 100 && *(a+b) < 150) *(a+b)=255;
		// else *(a+b)=0;


		a+=vd->w;
	}

}




void median_filter(struct vd_video *vd) {
    	int x, y,  sub_pixel, depth, pos;
    	unsigned int delta;
	int width=vd->x;
	int height=vd->y;
	
	unsigned char *imageIn = vd->image, *imageOut =NULL;
	imageOut=malloc(vd->grab_size * sizeof(unsigned char));
	if (!imageOut) {
		perror("median_filter: imageOut malloc");
		return;
		}
	
	
	depth=vd->w;
		/* kernel: median
		 *  X1 X2 X3
		 *  X4 A X6
		 *  X7 X8 X9
		 * A = ( X1 + X2 + X3 + X4 + X6 + X7 + X8 + X9)/8
		 */
		
		/* (123) (123) (123)
		 * (123) (XXX) (123)
		 * (123) (123) (123)
		 */
		 
    	//for (i = width; i < ((height-1)*width); i=i+vd->w) {
    	for (y = 1;  y < height ; y++) {
    		for (x=1; x<width; x++) {
			for (sub_pixel=0; sub_pixel<vd->w; sub_pixel++){
				pos=(x*y*depth)+sub_pixel;
				delta= ( 
					
					imageIn[pos - (depth *width)-depth] + imageIn[pos - (depth *width)] +imageIn[pos-(depth*width)+depth]+
					imageIn[pos - depth] /*+ imageIn[pos]*/  +imageIn[pos+depth] +
					imageIn[pos + (depth*width)-depth] + imageIn[pos+ (depth*width)] +imageIn[pos+(depth*width)-depth]
				
				)/8;
				if (delta >255) imageOut[pos]=255;
				else imageOut[pos]=delta;
			
			}
		}
	}
	
	memcpy(vd->image, imageOut, vd->grab_size);
	free(imageOut);
}


/* Template for a effect function. */

/*
void myeffect (struct vd_video *vd, ....) {  // The extra parameters are on your own
	unsigned long int a=0;

	for (a=0; a < vd->grab_size; a++) { // loop through image
		MyImageProcessingAlgorithm(*(vd->image + a)); // reach each byte
	}

}

*/

