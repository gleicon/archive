
/* Gleicon S. Moraes - gleicon@terra.com.br */
/* cc -o xputjpeg xputjpeg.c  -L/usr/X11R6/lib -lX11 -lXpm -lXext -lm -ljpeg */
/* just shows a jpeg image in the screen, using xlib */
/* for now, I am just translating from the 24 bits (JPEG) to 16 bits ( my X display) */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>
#include <stdio.h>
#include <jpeglib.h>
#include <setjmp.h>

void DEBUG (char *ui) { fprintf (stderr, ui); fflush(stderr);}

struct imagem { 
        unsigned char *buffer;
        int x;
        int y;
        int w;
        } imagem_t;


struct error_mgr {
        struct jpeg_error_mgr pub;    
        jmp_buf setjmp_buffer;        
        };

typedef struct error_mgr * my_error_ptr;


static struct {
        Display         *display;
        Window          win;
        GC              gc;
        XGCValues       gcv;
        int             screen;
} q;

typedef struct {
        XImage *image, *shape;
        XpmAttributes attributes;
} xpm_t;

#define OPTIONS "hd:s:"


GLOBAL(int) read_JPEG_file (char *, struct imagem *);


static void usage (char *name)
{
        fprintf (stderr, "asddddddddddddddd.\n");
        return;
}
void grava_pnm (char *out, struct imagem *im_t) {

        FILE *fp;
        if ((fp=fopen(out, "wb+")) == NULL) {
                DEBUG ("erro ao abrir file");
                exit (1);
                }
        fprintf(fp, "P6\n%d %d\n255\n", im_t->x, im_t->y);
        fwrite(im_t->buffer, im_t->x*im_t->y*im_t->w, 1, fp);
        fclose (fp);
        }
 
int main (int argc, char **argv)
{
        int           s, c, bpl, depth;
        Pixel         white, black;
        Window        Root, iconwin;
        char          display[128], *wname = argv[0],*translated_buffer;
        XEvent x_event;
        XImage *ximage;

        
	
	if (argc < 2) {
		fprintf (stderr,"Use: %s image.jpg\n", argv[0]);
		exit(1);
		}

	/* default display */
        memset (display, 0, sizeof (display));
        snprintf (display, sizeof (display), ":0.0");

        /* default speed */
        s = 100000;

        while ((c = getopt (argc, argv, OPTIONS)) != EOF)
        switch (c) {
                case 'd':
                        snprintf (display, sizeof (display), "%s", optarg);
                        break;
                case 's':
                        s = atoi (optarg);
                        s *= 1000;
                        break;
                case 'h':
                default:
                        usage (wname);
                        return 0;
        }
                        
        read_JPEG_file (argv[1], &imagem_t);
                
        q.display = XOpenDisplay (display);
        if (!q.display) {
                fprintf (stderr, "Can't open display %s!\n", display);
                return 0;
        }

        q.screen = DefaultScreen (q.display);
        Root = RootWindow (q.display, q.screen);
        white = WhitePixel (q.display, q.screen);
        black = BlackPixel (q.display, q.screen);

        q.win = XCreateSimpleWindow (q.display, Root, 0, 0, imagem_t.x, imagem_t.y, 0, black, white);
        iconwin = XCreateSimpleWindow (q.display, q.win, 0, 0, imagem_t.x, imagem_t.y, 0, black, white);

	printf("wid=%x\n",q.win);

	XSelectInput(q.display,q.win,ExposureMask);

        DEBUG ("Map");
        XMapWindow (q.display, q.win);
        q.gc = XCreateGC (q.display, q.win, 0, &q.gcv);
        DEBUG ("GRAVANDO");

        grava_pnm("tosco.pnm", &imagem_t);
        DEBUG("pre put image: ");

	XFlush(q.display);
	DEBUG("flush: ");
/*        
        ximage = XCreateImage (q.display, 
        	CopyFromParent, 
        	imagem_t.w, 
        	ZPixmap,
        	0, 
        	imagem_t.buffer, 
        	imagem_t.x, 
        	imagem_t.y, 
        	32, 
        	4 * imagem_t.x);
*/
	depth=DefaultDepth(q.display,q.screen);

	switch(depth) {
	
		case 24: 	
			bpl=4; break;
		
		case 16:
		case 15: 	
			bpl=2; break;
		default:
			// in this default, you must allocate colors!
			bpl=1; break;
	
	}

	translated_buffer=(char *)malloc(bpl*imagem_t.x*imagem_t.y);

	DEBUG("depth translation");

	switch(depth) {

		case 8: 
			{
				int x,y,z,k,pixel;
				
				for(z=k=y=0;y!=imagem_t.y;y++)
					for(x=0;x!=imagem_t.x;x++) {
					
						// for grayscale-only 8 bit depth
						// can't work in 8 bit color display
					
						pixel=(imagem_t.buffer[z++]+
						imagem_t.buffer[z++]+						
						imagem_t.buffer[z++])/3;
						
						translated_buffer[k++]=pixel;
					}			
			}
			break;

		case 16:
			{
				unsigned x,y,z,k,pixel,r,g,b;
				unsigned short *word;
				
				word=translated_buffer;
				
				for(z=k=y=0;y!=imagem_t.y;y++)
					for(x=0;x!=imagem_t.x;x++) {
					
						// for 16 bit depth, organization 565
					
						r=imagem_t.buffer[z++]<<8;
						g=imagem_t.buffer[z++]<<8;						
						b=imagem_t.buffer[z++]<<8;
						
						r &= 0xf800;
						g &= 0xfc00;						
						b &= 0xf800;
						
						word[k++]=r|g>>5|b>>11;
					}			
			}
			break;		

		case 32:
		case 24:
			{
				unsigned x,y,z,k;
				unsigned buffer;
				
				translated_buffer;
				
				for(z=k=y=0;y!=imagem_t.y;y++)
					for(x=0;x!=imagem_t.x;x++) {
					
						// for 24 bit depth, organization BGRX
					
						translated_buffer[k+0]=imagem_t.buffer[z+2];
						translated_buffer[k+1]=imagem_t.buffer[z+1];
						translated_buffer[k+2]=imagem_t.buffer[z+0];
						k+=4; z+=3;
					}			
			}
			break;		

		default:
			DEBUG("unsuported depth for now.");
			break;
	
	}

        ximage = XCreateImage (q.display, 
        	CopyFromParent, 
        	depth, 
        	ZPixmap,
        	0, 
        	translated_buffer, 
        	imagem_t.x, 
        	imagem_t.y, 
        	bpl*8, 
        	bpl * imagem_t.x);

	DEBUG("ximage ok: ");

	XFlush(q.display);

	DEBUG("flush: ");


      while (1) {

                XNextEvent(q.display, &x_event);

                switch (x_event.type) {

                        //case CreateNotify:
                        case Expose: 
                        DEBUG ("put-o");

                        XPutImage (q.display, 
                        	q.win, 
                        	q.gc, 
                        	ximage, 
                        	0,0,0,0, 
                        	imagem_t.x, 
                        	imagem_t.y);   
                        	
                        XFlush (q.display);     
                      break;
                }
        }


        sleep (100);

        free (imagem_t.buffer); 
 

        return 0;
}
/* decompress jpeg
 * Gleicon - 05/10/99
 * cc -o jdec jdec.c -ljpeg
 */
 
