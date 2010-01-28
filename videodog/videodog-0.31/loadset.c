/* 
 * loadset.c 
 * "drivers" to the different configure types
 */

#include "loadset.h"

//#define ULTRADEBUG  // ultimate debug
//#define CONFIG_DEBUG // config read debugger


/* use getopt */

int command_line (struct vd_video *videodog, int argc, char **argv, struct vd_flags *v_flags) {
	
	int c;
	unsigned char *default_mode = "NTSC";
	
	videodog->img_format=VD_RAW;
	videodog->v_device=NULL;
	videodog->v_filename=NULL;
	videodog->v_mode=NULL;
	videodog->x = videodog->y = videodog->w=0;
 	videodog->stamp=NULL;	
	videodog->stamp_pos_x=videodog->stamp_pos_y=videodog->stamp_scale_w=videodog->stamp_scale_h=0;
	videodog->tstampname=0;
	videodog->cycleinput=0;
	videodog->channel=0; /* default channel */	

	v_flags->hue= v_flags->contrast= v_flags->brightness=v_flags->colour=-1;
	v_flags->nograb=v_flags->showc=0;
	v_flags->singleframe=1;

	while ((c = getopt (argc, argv, OPTSTR)) != EOF)
		switch (c) {
			case 'x':
				videodog->x=atoi(optarg);
				break;
			case 'y':
				videodog->y=atoi(optarg);
				break;
			case 'w':
				videodog->w=atoi(optarg);
				break;
			case 'f':
				if (!videodog->v_filename) {
					videodog->v_filename= (char *)malloc ((strlen(optarg) * sizeof(char))+1);
					memset(videodog->v_filename, 0, strlen(optarg));
					strcpy(videodog->v_filename, optarg);
					
					#ifdef DEBUG
					fprintf(stderr,"%s\n", optarg);
					#endif
				}
				break;
				
			case 'p':
				videodog->img_format=VD_PNM;
				break;
			
			case 't':
				videodog->img_format=VD_TGA_24;
				break;
			
			case 'r':
				videodog->img_format=VD_RAW;
				break;
			
			case 'd':
				if (!videodog->v_device) {
					videodog->v_device= (char *)malloc ((strlen(optarg) * sizeof(char))+1);
					if (!videodog->v_device) {
						perror("malloc v_device");
						exit(1);
						}
					memset(videodog->v_device, 0, strlen(optarg));
					strcpy(videodog->v_device, optarg);
				}
				break;
			case 'i':
				videodog->channel=atoi(optarg);
				#ifdef DEBUG
				fprintf(stderr,"channel -i=%d\n", videodog->channel);
				#endif
				if (videodog->channel==-1) 
					videodog->cycleinput=1;

				break;
			case 'm':
				videodog->v_mode= (char *)malloc ((strlen(optarg) * sizeof(char))+1);
				memset(videodog->v_mode, 0, strlen(optarg));
				strcpy(videodog->v_mode, optarg);
				break;
			
			#ifdef HASJPEG
			case 'j':
				videodog->img_format=VD_JPG;
				break;
			#endif
			
			case 'q':
				if (!videodog->v_filename) {
					videodog->v_filename= (char *)malloc (MAXLEN * sizeof(char));
					}
				v_flags->singleframe=0;
				break;
			
			case 'u':
				v_flags->hue=atoi(optarg);
				break;
			case 'c':
				v_flags->contrast=atoi(optarg);
				break;
			case 'b':
				v_flags->brightness=atoi(optarg);
				break;
			case 'l':
				v_flags->colour=atoi(optarg);
				break;
			case 'n':
				v_flags->nograb=1;
				break;
			
			case 's':
				v_flags->showc=1;
				v_flags->nograb=1;
				break;
			/* stamp */
			case 'S':
				if (strlen(optarg)) {
					videodog->stamp= (unsigned char *)malloc ((strlen(optarg) * sizeof(unsigned char)) +1);
					memset(videodog->stamp, 0, strlen(optarg));
					strcpy(videodog->stamp, optarg);
				}
				break;
			
			case 'X':
				videodog->stamp_pos_x=atoi(optarg);
				break;
			case 'Y':
				videodog->stamp_pos_y=atoi(optarg);
				break;
			case 'W':
				videodog->stamp_scale_w=atoi(optarg);
				break;
			case 'H':
				videodog->stamp_scale_h=atoi(optarg);
				break;
							
			case 'h':
			default:
				showhelp();
				break;
			
			}
	if (!videodog->x || !videodog->y || !videodog->w ) {
		v_flags->nograb=1;
		if (!v_flags->showc) { 
			fprintf(stderr," -x, -y or -w wasn`t set in grab mode\n");
			showhelp();
		}
	}
	if (!videodog->v_mode) {
		videodog->v_mode= (char *)malloc ((strlen(default_mode) * sizeof(char)) + 1);
		memset(videodog->v_mode, 0, strlen(default_mode));
		strcpy(videodog->v_mode, default_mode);
	}
        if (!videodog->v_device) {
                fprintf (stderr, " I NEED A DEVICE NAME \n");
                showhelp();
	}
	if (!videodog->v_filename && v_flags->singleframe && !v_flags->showc) {
		   fprintf (stderr, " I NEED A FILE NAME \n");
		   showhelp();
	}
	if (videodog->v_filename && !strncmp(videodog->v_filename, "+", 1)) {
		videodog->tstampname=1;	
		if (videodog->v_filename) free(videodog->v_filename);
		videodog->v_filename=malloc(MAXFNAME);
		if (!videodog->v_filename) {
			perror("malloc");
			exit(0);
		}
		#ifdef DEBUG
	        fprintf(stderr,"File: timestamp\n");
		#endif
	}		
	
	return 1;			   


} /* main */

