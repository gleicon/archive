/*
 * funcoes gerais
 */
 


#include "main.h"


void showhelp(void) {
	fprintf (stderr,"VideoDog Version %s\n", VERSION);
	fprintf (stderr,"Gleicon S. Moraes\n");
	fprintf (stderr,"Compile time options \n");
#ifdef BENCH
	fprintf (stderr, "-> Simple benchmark support\n");
#endif
#ifdef STAMP
	fprintf (stderr, "-> Timestamp support enabled\n");
#endif
#ifdef HASJPEG
	fprintf (stderr, "-> JPEG support \n");
#endif
#ifdef BGR2RGB
	fprintf (stderr, "-> Using bgr2rgb asm swap\n");
#endif
	fprintf (stderr, "-> Config file PATH: %s\n", CONFIG_FILE);
	fprintf (stderr, "For information on the options, check the USAGE file in the documentationi\n\n");
	

	exit(0);
	}

/* signal handler */
void _sighandler (int sig) {
	switch (sig){
			
		case SIGINT: /* ctrl+c */
			fprintf (stderr, "Caught SIGINT - Cleaning \n");
			exit(0);
			break;
		}
	
}

#ifdef HASJPEG
#define FIXEDQUALITY	30 	// that should be enough to most uses...

int create_jpeg (struct vd_video *vd) {

        char *fileout= vd->v_filename;
	unsigned char *img=vd->image;
	int lx=vd->x;
	int ly=vd->y;
	int lw=vd->w;
	
	FILE *fp;
	unsigned char *line;  // pointer to line
	unsigned int linesize = lx * lw, i;
	struct jpeg_compress_struct cinfo;
        struct jpeg_error_mgr jerr;


	
	if (!strncmp(fileout, "-", 1)) {
	#ifdef DEBUG 
		fprintf(stderr,"File: stdout\n");
	#endif
		fp=stdout;
	} else {
		if ( (fp=fopen(fileout , "w")) == NULL) {
        		perror ("fopen");
			return -1;
		}
	}
	
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
        jpeg_stdio_dest(&cinfo, fp);
        cinfo.image_width = lx;
        cinfo.image_height = ly;

	cinfo.input_components = lw;
	if (lw == 1)  cinfo.in_color_space = JCS_GRAYSCALE;
	else cinfo.in_color_space = JCS_RGB;

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, FIXEDQUALITY, TRUE);
	jpeg_start_compress(&cinfo, TRUE);
	
	line=img;

	for (i = 1; i <= ly; i++) {
		jpeg_write_scanlines(&cinfo, &line, 1);
		line=img + (linesize * i);
		}
	
	jpeg_finish_compress(&(cinfo));
	jpeg_destroy_compress(&(cinfo));
	fclose (fp);
	
	return 0;		
}
#endif

int create_pnm (struct vd_video *vd) {
unsigned char *buf=0;
int x=vd->x; 
int y=vd->y;
int depth=vd->w; 
        FILE *fp;

	buf=vd->image;
	
	
	if (!strncmp(vd->v_filename, "-", 1)) {
	#ifdef DEBUG 
		fprintf(stderr,"File: stdout\n");
	#endif
		fp=stdout;
	} else {
		if ( (fp=fopen(vd->v_filename , "w")) == NULL) {
        		perror ("fopen");
			return -1;
		}
	}
	


        if (depth==3) fprintf(fp, "P6\n%d %d\n255\n", x, y);
        else if (depth==1) fprintf(fp, "P5\n%d %d\n255\n", x, y);
        fwrite ((unsigned char*) buf, x * y * depth, 1, fp);
	fclose(fp);
return 0;
}


int create_raw (struct vd_video *vd) {
unsigned char *buf=0;
int x=vd->x; 
int y=vd->y;
int depth=vd->w; 
        FILE *fp;

	buf=vd->image;
	
	
	if (!strncmp(vd->v_filename, "-", 1)) {
	#ifdef DEBUG 
		fprintf(stderr,"File: stdout\n");
	#endif
		fp=stdout;
	} else {
		if ( (fp=fopen(vd->v_filename , "w")) == NULL) {
        		perror ("fopen");
			return -1;
		}
	}
	


        fwrite ((unsigned char*) buf, x * y * depth, 1, fp);
	fclose(fp);
return 0;
}