/* Error handling */

    
METHODDEF(void) my_error_exit (j_common_ptr cinfo) {
        my_error_ptr myerr = (my_error_ptr) cinfo->err;
        (*cinfo->err->output_message) (cinfo);
        longjmp(myerr->setjmp_buffer, 1);
        }
   
GLOBAL(int) read_JPEG_file (char *filename, struct imagem *dest) {  
          struct jpeg_decompress_struct cinfo;
          struct error_mgr jerr;
          FILE *infile;
          JSAMPARRAY buffer;      
          int row_stride, lines=0, total_size;   
         unsigned char *buf_tmp;
        
          
          if ((infile = fopen(filename, "rb")) == NULL) {
                fprintf(stderr, "Can't open: %s\n", filename);
                return 0;
                }
          
          
          cinfo.err = jpeg_std_error(&jerr.pub);
          jerr.pub.error_exit = my_error_exit;  
          
          if (setjmp(jerr.setjmp_buffer)) {
                jpeg_destroy_decompress(&cinfo);
                jpeg_destroy_decompress(&cinfo);
                fclose(infile);
                return 0;
                }
          
          jpeg_create_decompress(&cinfo);
          jpeg_stdio_src(&cinfo, infile);
          (void) jpeg_read_header(&cinfo, TRUE);
          (void) jpeg_start_decompress(&cinfo);
          row_stride = cinfo.output_width * cinfo.output_components;
          total_size= row_stride * cinfo.output_height;

          
          buffer = (*cinfo.mem->alloc_sarray)
                          ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
          


          dest->x=cinfo.output_width;
          dest->y=cinfo.output_height;
          dest->w=cinfo.output_components;

          dest->buffer= malloc (total_size * sizeof (char ));
          buf_tmp=dest->buffer;
          
          /* read line per line  */
          
          while (cinfo.output_scanline < cinfo.output_height) {
              (void) jpeg_read_scanlines(&cinfo, buffer, 1);
                memcpy (buf_tmp, buffer[0], row_stride);               
                buf_tmp+=row_stride;
              lines++;
              }

          (void) jpeg_finish_decompress(&cinfo);
          jpeg_destroy_decompress(&cinfo);
            
          fclose(infile);
              
        }                  
            