int read_config (struct vd_video *vd, struct vd_flags *v_flags) {
	
	FILE *fp;
        char bufo[LINESIZE+1];
	unsigned char *default_mode = "NTSC";
	char * line = NULL;
	size_t len = 0;
	ssize_t read;


	#ifdef DEBUG
	fprintf(stderr,"Looking for %s ..", CONFIG_FILE);
	#endif
	vd->img_format=VD_RAW;
	vd->v_device=NULL;
	vd->v_filename=NULL;
	vd->v_mode=NULL;
	vd->x = vd->y = vd->w=0;
	//vd->channel = 99; /* just to jump over the set_channel.	*/
	vd->tstampname=0;
	vd->cycleinput=0;
	vd->channel=0; /* default channel */

	v_flags->singleframe=1; /* defaults to single frame capture */
	vd->stamp=NULL;
	v_flags->nograb=v_flags->showc=0;
	
	if ((fp=fopen(CONFIG_FILE,"rb"))==NULL) {
		#ifdef DEBUG
		fprintf(stderr, "Not found.\n");
		#endif
		fprintf(stderr,"No config files, no command line options\n");
        	fprintf(stderr,"Go check the README and the HELP, willys \n");
        	showhelp();
	}

	#ifdef CONFIG_DEBUG
	fprintf(stderr, "Found.\n");
	#endif
	
	
	while ((read = getline(&line, &len, fp)) != -1) {
		 if (line[0]!='#' && read > 2) param(line, vd, v_flags);
	}
	
	if (line) free(line);	
	fclose(fp);
	
	if (!strncmp(vd->v_filename, "+", 1)) {
        	vd->tstampname=1;	
		free(vd->v_filename);
		vd->v_filename=malloc(MAXFNAME);
		if (!vd->v_filename) {
			perror("malloc");
			exit(0);
		}
		#ifdef CONFIG_DEBUG
	        fprintf(stderr,"File: timestamp\n");
		#endif
	}		
	if (!vd->v_mode) {
		vd->v_mode= (char *)malloc ((strlen(default_mode) * sizeof(char)) + 1);
		memset(vd->v_mode, 0, strlen(default_mode));
		strcpy(vd->v_mode, default_mode);
	}
 #ifdef CONFIG_DEBUG
	fprintf (stderr,"vd struct contents:\n");
	fprintf (stderr,"vd->v_device		: %s\n", vd->v_device);
	fprintf (stderr,"vd->x			: %d\n", vd->x);
	fprintf (stderr,"vd->y			: %d\n", vd->y);
	fprintf (stderr,"vd->w			: %d\n", vd->w);
	fprintf (stderr,"vd->img_format		: %d\n", vd->img_format);
	fprintf (stderr,"vd->filename		: %s\n", vd->v_filename);
	fprintf (stderr,"vd->v_mode		: %s\n", vd->v_mode);
	
	fprintf (stderr,"vd->stamp		: %s\n", vd->stamp);
	fprintf (stderr,"vd->stamp_pos_x	: %d\n", vd->stamp_pos_x);
	fprintf (stderr,"vd->stamp_pos_y	: %d\n", vd->stamp_pos_y);
	fprintf (stderr,"vd->stamp_scale_w	: %d\n", vd->stamp_scale_w);
	fprintf (stderr,"vd->stamp_scale_h	: %d\n", vd->stamp_scale_h);
	
	fprintf (stderr,"v_flags->singleframe = %d\n", v_flags->singleframe);
	
#endif
	return 1;
}
	
void parse(char *buf, char **args) {
	char *mytemp;

	mytemp=stripspace(buf);
	if(mytemp[0] == '#') {
		*args=NULL;
		return;
	}

	while ((*buf != (long) NULL)) {
		while ((*buf == '=')   || (*buf=='#') || (*buf=='\n')) *buf++=(long) NULL;
		*args++=buf;
		while ((*buf != (long) NULL) && (*buf!='=' && *buf !='#' && *buf!='\n') ) buf++;
		}
	*args=NULL;
}

char *stripspace(char *wot) {
	while ((*wot == ' ' || *wot =='\t') && *wot != 0) {
		wot++;
	}
	return wot;
}
 char *trailspace(char *endo) {
	
	while (*endo !=0) {
		if (*endo==' ') *endo=0; 
		endo++;
	}
 
 }

