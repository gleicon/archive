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
#include <errno.h>
#include <time.h>
#include <math.h>
#include <stdarg.h>

#ifdef HASJPEG
#include <jpeglib.h>
#endif

#ifdef HASPNG
#include <png.h>
#endif

#define DOT fprintf(stderr,".");

#define VD_RAW	1
#define VD_PNM	2
#define VD_JPG	3
#define VD_TGA_24	4
#define VD_PNG	5

#define MAXLEN 255
#define DOT fprintf(stderr,".");

#ifndef CONFIG_FILE
#define CONFIG_FILE	"/etc/vd.conf"
#endif

#define MAXPATH		1024
#define MAXFNAME	25

struct vd_flags {

        int hue, contrast, brightness, colour, nograb, showc, singleframe;
	        };



struct vd_video{
        struct video_picture      grab_pic;
        struct video_capability   grab_cap;
        struct video_tuner        grab_tun; /* choose PAL/NTSC/SECAM */
        struct video_mmap         grab_buf;
        struct video_mbuf         grab_vm;
	struct video_channel	  grab_vid;

 	int                grab_fd, grab_size;
        unsigned char     *grab_data;
	int x, y, w, channel;
	unsigned char *v_device; /* device */
	unsigned char *v_mode; /* mode */
	unsigned char *v_filename; /* files and paths */
	short int img_format;
	#ifdef HASJPEG
		unsigned char *v_fjpeg; /* jpeg imagename */
	#endif
	
	// begin the stamp vars.
	unsigned char *stamp;
	unsigned int stamp_pos_x, stamp_pos_y, stamp_scale_w, stamp_scale_h;
	
	// info
	unsigned char tstampname; // flag: 1 if the name must be tstamp-seq-in
	unsigned char frame; // current frame (loop) 
	unsigned char *image; // will always point to the current image (loop)
	unsigned char framerate; // frames by sec (loop)
	unsigned char cycleinput; // active auto input change (loop)
  	struct vd_flags	v_flags;

} ;

