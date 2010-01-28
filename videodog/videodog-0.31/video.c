/*
 *  Video4Linux functions
 */

#include "main.h"

#ifdef MOTION
#include "motion.h"
#endif

int set_picture(struct vd_video *vd) {
	
	if (ioctl(vd->grab_fd, VIDIOCGPICT, &(vd->grab_pic)) == -1) {
		perror ("PICTURE");
		return (-1);
		}
	
	if (vd->v_flags.hue > -1) vd->grab_pic.hue=vd->v_flags.hue;
        if (vd->v_flags.contrast > -1) vd->grab_pic.contrast=vd->v_flags.contrast;
	if (vd->v_flags.brightness > -1) vd->grab_pic.brightness=vd->v_flags.brightness;
	if (vd->v_flags.colour > -1) vd->grab_pic.colour=vd->v_flags.colour;
			    
	if (ioctl(vd->grab_fd, VIDIOCSPICT, &(vd->grab_pic)) == -1) {
		perror ("PICTURE");
		return (-1);
		}
	return (0);
	}

void cycle_channel(struct vd_video *vd) { // increments channel
	static int cur_chan=0;
	struct video_channel l_chan;

	if (cur_chan == vd->grab_cap.channels) cur_chan=0;
	

	
	l_chan.channel=vd->channel=cur_chan;
	
	set_channel(vd);
	cur_chan++;
}

void set_channel(struct vd_video *vd) {		
	
	struct video_channel l_chan;

	l_chan.channel=vd->channel;
	
	if (ioctl(vd->grab_fd, VIDIOCGCHAN, &l_chan) == -1) {
		perror("02 - VIDIOCGCHAN");
		return;
	}
	
	//l_chan.norm=vd->grab_tun.mode;
	
	#ifdef DEBUG
	fprintf (stderr, "channel: %d - (%d) Name: %s\n", vd->channel, l_chan.channel, l_chan.name);
	#endif
	
	if (ioctl(vd->grab_fd, VIDIOCSCHAN, &l_chan) == -1) {
		perror("02 - VIDIOCSCHAN");
		return;
	}
	
	set_picture(vd);
	set_mode(vd);
		       
}

void set_mode(struct vd_video *vd) {
        vd->grab_vid.channel=vd->channel;
	
	if (ioctl(vd->grab_fd, VIDIOCGCHAN, &(vd->grab_vid)) == -1) {
		perror("set_mode: VIDIOCGCHAN");
		return;
	}

	if (strcmp(vd->v_mode, "PAL") == 0){
		vd->grab_vid.norm = VIDEO_MODE_PAL;
        }
	else if (strcmp(vd->v_mode, "NTSC") == 0) {
		vd->grab_vid.norm = VIDEO_MODE_NTSC;
        }
	else if (strcmp(vd->v_mode, "SECAM") == 0) {
		vd->grab_vid.norm = VIDEO_MODE_SECAM;
	}
	else {
		fprintf(stderr, 
			"%s is not a valid mode. Defaulting to NTSC\n", vd->v_mode);
		vd->grab_vid.norm = VIDEO_MODE_NTSC;
					    
		return;
	}
	
	if (ioctl(vd->grab_fd, VIDIOCSCHAN, &(vd->grab_vid)) == -1) {
		perror("set_mode: VIDIOCSCHAN");
		return;
	}
}


int grab_init(struct vd_video *vd) {
        if ((vd->grab_fd = open(vd->v_device,O_RDWR)) == -1 ) {
		fprintf(stderr,"%s\n", vd->v_device);
		perror("open videodev");
		exit(1);
		}
	if (ioctl(vd->grab_fd,VIDIOCGCAP,&(vd->grab_cap)) == -1) {
	        fprintf(stderr,"wrong device\n");
		exit(1);
		}
	if (ioctl(vd->grab_fd,  VIDIOCGCHAN, &(vd->grab_vid))==-1){
        	perror("init: VIDIOCGCHAN");
		}
	// default settings

	vd->grab_vid.norm=VIDEO_MODE_NTSC;
        vd->grab_vid.channel=0; 
	 
	if (ioctl(vd->grab_fd,  VIDIOCSCHAN, &(vd->grab_vid))==-1){
        	perror("init: VIDIOCGCHAN");
		}
				 
	
	
	memset (&(vd->grab_pic), 0, sizeof(struct video_picture));
	if (ioctl (vd->grab_fd, VIDIOCGPICT, &(vd->grab_pic)) == -1) {
		fprintf (stderr, "no pict");
		exit(1);
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
		perror("VIDIOCGMBUF");
		exit(-1);
	}
	
	
	
	vd->grab_data = mmap(0,vd->grab_vm.size,PROT_READ|PROT_WRITE,MAP_SHARED,vd->grab_fd,0);
	
	
	vd->frame=0; /* default frame */
	vd->grab_vm.offsets[vd->frame]=0;

	#ifdef DEBUG
	fprintf (stderr,"offset %d\n", vd->grab_vm.offsets[vd->frame]);
	#endif

	return(1);
	}

