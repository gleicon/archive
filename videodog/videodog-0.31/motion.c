/* Motion detection package
 *  (c) Gleicon S. Moraes
 *
 *  The simpliest algorithm to do it is:
 *
 *  [Get 1st frame]
 *  [Copy to reference buffer]
 *  loop:
 *      [Get frame]
 *      [Compare with reference buffer +- tolerance]
 *      if (changed)
 *      		[Store it back in the reference buffer]
 *      		[Signal back]
 *		[Loop]
 *      else 
 *      		[goto loop]
 * 
 *
 * More advanced forms can use a statistical approach and divide the image in 
 * small blocks to keep things balanced. the _tolerance_ factor is an attempt in
 * to dealing with subtle changes that should not be interpreted as movement
 * A mask with the region of interest can be taken in consideration
 * This method uses a *element* concept, which is the complete pixel, and calculates if all sub-elements were modified, or just
 * one of them. Statistically, if there is no change in one of the sub-elements within a pixel, chances are high its just a noise or
 * light variation. If all sub elements are affected, then the average counter is incremented. Testing shows that average
 * rates above 5% of the total image size are significant motion.
 */


#include "_motion.h"


#define DEBUG
#define DOT fprintf(stderr,".");

// provides automagic testing for more than 1 channel
#define MAXCHAN 4

int multi_motion(struct vd_video *vd, unsigned char tolerance) {
        static int first=1;
        static struct image_motion _stored_image[MAXCHAN], _cur_image;
        int a, img_size, my_average;

        img_size=vd->grab_size;
	if (first) { // aloca, etc
		for (a=0; a<vd->grab_cap.channels; a++) {
			_stored_image[a].image=malloc(img_size);
			_stored_image[a].x=vd->x;
			_stored_image[a].y=vd->y;
			_stored_image[a].depth=vd->w;
			if (_stored_image[a].image==NULL) {
				 perror("malloc: stored_image\n");
				 return -1;
			}
		}
		first=0;
	}
        // seleciona input e compara

        _cur_image.image=vd->image;
        _cur_image.x=vd->x;
        _cur_image.y=vd->y;
        _cur_image.depth=vd->w;

        my_average=detect_motion(&_stored_image[vd->channel], &_cur_image, tolerance); // our tolerance at +-tolerance
	return my_average;
}
								
			
							
			


int detect_motion (struct image_motion *_reference_image, struct image_motion *_local_img, char _tolerance) {
	int a, pixel,  _img_size, b;
	unsigned char ref_elem, given_elem;
	unsigned int _average=0, sub_avg;
	
	b=0;	
	_average=sub_avg=0;	
	_img_size=_reference_image->x * _reference_image->y * _reference_image->depth;
	if (_img_size != _local_img->x * _local_img->y * _local_img->depth) {
		#ifdef DEBUG
		fprintf(stderr,"unmatched images\n\n");
		#endif
		return FALSE;
		}
	// clean it up
	_local_img->left=_local_img->right=0;
	
	/* travels through the image`s elements */
	for (a=0; a< _img_size; a=a+_reference_image->depth) {
		sub_avg=0;
		/* travels through the element */
		for (pixel=0; pixel< _reference_image->depth; pixel++) {
			ref_elem=*(_reference_image->image+a+pixel);
			given_elem=*(_local_img->image+a+pixel);
			if (ref_elem != given_elem) {  
				/* checks the tolerance */
				if (ref_elem > (given_elem + _tolerance) || 
				    ref_elem < (given_elem - _tolerance))	sub_avg++;			
			}
			if (sub_avg==_local_img->depth) {
				//_average++;
				if (b / _local_img->y < (_local_img->x /2)) _local_img->left++;
				else _local_img->right++;
			}
		
		}
	b++;
	}
 
 	/* replaces the image - it can be substituted by an average or composed image */
	/* all changes: */
	_average=_local_img->left + _local_img->right;
	memcpy (_reference_image->image, _local_img->image,_img_size);
	_reference_image->x=_local_img->x;
	_reference_image->y=_local_img->y;
	_reference_image->depth=_local_img->depth;
	_reference_image->left=_local_img->left;
	_reference_image->right=_local_img->right;
	return (_average); /* returns the count of pixels changed beyond the tolerance ( full elements ) */
}




/* subtracts one image from another */

