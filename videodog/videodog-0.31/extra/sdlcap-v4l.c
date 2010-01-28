// Based on sdlcap & videodog


#include <SDL/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/types.h>
#include <linux/videodev.h>

#define VERSION	"0.1"

#define	DEBUG	1
#define WIDTH	640
#define HEIGHT	480
#define DEPTH	24

#define VIDEODEVICE	"/dev/video0"


// It contains an almost complete v4l library
// Video4linux control/info structures 

#define MAXINPUTS	4	// max inputs per card
#define DOT	fprintf(stderr,".");

struct input_data_t {
	 int hue, contrast, brightness, colour, nograb, showc, singleframe;
	 unsigned char v_mode[10];
};

struct vd_video {
	int grab_fd;
	unsigned char v4l_device[256];
	unsigned char v_mode[10];
	struct video_picture      grab_pic;
	struct video_capability   grab_cap;
	struct video_tuner        grab_tun; /* choose PAL/NTSC/SECAM */
	struct video_mmap         grab_buf;
	struct video_mbuf         grab_vm;
	struct video_channel      grab_vid;
	unsigned char		*grab_data; // mmap buffer
	unsigned char 		*image; // current image
	unsigned int  grab_size;
	unsigned int x, y, w, channel, frame;
	// per input data
	struct input_data_t input_data[MAXINPUTS];
	int current_input;
};


int DeviceInit(struct vd_video *vd) {
	int inputs;
	vd->grab_fd = open(vd->v4l_device, O_RDWR);
	if (vd->grab_fd < 0) {
		printf("Unable to open %s: %s\n", vd->v4l_device, strerror(errno));
		return -1;
	}
	// init vd->input_data[] members to default data
	for (inputs=0; inputs < MAXINPUTS; inputs++) {
		// set default mode
		memset (vd->input_data[inputs].v_mode, 0, 10);
		strncpy(vd->input_data[inputs].v_mode, "NTSC", 4);
		// set default picture attributes
		vd->current_input=inputs;
		DeviceGetPicture(vd);
	}
	// Load capabilities
	if (ioctl(vd->grab_fd,VIDIOCGCAP,&(vd->grab_cap)) == -1) {
		perror ("DeviceInit: VIDIOCGCAP (1)");
		return (-1);
	}
	
	vd->current_input=0;
	DeviceSetInput(vd, 0);

	return 0;
}

