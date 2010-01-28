/* videodog4.c - v4l capture using the double buffer capability */
/* experimental code */
/* tested only w/ cpia driver over paralel port */



#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/videodev.h>


void DOT (void) { static int indice=0; fprintf(stderr,"%d", indice++); }


#define VIDEO_BUFFERS	2 // numero de buffers usados na captura

// padrao de imagens

#define WIDTH	320
#define	HEIGHT	240
#define	DEPTH	3
#define	IMGSIZE	(WIDTH * HEIGHT * DEPTH)



// structs globais

struct video_mbuf mbuf;
struct video_picture pic;
struct video_channel chn;
struct video_mmap buf[VIDEO_BUFFERS];
struct video_capability cap;
unsigned char *buffer;


int video_fd;
static int frame_flag=0;

int _grab_frame (int frame) { 		// captura

	if (ioctl(video_fd,  VIDIOCMCAPTURE, buf + frame) == -1) ; // captura para o frame determinado
	return 0;
}
	

int _grab_sync ( int frame ) { 		// sincroniza
	if (ioctl (video_fd, VIDIOCSYNC, buf+frame) == -1); // sincroniza o frame determinado
		return 0;
	return frame; // apenas para controle, retorna o frame
			
}


int _grab_open (char *device, int width, int height, int depth) {
	
	unsigned int size=0;

	
	if ((video_fd=open(device, O_RDWR)) < 0 ) return -1;
	
	if (ioctl (video_fd, VIDIOCGCAP, &cap) < 0) return -1;

	if (width > cap.maxwidth || height > cap.maxheight ||
		width < cap.minwidth || height < cap.minheight)
		               return -1;
	
	// if (ioctl (video_fd, VIDIOCGPICT, &pic) < 0) return -1;
	
	// chn.type = VIDEO_TYPE_CAMERA;
	// chn.norm = norm;
	// chn.channel = 0;
	
	// if (ioctl (video_fd, VIDIOCSCHAN, &chn) < 0) return -1;
	
	// mmap

	buf[0].frame = 0;
	buf[1].frame = 1;
	buf[0].width = buf[1].width = width;
	buf[0].height = buf[1].height = height;
	
	switch (depth) {

	case 1:
		buf[0].format = buf[1].format = VIDEO_PALETTE_GREY;
		break;
	case 2:
	        buf[0].format = buf[1].format = VIDEO_PALETTE_RGB565;
	        break;
	case 3:
	default:
	        buf[0].format = buf[1].format = VIDEO_PALETTE_RGB24;
	        break;
	}
			
	size = width * height * depth;
	mbuf.size = size * VIDEO_BUFFERS;
	mbuf.frames = 0;
	mbuf.offsets[0] = 0;
	mbuf.offsets[1] = size;

	if (ioctl (video_fd, VIDIOCGMBUF, &mbuf) < 0) return -1;
	buffer = mmap (0, mbuf.size, PROT_READ | PROT_WRITE, MAP_SHARED, video_fd, 0);
	
	if (buffer == (unsigned char *) -1) return -1;
	if (_grab_frame(frame_flag) < 0) return 0;
	
	if (frame_flag) frame_flag=0;
	else frame_flag=1;
	
	return 0;
	
}

void _grab_close (void) {
	munmap (buffer, mbuf.size);
	close (video_fd);
}


unsigned char *_grab_pix (void) { 	// retorna um frame, alternando
	
//	static int frame=frame_flag; // registro static , inicia no frame_flag e a cada chamada alterna
	if (_grab_frame(frame_flag) < 0) return 0;
	
	frame_flag=!frame_flag; 
	
	if(_grab_sync(!frame_flag) < 0) return 0;
	
	return buffer + mbuf.offsets[frame_flag]; // inverte
}

void _save_test_pnm (unsigned char *buffer, int wido, int heio, unsigned long si) {

	static int indo=0;
	FILE *fp;
	char bewf[128];

	sprintf (bewf, "image-%d.pnm", indo);
	indo++;
	if ((fp = fopen (bewf, "w")) == NULL) return;
	fprintf (fp, "P6\n%d %d\n255\n", wido, heio);
	fwrite (buffer, si, 1, fp);
	fclose (fp);

	}

void main (void) {
	
	time_t t, t2;
	struct tm *mytm;
	
	int a;
	unsigned char *bufo;
	int interlock=0;
	
	if (_grab_open("/dev/video", WIDTH, HEIGHT, DEPTH) < 0) {
		fprintf (stderr,"cant open\n");
		exit(0);
		}
	while (1) {	
	time(&t);
	
	for (a=0; a< 30; a++) {
		ioctl (video_fd,  VIDIOCMCAPTURE, &buf[interlock]);
		interlock = !interlock;
		
		ioctl (video_fd, VIDIOCSYNC, &buf[interlock] ); 

		
	}
	
	time (&t2);
	
	fprintf (stderr,"Time: initial: %d \nfinal: %d\nelapsed: %d \n", (time_t)t, (time_t)t2, (time_t)t2-t);

	}
	_grab_close();

}