int param (char *wot, struct vd_video *vd, struct vd_flags *v_flags) {
	int a=0, found=0;
	char *args[300];
	parse (wot, args);
	
	trailspace(args[0]);
	
	#ifdef CONFIG_DEBUG
	fprintf (stderr,"key= |%s|  value = |%s|\n", args[0], args[1]);
	#endif
	
	if (*args) {
		for (a=0; a < MAXKEYS ; a++) {

			if (strncasecmp(vd_configkeys[a], args[0], strlen(args[0])) == 0) {
				found=1;	
				switch (a) {
					
					case VD_DEVICE:
						if (strlen(args[1])) {
							vd->v_device= (char *)malloc ((strlen(args[1]) * sizeof(char)) +1);
							memset(vd->v_device, 0, strlen(args[1]));
							strcpy(vd->v_device, stripspace(args[1]));
							}
						break;
						
					case VD_WIDTH:
						vd->x=atoi(stripspace(args[1]));
						break;
					case VD_HEIGHT:
						vd->y=atoi(stripspace(args[1]));
						break;
					case VD_DEPTH:
						vd->w=atoi(stripspace(args[1]));
						break;
					case VD_OFORMAT:
						if (strstr(args[1], "raw")) vd->img_format=VD_RAW;
						else if (strstr(args[1], "pnm")) vd->img_format=VD_PNM;
						else if (strstr(args[1], "tga")) vd->img_format=VD_TGA_24;
      
						#ifdef HASJPEG
						else if (strstr(args[1], "jpg")) vd->img_format=VD_JPG;
      			#endif 

						else {
							found=0;
							fprintf(stderr,"Unknown image format\n");
						}
						break;
					case VD_FXCHAIN:
						fprintf(stderr,"VD_FXCHAIN: Not implemented yet\n");
						break;
					case VD_CAPTYPE:
						
						if (strstr(args[1], "loop")) {
							#ifdef CONFIG_DEBUG
							fprintf(stderr,"Loop cap\n");
							#endif
							v_flags->singleframe=0;
						}
						else if (strstr(args[1], "single")) {
							#ifdef CONFIG_DEBUG
							fprintf(stderr,"Single cap\n");
							#endif

							v_flags->singleframe=1;
						}
					 else {
							fprintf(stderr,"Unknown capture type %s\n", args[1]);
							found=1;
						}
					break;
					
					
					case VD_FILE:
						if (strlen(args[1])) {
							vd->v_filename= (char *)malloc ((strlen(args[1]) * sizeof(char)) +1);
							memset(vd->v_filename, 0, strlen(args[1]));
							strcpy(vd->v_filename, stripspace(args[1]));
							}
						break;
					case VD_ICHANNEL:
						vd->channel=atoi(args[1]);
						if (vd->channel==-1) 
							vd->cycleinput=1;
						break;
				
					case VD_MODE:
						if (strlen(args[1])) {
							vd->v_mode= (char *)malloc ((strlen(args[1]) * sizeof(char)) +1);
							memset(vd->v_mode, 0, strlen(args[1]));
							strcpy(vd->v_mode, stripspace(args[1]));
							}
					break;
				
					case ST_TEXT:
						#ifdef CONFIG_DEBUG
						fprintf(stderr,"ST_TEXT: %s\n", args[1]);
						#endif
						if (strlen(args[1])) {
							vd->stamp= (unsigned char *)malloc ((strlen(args[1]) * sizeof(unsigned char)) +1);
							memset(vd->stamp, 0, strlen(args[1]));
							strcpy(vd->stamp, stripspace(args[1]));
						}
					break;
					
					case ST_POSX:
						#ifdef CONFIG_DEBUG
						fprintf(stderr,"ST_POSX: %s\n", args[1]);
						#endif
						
						vd->stamp_pos_x=atoi(stripspace(args[1]));
						break;
					
					case ST_POSY:
						#ifdef CONFIG_DEBUG
						fprintf(stderr,"ST_POSY: %s\n", args[1]);
						#endif
						vd->stamp_pos_y=atoi(stripspace(args[1]));
						break;
					
					case ST_SCAW:
						#ifdef CONFIG_DEBUG
						fprintf(stderr,"ST_SCAW: %s\n", args[1]);
						#endif
						vd->stamp_scale_w=atoi(stripspace(args[1]));
						break;
					
					case ST_SCAH:
						#ifdef CONFIG_DEBUG
						fprintf(stderr,"ST_SCAH: %s\n", args[1]);
						#endif
						vd->stamp_scale_h=atoi(stripspace(args[1]));
						break;
						

					
					default:
						found=0;
						#ifdef CONFIG_DEBUG
						fprintf (stderr,"No index\n");
						#endif
						break;
					}
				
				//found=1;
			//	break;
			} 
		}
	} 
	
	#ifdef CONFIG_DEBUG
	if (!found && *args) fprintf (stderr,"Invalid Key or malformed line: %s\n", args[0]);
	if(!(*args)) fprintf(stderr,"Skipped remark line\n");
	#endif

	return 1;
}