void DeviceInfo(struct vd_video *vd){
	struct video_capability capability;
	struct video_channel channel;
	int j;

	DOT
	fprintf (stderr,"\nSDL v4l cap - v. %s\n\n", VERSION);
	fprintf (stderr, "Device %s data\n\n",  vd->v4l_device);
	fprintf (stderr, "Device Name\t: %s\n", vd->grab_cap.name);
	fprintf (stderr, "Max Width\t: %d\n", vd->grab_cap.maxwidth);
	fprintf (stderr, "Max Height\t: %d\n", vd->grab_cap.maxheight);
	fprintf (stderr, "\n");
	fprintf (stderr, "Current Settings :\n\n");
	fprintf (stderr, "\tSelected input	: %d\n", vd->current_input);
	fprintf (stderr, "\tMode        	: %s\n", vd->input_data[vd->current_input].v_mode);
	fprintf (stderr, "\tBrightness  	: %i\n", vd->grab_pic.brightness);
	fprintf (stderr, "\tHue         	: %i\n", vd->grab_pic.hue);
	fprintf (stderr, "\tColour      	: %i\n", vd->grab_pic.colour);
	fprintf (stderr, "\tContrast    	: %i\n", vd->grab_pic.contrast);
	
	if (ioctl(vd->grab_fd, VIDIOCGCAP, &capability) == -1) {
		perror("ioctl VIDIOCGCAP");
	}

	if (!(capability.type & VID_TYPE_CAPTURE)) {
		fprintf(stderr, "device can't capture\n");
	}

	fprintf (stderr, "\tInput names : \n");

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

void DeviceClose( struct vd_video *vd){
	if (vd->grab_fd >= 0) {
		close(vd->grab_fd);
		vd->grab_fd = 0;
	}
}


int DeviceSetup(struct vd_video *vd) { 
	if (ioctl(vd->grab_fd,VIDIOCGCAP,&(vd->grab_cap)) == -1) {
		perror ("DeviceSetup: VIDIOCGCAP (1)");
		return (-1);
	}

	if (ioctl(vd->grab_fd,  VIDIOCGCHAN, &(vd->grab_vid))==-1){
		perror("DeviceSetup: VIDIOCGCHAN ");
	}

	// default settings

	vd->grab_vid.norm=VIDEO_MODE_NTSC;
	vd->grab_vid.channel=0;

	if (ioctl(vd->grab_fd,  VIDIOCSCHAN, &(vd->grab_vid))==-1){
		perror("DeviceSetup: VIDIOCSCHAN ");
	}

	memset (&(vd->grab_pic), 0, sizeof(struct video_picture));
        if (ioctl (vd->grab_fd, VIDIOCGPICT, &(vd->grab_pic)) == -1) {
		perror("DeviceSetup: VIDIOCGPICT ");
		return (-1);
	}

	switch (vd->w) {
		case 1:
			vd->grab_buf.format = VIDEO_PALETTE_GREY;
			break;
		case 2:
			vd->grab_buf.format = VIDEO_PALETTE_RGB565;
			break;
		case 3:
		default:
			vd->grab_buf.format = VIDEO_PALETTE_RGB24;
			break;
	}
	
	vd->grab_buf.frame  = 0;
	vd->grab_buf.width  = vd->x;
	vd->grab_buf.height = vd->y;
	vd->grab_size = vd->x * vd->y * vd->w;

	/* Query the actual buffers available */
	if(ioctl(vd->grab_fd, VIDIOCGMBUF, &(vd->grab_vm)) < 0) {
		perror("DeviceSetup: VIDIOCGMBUF ");
		return(-1);
	}
	// mmap video device`s buffers
	
	vd->grab_data = mmap(0,vd->grab_vm.size,PROT_READ|PROT_WRITE,MAP_SHARED,vd->grab_fd,0);
	// Default initial frame and offset
	vd->frame=0; 
	vd->grab_vm.offsets[vd->frame]=0;

	return (1);
}
// Get picture attributes to current input

int DeviceGetPicture (struct vd_video *vd) {
	if (ioctl(vd->grab_fd, VIDIOCGPICT, &(vd->grab_pic)) == -1) {
		perror ("DeviceSetPicture: VIDIOCGPICT");
		return (-1);
	}

	vd->input_data[vd->current_input].hue=vd->grab_pic.hue;
	vd->input_data[vd->current_input].contrast=vd->grab_pic.contrast;
	vd->input_data[vd->current_input].brightness=vd->grab_pic.brightness;
	vd->input_data[vd->current_input].colour=vd->grab_pic.colour;
	
	return (1);
}

int DeviceSetPicture (struct vd_video *vd) {
	if (ioctl(vd->grab_fd, VIDIOCGPICT, &(vd->grab_pic)) == -1) {
		perror ("DeviceSetPicture: VIDIOCGPICT");
		return (-1);
	}

	if (vd->input_data[vd->current_input].hue > -1) 
		vd->grab_pic.hue=vd->input_data[vd->current_input].hue;
	if (vd->input_data[vd->current_input].contrast > -1) 
		vd->grab_pic.contrast=vd->input_data[vd->current_input].contrast;
	if (vd->input_data[vd->current_input].brightness > -1) 
		vd->grab_pic.brightness=vd->input_data[vd->current_input].brightness;
	if (vd->input_data[vd->current_input].colour > -1) 
		vd->grab_pic.colour=vd->input_data[vd->current_input].colour;
	
	if (ioctl(vd->grab_fd, VIDIOCSPICT, &(vd->grab_pic)) == -1) {
		perror ("DeviceGetPicture: VIDIOCGPICT");
		return (-1); 
	}
	return (1);
}

int DeviceSetMode    (struct vd_video *vd) {
	unsigned char *v_mode=vd->input_data[vd->current_input].v_mode;
	vd->grab_vid.channel=vd->current_input;

	if (ioctl(vd->grab_fd, VIDIOCGCHAN, &(vd->grab_vid)) == -1) {
		perror("DeviceSetMode: VIDIOCGCHAN (1)");
		return;
	}
	
	if (strcmp(v_mode, "PAL") == 0){
		vd->grab_vid.norm = VIDEO_MODE_PAL;
	} else if (strcmp(v_mode, "NTSC") == 0) {
		vd->grab_vid.norm = VIDEO_MODE_NTSC;
	} else if (strcmp(v_mode, "SECAM") == 0) {
		vd->grab_vid.norm = VIDEO_MODE_SECAM;
	} else {
		fprintf(stderr, "DeviceSetMode: %s is not a valid mode. Defaulting to NTSC\n", v_mode);
		vd->grab_vid.norm = VIDEO_MODE_NTSC;
	}
	
	if (ioctl(vd->grab_fd, VIDIOCSCHAN, &(vd->grab_vid)) == -1) {
		perror("DeviceSetMode: VIDIOCSCHAN ");
		return (-1);
	}
	return (1);
}

int DeviceSetInput (struct vd_video *vd, int input) {

	struct video_channel l_chan;
	l_chan.channel=input;
	if (input >= MAXINPUTS) {
		fprintf(stderr,"DeviceSetInput: parameter > than MAXINPUTS\n");
		return (-1);
	}
	
	if (ioctl(vd->grab_fd, VIDIOCGCHAN, &l_chan) == -1) {
		perror("DeviceSetInput: VIDIOCGCHAN ");
		return (-1);
	}


	if (ioctl(vd->grab_fd, VIDIOCSCHAN, &l_chan) == -1) {
		perror("DeviceSetInput: VIDIOCSCHAN ");
		return (-1);
	}
	// Sets picture properties
	vd->current_input=input;

	DeviceSetPicture(vd);
	DeviceSetMode(vd);
	return (1);
}


// Link between v4l and SDL

void UpdateScreen (struct vd_video *vd, SDL_Surface *screen) {
	SDL_Surface *temp_surface;
	temp_surface= SDL_CreateRGBSurfaceFrom((void *) vd->image, vd->x, vd->y, vd->w * 8, vd->x * vd->w, 0xFF0000, 0x00FF00, 0x0000FF, 0x000000);
//	temp_surface= SDL_CreateRGBSurfaceFrom((void *) vd->image, vd->x, vd->y, 24, vd->x * vd->w, 0xFF0000, 0x00FF00, 0x0000FF, 0x000000);
	SDL_BlitSurface(temp_surface, NULL, screen, NULL);
	SDL_UpdateRect(screen, 0, 0, 0, 0);
	SDL_FreeSurface(temp_surface);
		
}

int main(int argc, char **argv) {

	SDL_Surface *screen;
	SDL_Event event;
	struct vd_video vd;
	int frame, framecount=0;
	time_t inicio, fim;

	vd.x=WIDTH;
	vd.y=HEIGHT;
	vd.w=DEPTH/8;
	
	memset (vd.v4l_device, 0, 255);
	strncpy(vd.v4l_device, "/dev/video0", 11);
	
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Error initializing SDL: %s\n",
		       SDL_GetError());
		return 1;
	}
	// automatic SDL cleanup on exit to avoid display mess
	
	atexit(SDL_Quit);

	if ((screen = SDL_SetVideoMode(WIDTH, HEIGHT, DEPTH, SDL_SWSURFACE)) == NULL) {
		printf("Unable to set video mode: %s\n",
		       SDL_GetError());
		return 1;
	}

	
	if (DeviceInit(&vd) < 0) {
		printf("Unable to initialize capture device\n");
		return 1;
	}
	if (DeviceSetup(&vd) < 0) {
		printf("Unable to prepare capture\n");
		DeviceClose(&vd);
		return 1;
	}
	// Doggy style - tell card to queue all frames beforehand
	for(frame=0; frame < vd.grab_vm.frames; frame++) {
		vd.grab_buf.frame = frame;
		if(ioctl(vd.grab_fd, VIDIOCMCAPTURE, &(vd.grab_buf))<0) {
			perror("main: VIDIOCMCAPTURE ");
			DeviceClose(&vd);
			exit(-1);
		}
	}
	 
	
	// main capture loop - queue and capture to all buffers device creates
	frame=0;

	while (1) {
		if (framecount==0) time(&inicio);
		while ((ioctl(vd.grab_fd, VIDIOCSYNC, &frame)) < 0) {
			#ifdef DEBUG
			fprintf(stderr, "Waiting frame: %d\n", frame);
			#endif
		}
		/* refer the frame */
		vd.frame=frame;
		vd.image=vd.grab_data + vd.grab_vm.offsets[frame];
		
		// send it to SDL surface
		UpdateScreen(&vd, screen);
		
		vd.grab_buf.frame = frame;
		if(ioctl(vd.grab_fd, VIDIOCMCAPTURE, &(vd.grab_buf))<0) {
			perror("main: VIDIOCMCAPTURE ");
			DeviceClose(&vd);
			exit(-1);
		}
		/* next frame */
		frame++;
		
		/* reset to the 1st frame */
		if (frame >= vd.grab_vm.frames) frame = 0;
	
		// Place to handle events
		time(&fim);
		if (fim-inicio ==1) {
			fprintf (stderr,"Frames %d/s\n", framecount);
			framecount=0;
		} else framecount++;
		
		SDL_PollEvent(&event);
		if (event.type == SDL_KEYDOWN ) {
			if(event.key.keysym.sym==SDLK_ESCAPE) break;
			if(event.key.keysym.sym==SDLK_F1) DeviceInfo(&vd);
			
		}
	}

	DeviceClose(&vd);
	SDL_Quit();
	
	return 0;
}