int update_cap(struct vd_video *vd) {
	if (ioctl(vd->grab_fd,VIDIOCGCAP,&(vd->grab_cap)) == -1) {
	        fprintf(stderr,"couldn't update capabilities.\n");
		exit(1);
		}
	return(1);
	}


int grab_one(struct vd_video *vd) {
	char bufo[256];
	char xten[4][5] = {"-","raw", "pnm", "jpg"};
		
	
	#ifdef STAMP
	time_t t;
	struct tm *tm;
	char stringo[128];
	#endif
	
	/* gamma test */
	/* 
	   unsigned char g_table[256];
	   gamma_lookup(0.2, g_table);
	 
	*/
	
	
	
	if (-1 == ioctl(vd->grab_fd,VIDIOCMCAPTURE,&(vd->grab_buf))) {
			perror("ioctl VIDIOCMCAPTURE");
			exit(-1);
	} 
	if (-1 == ioctl(vd->grab_fd,VIDIOCSYNC,&(vd->grab_buf))) {
				perror("ioctl VIDIOCSYNC");
				exit(-1);
	}
 
	vd->image=vd->grab_data;

	if (vd->w == 3 && vd->grab_data  && (vd->img_format != VD_TGA_24)) {
			/* original or faster swap function */
			#ifdef BGR2RGB
			bgr2rgb(vd->grab_data, vd->grab_buf.width * vd->grab_buf.height);
			#else 
		 	
			swap_rgb24(vd->grab_data, vd->grab_buf.width * vd->grab_buf.height);
			#endif
	
		}


/* HOOK for effects */
	
	/* Uncomment and test the effects */
	/* try another order and parameters */
	/* effects test */
	/* threshold (vd, 30); */
	   //sobel(vd);   
	
	 /*gamma_apply(vd, g_table); */
	 /* laplace(vd); */ 
	/* noise (vd, 0.05); */
	 //negative(vd); 
	/* mirror(vd); */
	 /* lowpass(vd); */
	/* highpass(vd); */
	
	/* channel_threshold(vd); */
	/* predator(vd); */
	 /* posterize(vd, 3); */
	
	//threshold (vd, 200); 
	// sobel(vd);   
	//negative(vd);
	//mark_areas(vd);
	
	
	/* end test */
	#ifdef STAMP
	/* initial tests for the timestamp function */
	time (&t);
	tm = localtime (&t);
	if (vd->stamp) {	
		// validation
		if (vd->stamp_scale_w <= 0) vd->stamp_scale_w=1;
		if (vd->stamp_scale_h <= 0) vd->stamp_scale_h=1;
		strftime (stringo, sizeof (stringo), vd->stamp, tm);
		rgb24_printf(vd->stamp_pos_x, vd->stamp_pos_y, vd->stamp_scale_w, vd->stamp_scale_h, vd, "%s", stringo);
	}
	
	#endif
	/*
	 * timestamp
	 *
	 */
	 if (vd->tstampname) {
	 	sprintf(bufo, "%d-0.%s", (unsigned int)t,  xten[vd->img_format]);
 		#ifdef DEBUG
                fprintf (stderr,"filename: %s \n", bufo);
		#endif
		strncpy (vd->v_filename, bufo, strlen(bufo));
	 }
	vd->image=vd->grab_data; /* compatability */

	
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
			case VD_TGA_24:
				create_targa24(vd);
				break;
		}
	return 1;
}