int subtract_motion (struct image_motion *_reference_image, struct image_motion *_local_img, char _tolerance) {
	struct image_motion	_temp_image; // temp buffer
	int a, pixel,  _img_size;
	unsigned char ref_elem, given_elem;
	unsigned int _average=0, sub_avg;
	
	
	_average=sub_avg=0;	
	_img_size=_reference_image->x * _reference_image->y * _reference_image->depth;
	if (_img_size != _local_img->x * _local_img->y * _local_img->depth) {
		#ifdef DEBUG
		fprintf(stderr,"unmatched images\n\n");
		#endif
		return FALSE;
		}
	
	_temp_image.image=malloc(_img_size * sizeof(unsigned char));
	if ((_temp_image.image == (unsigned char *)NULL)) {
		perror("subtract_image: malloc");
		return(-1);
	}
	
	memcpy (_temp_image.image, _local_img->image,_img_size);
	_temp_image.x=_local_img->x;
	_temp_image.y=_local_img->y;
	_temp_image.depth=_local_img->depth;

	/* travels through the image`s elements */
	for (a=0; a< _img_size; a=a+_reference_image->depth) {
		sub_avg=0;
		/* travels through the element */
		for (pixel=0; pixel< _reference_image->depth; pixel++) {
			ref_elem=*(_reference_image->image+a+pixel);
			given_elem=*(_local_img->image+a+pixel);
			
			/* checks the tolerance */
			if (ref_elem != given_elem) {  
				if (ref_elem > (given_elem + _tolerance) || 
			    		ref_elem < (given_elem - _tolerance))	sub_avg++;			
			}
		}	
			
		if (sub_avg==_reference_image->depth) {
	//		*(_local_img->image+a+pixel)=(*(_reference_image->image+a+pixel) + *(_local_img->image+a+pixel)) / 2;		
	//		*(_local_img->image+a+pixel-1)=(*(_reference_image->image+a+pixel-1) + *(_local_img->image+a+pixel-1)) /2 ;		
	//		*(_local_img->image+a+pixel-2)=(*(_reference_image->image+a+pixel-2) + *(_local_img->image+a+pixel-2)) /2 ;		
			_average++;
		} else {
			*(_local_img->image+a+pixel)=0;		
			*(_local_img->image+a+pixel-1)=0;
			*(_local_img->image+a+pixel-2)=0;
		
		}
	}
 
 	/* replaces the image - it can be substituted by an average or composed image */
	memcpy (_reference_image->image, _temp_image.image,_img_size);
	_reference_image->x=_temp_image.x;
	_reference_image->y=_temp_image.y;
	_reference_image->depth=_temp_image.depth;
	
	free(_temp_image.image);
	return (_average); /* returns the count of pixels changed beyond the tolerance ( full elements ) */
}





/* load PHM file */
int load_pnm(char *wot, struct image_motion *bufo){
        FILE *fp;
	int x, y, m, depth=1;

	unsigned char ui[4];
	if (!bufo->image) {
		fprintf(stderr,"Buffer is null \n");
		exit(0);
	}
	if (!(fp=fopen(wot, "rb"))) {
		perror("file");
		exit(0);
	}
	fscanf(fp, "%s\n%d %d\n%d\n", ui, &x, &y, &m); // header
	fread(bufo->image, x * y * depth, 1, fp);
	//fprintf(stderr,"X: %d\nY: %d\n",  x , y );
	fclose(fp);
	bufo->x=x;
	bufo->y=y;
	bufo->depth=1; // grayscale images only plz
}

int detect_motion_mask (struct image_motion *_reference_image, struct image_motion *_local_img, struct image_motion *mask, char _tolerance) {
	int a, pixel,  _img_size, b;
	unsigned char ref_elem, given_elem;
	unsigned int _average=0, sub_avg;
	
	b=0;	
	_average=sub_avg=0;	
	_img_size=_reference_image->x * _reference_image->y * _reference_image->depth;
	if (_img_size != _local_img->x * _local_img->y * _local_img->depth) {
		#ifdef DEBUG
		fprintf(stderr,"unmatched images\n\n");
		#endif
		return FALSE;
		}
	if (mask->image==NULL || mask->x != _local_img->x || mask->y != _local_img->y) {
		#ifdef DEBUG
		fprintf(stderr,"fix the mask x:%d y:%d depth:%d\n", mask->x, mask->y, mask->depth);
		#endif
		return -1;
	}
	
	// clean it up
	_local_img->left=_local_img->right=0;
	
	/* travels through the image`s elements */
	for (a=0; a< _img_size; a=a+_reference_image->depth) {
		if (*(mask->image +b) != 255) {
			b++;
			continue; // skips if pixel is 0
		}
		sub_avg=0;
		/* travels through the element */
		for (pixel=0; pixel< _reference_image->depth; pixel++) {
			ref_elem=*(_reference_image->image+a+pixel);
			given_elem=*(_local_img->image+a+pixel);
			if (ref_elem != given_elem) {  
				/* checks the tolerance */
				if (ref_elem > (given_elem + _tolerance) || 
				    ref_elem < (given_elem - _tolerance))	sub_avg++;			
			}
			if (sub_avg==_local_img->depth) {
				//_average++;
				if (b / _local_img->y < (_local_img->x /2)) _local_img->left++;
				else _local_img->right++;
			}
		
		}
	b++;
	}
 
 	/* replaces the image - it can be substituted by an average or composed image */
	/* all changes: */
	_average=_local_img->left + _local_img->right;
	memcpy (_reference_image->image, _local_img->image,_img_size);
	_reference_image->x=_local_img->x;
	_reference_image->y=_local_img->y;
	_reference_image->depth=_local_img->depth;
	_reference_image->left=_local_img->left;
	_reference_image->right=_local_img->right;
	return (_average); /* returns the count of pixels changed beyond the tolerance ( full elements ) */
}


																		      
