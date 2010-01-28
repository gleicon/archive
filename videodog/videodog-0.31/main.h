/* 
 * main.h prototypes, includes and global flags
 *
 */

#include "global.h"
#include "effects.h"



/* prototypes */

void showhelp(void); 
void swap_rgb24(unsigned char *, int );
int grab_one(struct vd_video *);
int grab_init();
int update_cap();
void _sighandler (int sig);

/* asm swap function */
void __bgr2rgb_16 (unsigned char *b, unsigned int tam); 
#define bgr2rgb(a, b) __bgr2rgb_16(a, b)


#ifdef HASJPEG
//#define FIXEDQUALITY	75 	// that should be enough to most uses...
#define FIXEDQUALITY	30 	// that should be enough to most uses...
int create_jpeg (struct vd_video *vd);
#endif

int fx_type;

int set_picture();
void cycle_channel(struct vd_video *);
void set_channel(struct vd_video *); 
void set_mode(); 
void show_cap();
void swap_rgb24(unsigned char *mem, int n); 
int grab_init();
int update_cap();
int grab_one(struct vd_video *); // single frame
int grab_loop(struct vd_video *); // double frame, continuous capture
int create_pnm (struct vd_video *vd);
int create_raw (struct vd_video *vd);
int create_targa24(struct vd_video *vd);
int loop_img(struct vd_video *vd);
int command_line (struct vd_video *vd, int, char **, struct vd_flags *vf);
int read_config (struct vd_video *vd, struct vd_flags *vf);
void rgb24_printf (int , int , int, int, struct vd_video *vd, const char *fmt, ...);