int grab_loop(struct vd_video *vd){
	#ifdef STAMP
	time_t t;
	struct tm *tm;
	char stringo[128];
	#endif
	
#ifdef BENCH
	time_t ini,fim;
	int a=0;
#endif
#ifdef MOTION
	struct image_motion _ref_image, _cur_image, mask;
	int avg=0;
#endif

	int frame=0, i;

#ifdef MOTION
	/* initialize _ref_image, our reference image */
	if ((_ref_image.image=malloc(vd->x * vd->y * vd->w))==NULL) {
		perror("grab_loop: malloc _ref_image");
		exit(-1);
	}
	memset (_ref_image.image, 0, vd->grab_size);
	_ref_image.x=vd->x;
	_ref_image.y=vd->y;
	_ref_image.depth=vd->w;
	if ((mask.image=malloc(vd->x * vd->y * vd->w))==NULL) {
		perror("grab_loop: malloc mask");
		exit(-1);
	}
	load_pnm ("mask.pnm", &mask);
#endif

	/* Query the actual buffers available */
        if(ioctl(vd->grab_fd, VIDIOCGMBUF, &(vd->grab_vm)) < 0) {
		perror("VIDIOCGMBUF");
		exit(-1);
	}
	

	#ifdef DEBUG
	fprintf (stderr,"vm.size=%d ; grab_size= %d\n", vd->grab_vm.size, vd->grab_size);
	#endif

	
	for(frame=0; frame < vd->grab_vm.frames; frame++) {
		vd->grab_buf.frame = frame;
		if(ioctl(vd->grab_fd, VIDIOCMCAPTURE, &(vd->grab_buf))<0) {
			perror("VIDIOCMCAPTURE");
			exit(-1);
		}
	}

	frame = 0;
#ifdef BENCH
	time(&ini);
#endif
	while(1) {
		i = -1;
		 /* waits for the frame sync */
                 while ((ioctl(vd->grab_fd, VIDIOCSYNC, &frame)) < 0) {
                         #ifdef BENCH
			 fprintf(stderr, "Waiting frame: %d\n", frame);
                         #endif
			 }
		 /* refer the frame */
		 
		 vd->frame=frame;
		 vd->image=vd->grab_data + vd->grab_vm.offsets[frame];
		 //vd->image=vd->grab_data;
		 if (vd->w == 3 && vd->grab_data) {
		 	
			/* original or faster swap function */
			#ifdef BGR2RGB
			bgr2rgb(vd->image, vd->grab_buf.width * vd->grab_buf.height);
			#else 
		 	
			swap_rgb24(vd->image, vd->grab_buf.width * vd->grab_buf.height);
			#endif

			#ifdef DEBUG
			fprintf (stderr,"rgb swap for frame (%d)\n", frame);
			fprintf (stderr,"image addr %p\n", vd->image);
			fprintf (stderr,"offset %d \n", vd->grab_vm.offsets[frame]);
		        #endif
		 }
				
		 // change channel. it needs to keep each channel
		 // properties
		 if (vd->cycleinput) cycle_channel(vd);
		 
		
		/* HOOK for effects */
		
		
		/* save it */
		
		#ifdef MOTION
		/* test for motion */
		 _cur_image.image=vd->image;
		 _cur_image.x=vd->x;
		 _cur_image.y=vd->y;
		 _cur_image.depth=vd->w;

		 avg=detect_motion_mask(&_ref_image, &_cur_image, &mask, 10); // our tolerance at +-10 pixels
		 //avg=subtract_motion(&_ref_image, &_cur_image, 50); // our tolerance at +-10 pixels

		 //fprintf(stderr,"Average changes: %d ", avg);
		 //if (_ref_image.left > _cur_image.right ) fprintf (stderr, " - left (l: %d r: %d) ", _ref_image.left, _cur_image.right);
		 //if (_ref_image.right > _cur_image.left ) fprintf (stderr, " - right (l: %d r: %d) ", _cur_image.left, _ref_image.right);
		 //fprintf(stderr,"\r");

	#ifdef STAMP
	/* initial tests for the timestamp function */
	time (&t);
	tm = localtime (&t);
	if (vd->stamp) {	
		// validation
		/* if (vd->stamp_scale_w <= 0) vd->stamp_scale_w=1;
		if (vd->stamp_scale_h <= 0) vd->stamp_scale_h=1;
		*/
		strftime (stringo, sizeof (stringo), vd->stamp, tm);
		rgb24_printf(vd->stamp_pos_x, vd->stamp_pos_y, vd->stamp_scale_w, vd->stamp_scale_h, vd, "%s ->  %d", stringo, vd->channel);
	}
	
	#endif
		// threshold
		//threshold (vd, 127);
		//median_filter(vd);
		//mark_areas(vd);
		 // sensibility 
//		 if (avg > ((vd->grab_size/vd->w)/10)) loop_img(vd);  // saves when there are more than ~5% of elements have changed.
				
		 #endif
		loop_img(vd);

		 /* Once buf is no longer in use by the application... */

		 vd->grab_buf.frame = frame;
		 if(ioctl(vd->grab_fd, VIDIOCMCAPTURE, &(vd->grab_buf))<0) {
		 	perror("VIDIOCMCAPTURE");
			exit(-1);
		 }
		 
		 /* next frame */
		 frame++;
		 /* reset to the 1st frame */
                 if (frame>=vd->grab_vm.frames) {
		 	#ifdef DEBUG
			fprintf(stderr,"Max buffers %d\n", frame -1);
			#endif
			frame = 0;
		}
#ifdef BENCH
      		 if (a == 30) {
		 	time (&fim);
			fprintf (stderr, "30 frames/%lu sec\n", (unsigned long)fim -ini);
			a=0;
			ini=fim;
		 } else a++;
#endif
	} // loop
					 
}