int create_targa24 (struct vd_video *vd) {
	unsigned char *buf=0;
	int x=vd->x; 
	int y=vd->y;
	unsigned long a=0;
        FILE *fp;
	if (vd->w != 3) {
		fprintf(stderr,"Image must be 24 bits (3 bytes) depth. Aborting TARGA.\n");
		return (1);
		}
	buf=vd->image;
	
	if (!strncmp(vd->v_filename, "-", 1)) {
	#ifdef DEBUG 
		fprintf(stderr,"File: stdout\n");
	#endif
		fp=stdout;
	} else {
		if ( (fp=fopen(vd->v_filename , "w")) == NULL) {
        		perror ("fopen");
			return -1;
		}
	}
	


	/* header */

        putc(0,fp);
	putc(0,fp);
	putc(2,fp);                         /* uncompressed RGB */
	putc(0,fp); putc(0,fp);
	putc(0,fp); putc(0,fp);
	putc(0,fp);
	
	putc(0,fp); putc(0,fp);           /* X origin */
	putc(0,fp); putc(0,fp);           /* y origin */
	
	putc((x & 0x00FF),fp);
	putc((x & 0xFF00) / 256,fp);
	
	putc((y & 0x00FF),fp);
	putc((y & 0xFF00) / 256,fp);
	
	
	
	putc(24,fp);                        /* 24 bit bitmap */
	putc(0,fp);
	

	for (a = (vd->grab_size ) ; a > 0; a--) putc (*(vd->grab_data + a), fp);
	fclose (fp);
return 0;
}


int loop_img( struct vd_video *vd) {
	static int a=0;
	static time_t last=0, new=0;  /* timestamp */
	static int seqnum; /* sequence number */
	

	
	char bufo[256];
	char xten[4][5] = {"-","raw", "pnm", "jpg"};
	
	
//	if (vd->w == 3 && vd->grab_data) {
//		swap_rgb24(vd->grab_data, vd->grab_buf.width * vd->grab_buf.height);
//	}
	
	memset (bufo, 0, sizeof(bufo));
	
	
	//if (!vd->tstampname) {
	//	sprintf (bufo, "img-%d.%s", a, xten[vd->img_format]);
	//	strcpy (vd->v_filename, bufo);
//	}else {
		/* generate a timestamp-sequence name. */
		if (!last) time(&last); /* first time */
		time(&new);
		if (last != new) {
			last=new;
			seqnum=0;
		} 

		sprintf (bufo, "%d-%02d-%d.%s", (unsigned int)last, seqnum, vd->channel, xten[vd->img_format]);
		seqnum++;
	#ifdef DEBUG
		fprintf (stderr,"-> filename: %s \n", bufo);
	#endif
	memset (vd->v_filename, 0, MAXFNAME);
	strncpy (vd->v_filename, bufo, strlen(bufo));
//	}

	switch (vd->img_format) {
		case VD_RAW:
			create_raw(vd);
			break;
		case VD_PNM:
			create_pnm(vd);
			break;
	#ifdef HASJPEG
		case VD_JPG:
			create_jpeg(vd);
			break;
	#endif
	}
	a++;
return 0;
}
void show_cap(struct vd_video *vd){ 	/* show capabilities */ 

	struct video_capability capability;
	struct video_channel channel;
	int j;
	

	fprintf (stderr,"\nVideoDog Version %s\n\n", VERSION);
	fprintf (stderr, "Device Name	: %s\n", vd->grab_cap.name);
	fprintf (stderr, "Device		: %s\n", vd->v_device);
	fprintf (stderr, "Max Width  	: %d\n", vd->grab_cap.maxwidth);
	fprintf (stderr, "Max Height 	: %d\n", vd->grab_cap.maxheight);
//	fprintf (stderr, "Max Buffers 	: %d\n", vd->grab_vm.frames); broken
	fprintf (stderr, "\n");
	fprintf (stderr, "Current Settings :\n\n");
	fprintf (stderr, "\tMode		: %s\n", vd->v_mode);
	fprintf (stderr, "\tBrightness 	: %i\n", vd->grab_pic.brightness);
	fprintf (stderr, "\tHue        	: %i\n", vd->grab_pic.hue);
	fprintf (stderr, "\tColour     	: %i\n", vd->grab_pic.colour);
	fprintf (stderr, "\tContrast   	: %i\n", vd->grab_pic.contrast);
	
	if (ioctl(vd->grab_fd, VIDIOCGCAP, &capability) == -1) {
		perror("ioctl VIDIOCGCAP");
	}
	
	if (!(capability.type & VID_TYPE_CAPTURE)) {
		 fprintf(stderr, "device can't capture\n");
	}
	
	fprintf (stderr, "\tInput names	: \n");
   
   for (j = 0; j < capability.channels; j++) {
       channel.channel = j;
       if (ioctl(vd->grab_fd, VIDIOCGCHAN, &channel) == -1) {
       		perror("ioctl VIDIOCGCHAN");
	} else {
		fprintf(stderr, "\t\t%d - %s\n ", j, channel.name);
		}
	}
	fprintf(stderr,"\n\n");
}

