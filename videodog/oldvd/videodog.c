/* 
 * Compile with:
 * cc videodog.c -o videodog -O2
 *
 * videodog - 12/2000 - 01/2000
 * 
 * Gleicon S. Moraes - gleicon@uol.com.br rde@linuxbr.com
 * 		Anaximandro Melo warned me about some warnings and forgotten casts. Thanks.
 *		
 * Just grabs a frame from v4l device or set its picture defaults. Useful for scripts, webcam scripts. 
 * Output: a raw frame ( RGB ) or pnm frame ( -p option )
 * 
 * Tested with: CPiA driver
 *		Bttv driver ( bt 848 & 878 )
 *		ov511 ( webcam III usb )
 * FIXME: how to make it use the two buffer capture from bttv cards ?
 * 
 * options you need:
 *
 * h - help
 * x - width
 * y - height
 * w - depth ( in bytes )
 * d - device
 * p - pnm output (24 or 8 bits only )
 *
 * Optional:
 *
 * u - Hue
 * c - Contrast
 * b - Brightness
 * l - Colour
 * s - show device report
 * n - no capture, just setup and exit clean
 *
 * Examples:
 *	./videodog -s -d /dev/video0 
 *			- query the device about settings.
 *	./videodog -x 320 -y 240 -w 3 -d /dev/video0 -p | cjpeg > lelel.jpg
 *			- produces a lelel.jpg image with 320x240x24bpp ( 3 * 8)
 *	./videodog -s -d /dev/video0 -u 200 
 *			- set Hue == 200 and query the device
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <asm/types.h>
#include <linux/videodev.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>


#define VERSION "0.04"
#define OPTSTR "hx:y:w:d:pu:c:b:l:sn"
#define DOT fprintf(stderr,".");

void showhelp(void) {
	fprintf (stderr,"Video Dog tool Version %s\n", VERSION);
	fprintf (stderr,"Gleicon S. Moraes\n");
	fprintf (stderr,"Options you really need:\n");
	fprintf (stderr,"-x (number) image width\n");
	fprintf (stderr,"-y (number) image height\n");
	fprintf (stderr,"-w (number) image depth (in bytes) \n");
	fprintf (stderr,"-d device (like /dev/video0 )\n");
	fprintf (stderr,"-p set the output in pnm format (24 or 8 bits only)\n");
	fprintf (stderr, "Other options :\n");
	fprintf (stderr,"-h just show this lines, doesn't help you ok ?\n");
	fprintf (stderr,"-u image Hue\n");
	fprintf (stderr,"-c image Contrast\n");
	fprintf (stderr,"-b image Brightness \n");
	fprintf (stderr,"-l image Colour\n");
	fprintf (stderr,"-s show device report\n");
	fprintf (stderr,"-n Dont capture an image, just setup the device (Useful if you want to correct something before start grabbing)\n");
	exit(0);
	}

/* global */

unsigned short int pnm=0;
int x=0, y=0, w=0;
unsigned char *v_device; /* device */

static struct video_picture	 grab_pic;
static struct video_capability   grab_cap;
static struct video_channel      grab_vid; /* escolher canal e NTSC */
static struct video_mmap         grab_buf;
static int                       grab_fd, grab_size;
static unsigned char            *grab_data;
int hue=-1, contrast=-1, brightness=-1, colour=-1, nograb=0, showc=0;

/* prototype */

void swap_rgb24(char *, int );
unsigned char* grab_one(int *, int *);
int grab_init();


/* signal handler */
void _sighandler (int sig) {
	switch (sig){
			
		case SIGINT: /* ctrl+x */
			free (v_device);
			close (grab_fd);
			munmap(grab_data, grab_size);
			fprintf (stderr, "Caught SIGINT - Cleaning \n");
			break;
			}
	}
void set_picture() {
	if (hue > -1) grab_pic.hue=hue;
	if (contrast > -1) grab_pic.contrast=contrast;
	if (brightness > -1) grab_pic.brightness=brightness;
	if (colour > -1) grab_pic.colour=colour;
	if (ioctl(grab_fd, VIDIOCSPICT, &grab_pic) == -1) {
		perror ("PICTURE");
		exit (1);
		}
	}
	
