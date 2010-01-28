/* 
 *	alpha blend 
 *	(c) gleicon - 2003 
 * 	blends 2 pixels using alpha factor from 0 to 255
 *
 *	*_from - smallest image, must be smaller than the destination
 *	*_to - destination image
 *
 *
 *	from, from_w, from_h, from_depth 	- Data from logo 
 *	to, to_w, to_h, to_depth,		- Data to mix it
 *	off_x, off_y, alpha			- Stuff-o
 *
 *	off_x, off_y -> offset within image
 *	alpha - 0 to 255 (0 = opaque, 255 almost invisible)
 *
 *	After loading both image, just fill in the parameters and be happy
*/


/* aux function */

int blend_pix (unsigned char p1, unsigned char p2, unsigned char alpha ) {
	unsigned char diff ;
	float alphaFactor = alpha/255.0;
	unsigned char value;
	
	diff = (p1>p2)?p1-p2:p2-p1;
	
	if (diff == 0) return p1;
	
	diff = (unsigned char)(diff * alphaFactor);

	if (p1>p2) value = p1 - diff;
	else value = p1 + diff;
	
	return value;
}


int alphablend(unsigned char *from, int from_w, int from_h, int from_depth, 
		unsigned char *to, int to_w, int to_h, int to_depth, 
		int off_x, int off_y, unsigned char alpha){

	       int a, b, from_size, to_size, sub_pixel;
	       unsigned char *from_pix, *to_pix;

		if (to_depth != from_depth) {
			// unmatched depths
			return -1;
		}
		if (from_w > to_w || from_h > to_h){
			// "from" image is bigger than "to"
			return -2;
		}
		if (((off_x + from_w) > to_w) || ((off_y + from_h) > to_h)) {
			// offset out of range
			return -3;
		}

		from_size=from_w*from_h* from_depth;
		to_size=to_w*to_h* to_depth;

		for (a=0; a<from_h; a++) { //height
			for (b=0; b<from_w * from_depth; b+=from_depth) { // width
				for (sub_pixel=0; sub_pixel < to_depth; sub_pixel++) {
					from_pix= from + (a * from_w * from_depth) + b + sub_pixel;
					to_pix= to + ((a+off_y) * to_w * to_depth) + (b + off_x)+ sub_pixel ;
					//to_pix= to + b + sub_pixel + (a * to_w * to_depth) ;
					*(to_pix)=blend_pix(*(from_pix), *(to_pix), alpha);
				}
			}
		}

		
	

}
