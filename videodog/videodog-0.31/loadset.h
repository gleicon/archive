/*
 * loadset.h - data types and structures to the configuration drivers
 *
 */
#include "main.h"


#define FALSE           0
#define LINESIZE        256
#define MAXKEYS         17
#define KEYSIZE		20


char vd_configkeys [MAXKEYS][KEYSIZE]= {"device", "width", "height", "depth", "output_format", "effectchain", "capture_type", "file", "input_channel", "mode", "stamp", "stamp_pos_x", "stamp_pos_y", "stamp_scale_w", "stamp_scale_h", "cycleinputs"};	

enum { VD_DEVICE=0, VD_WIDTH, VD_HEIGHT, VD_DEPTH, VD_OFORMAT, VD_FXCHAIN, VD_CAPTYPE , VD_FILE, VD_ICHANNEL, VD_MODE, ST_TEXT, ST_POSX, ST_POSY, ST_SCAW, ST_SCAH, VD_CYCLEINPUT};

char *stripspace(char *);
void parse(char *, char **);
int param (char *, struct vd_video *, struct vd_flags *);

#ifdef HASJPEG
#define OPTSTR "hx:y:w:d:pu:c:b:l:m:sni:jyrif:qtS:X:Y:W:H:"
#else
#define OPTSTR "hx:y:w:d:pu:c:b:l:m:sni:yrif:qtS:X:Y:W:H:"
#endif

