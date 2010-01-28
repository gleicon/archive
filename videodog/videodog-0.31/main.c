/* 
 * main.c 
 * Reads the command line options, set the global flags
 * Starts the main loop
 */

#include "main.h"

int main (int argc, char **argv) {
	struct vd_video videodog;


	signal (SIGINT, _sighandler);

	if (argc > 1) {
		command_line (&videodog, argc, argv, &(videodog.v_flags));
	} else {
		#ifdef DEBUG
		fprintf(stderr,"read_config\n");
		#endif
		read_config (&videodog, &(videodog.v_flags));

	}
	

	grab_init(&videodog);
	set_picture(&videodog);
	//if (videodog.channel != 99) set_channel(&videodog);
	set_channel(&videodog);
	set_mode(&videodog);
	
	update_cap(&videodog);
	
	if (videodog.v_flags.showc) { 
		#ifdef DEBUG
		fprintf(stderr,"\n\nvideodog.v_flags.showc=%d\n", videodog.v_flags.showc);
		#endif
		show_cap(&videodog);
		if (videodog.v_device) free (videodog.v_device);
		if (videodog.v_mode) free (videodog.v_mode);
		if (videodog.v_filename) free (videodog.v_filename);
		if (videodog.stamp) free (videodog.stamp);
		close (videodog.grab_fd);
		munmap(videodog.grab_data, videodog.grab_size);
		return 0;
	}
		
	/* capture */
	if (!videodog.v_flags.nograb) {
		if (videodog.v_flags.singleframe) grab_one(&videodog);
		else grab_loop(&videodog);
	}
	/* end */

	/* frees all the malloc`ed members */
#ifdef DEBUG
fprintf (stderr,"\nfree`ing \n");
#endif
		if (videodog.v_device) free (videodog.v_device);
		if (videodog.v_mode) free (videodog.v_mode);
		if (videodog.v_filename) free (videodog.v_filename);
		if (videodog.stamp) free (videodog.stamp);
		close (videodog.grab_fd);
		munmap(videodog.grab_data, videodog.grab_size);
	return 0;
} /* main */