// This fuction only works for the BGR24 mode !!!
// huge overhead, need a better way to do it.

void swap_rgb24(unsigned char *mem, int n) {
        unsigned char  c;
	unsigned char *p = mem;
	int   i = n ;
	while (--i) {
		c = p[0];
		p[0] = p[2];
		p[2] = c;
		p += 3;
		}
}


#ifdef BGR2RGB
#error "oieee"
/* bgr2rgb -> flips the frame as fast as possible (1 bgr field @ time) */
/* Gleicon S. Moraes (gleicon@terra.com.br) */
/* __asm__() fix  Marco Aurelio Casaroli (nada@onda.com.br) */

/*  

	usage:  bgr2rgb(unsigned char *buffer, unsigned int size);
	compiler keys: NONE m0f0
	

*/


#define bgr2rgb(a, b) __bgr2rgb_16(a, b) 

void __bgr2rgb_16 (unsigned char *b, unsigned int tam) {

	
	__asm__	__volatile__ (" xorl %%ebx, %%ebx label: movb (%1, %%ebx), %%al 		movb 2(%1, %%ebx), %%ah movb %%al, 2(%0, %%ebx) movb %%ah,(%0, %%ebx)		addl $3, %%ebx	decl %2	jnz label" : "=r"(b) : "r" (b), "r" (tam) : "al", "ah", "ebx"); /* modify */

}

#endif


#ifdef HASPNG
int create_png(struct vd_video *vd) {
FILE *image;
unsigned long width, height;
int bitdepth, colourtype;
png_uint_32 i, j, rowbytes;
png_structp png;
png_infop info;
png_bytepp row_pointers = NULL;
unsigned char sig[8];
char *raster;




if((image = fopen("output.png", "wb")) == NULL){
    		fprintf(stderr, "Could not open the output image\n");
        	exit(42);
	 }

// Get ready for writing
if ((png = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL,
NULL)) == NULL){
fprintf(stderr, "Could not create write structure for PNG (out of memory?)");
exit(42);
}
// Get ready to specify important stuff about the image
if ((info = png_create_info_struct (png)) == NULL){
fprintf(stderr,
"Could not create PNG info structure for writing (out of memory?)");
exit(42);
}
if (setjmp (png_jmpbuf (png))){
fprintf(stderr, "Could not set the PNG jump value for writing");
exit(42);
}

// This is needed before IO will work (unless you define callbacks)
png_init_io(png, image);
// Define important stuff about the image
png_set_IHDR (png, info, width, height, bitdepth, PNG_COLOR_TYPE_RGB,
PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
PNG_FILTER_TYPE_DEFAULT);
png_write_info (png, info);
// Write the image out
png_write_image (png, row_pointers);
// Cleanup
png_write_end (png, info);
png_destroy_write_struct (&png, &info);
fclose(image);
}

#endif