void show_cap()	{ 	/* mostra settings atuais */ 
	fprintf (stderr,"\nVideo Cat tool Version %s\n", VERSION);
	fprintf (stderr, "Device Name: %s\n", grab_cap.name);
	fprintf (stderr, "Device: %s\n", v_device);
	fprintf (stderr, "Max Width  : %d\n", grab_cap.maxwidth);
	fprintf (stderr, "Max Height : %d\n", grab_cap.maxheight);
	fprintf (stderr, "Current Settings :\n\n");
	fprintf (stderr, "\tBrightness : %i\n", grab_pic.brightness);
	fprintf (stderr, "\tHue        : %i\n", grab_pic.hue);
	fprintf (stderr, "\tColour     : %i\n", grab_pic.colour);
	fprintf (stderr, "\tContrast   : %i\n", grab_pic.contrast);
	fprintf (stderr, "\n");
	}
	
int main (int argc, char **argv) {
	
	int c;
	signal (SIGINT, _sighandler);

	while ((c = getopt (argc, argv, OPTSTR)) != EOF)
		switch (c) {
			case 'x':
				x=atoi(optarg);
				break;
			case 'y':
				y=atoi(optarg);
				break;
			case 'w':
				w=atoi(optarg);
				break;
			case 'p':
				pnm=1;
				break;
			case 'd':
				v_device= (char *)malloc ((strlen(optarg) * sizeof(char))+1);
				memset(v_device, 0, strlen(optarg));
				strncpy(v_device, optarg, strlen(optarg) -1 );
				
				break;
			case 'u':
				hue=atoi(optarg);
				break;
			case 'c':
				contrast=atoi(optarg);
				break;
			case 'b':
				brightness=atoi(optarg);
				break;
			case 'l':
				colour=atoi(optarg);
				break;
			case 'n':
				nograb=1;
				break;
			
			case 's':
				showc=1;
				break;
			
			case 'h':
			default:
				showhelp();
				break;
			
			}
		if (!x || !y || !w ) nograb=1;
		if (!v_device) {
			fprintf (stderr, " I NEED A DEVICE NAME \n");
			showhelp();
			}

		grab_init();
		set_picture();
		if (showc) show_cap();
		
		if (pnm) {
			if (w == 1) { 
				fprintf(stdout, "P5\n%d %d\n255\n", x, y);
				}
			if (w == 3) { 
				fprintf(stdout, "P6\n%d %d\n255\n", x, y);
				}
			}
		if (!nograb) fwrite ((unsigned char*)grab_one( &x, &y), x * y * w, 1, stdout);
		
		free (v_device);
		close (grab_fd);
		munmap(grab_data, grab_size);
	
	} /* main */

void swap_rgb24(char *mem, int n) {
        char  c;
	char *p = mem;
	int   i = n;
	while (--i) {
		c = p[0];
		p[0] = p[2];
		p[2] = c;
		p += 3;
		}
	}

int grab_init() {
        if ((grab_fd = open(v_device,O_RDWR)) == -1 ) {
		perror("open videodev");
		exit(1);
		}
	if (ioctl(grab_fd,VIDIOCGCAP,&grab_cap) == -1) {
	        fprintf(stderr,"wrong device\n");
		exit(1);
		}
	memset (&grab_pic, 0, sizeof(struct video_picture));
	if (ioctl (grab_fd, VIDIOCGPICT, &grab_pic) == -1) {
		fprintf (stderr, "no pict");
		exit(1);
		}
	
	switch (w) {
		case 1:
			grab_buf.format = VIDEO_PALETTE_GREY;
			break;
		case 2:
			grab_buf.format = VIDEO_PALETTE_RGB565;
			break;
		case 3:
		default:
			grab_buf.format = VIDEO_PALETTE_RGB24;
			break;	
		}

	grab_buf.frame  = 0;
	grab_buf.width  = x;
	grab_buf.height = y;
	grab_size = x * y * w;
	grab_data = mmap(0,grab_size,PROT_READ|PROT_WRITE,MAP_SHARED,grab_fd,0);
	return(1);
	}
	
unsigned char* grab_one(int *width, int *height) {
	for (;;) {
		if (-1 == ioctl(grab_fd,VIDIOCMCAPTURE,&grab_buf)) {
			perror("ioctl VIDIOCMCAPTURE");
			} else {
				if (-1 == ioctl(grab_fd,VIDIOCSYNC,&grab_buf)) {
					perror("ioctl VIDIOCSYNC");
				} else {
				 	DOT
					// swap_rgb24(grab_data, grab_buf.width * grab_buf.width * w);
				 	*width  = grab_buf.width;
				 	*height = grab_buf.height;
				 	return grab_data;
			 	}
			}
			
		}
	}

