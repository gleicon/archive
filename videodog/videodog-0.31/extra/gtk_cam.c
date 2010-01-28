/*
    cc `gtk-config --cflags` -o gtk_cam gtk_cam.c `gtk-config --libs` `gdk-pixbuf-config --libs` `gdk-pixbuf-config --cflags`

*/



#include <strings.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <linux/videodev.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <gtk/gtk.h>

#define DOT	fprintf (stderr,".");

void swap_rgb24(unsigned char *mem, int n) {
	unsigned char  c;
	unsigned char *p = mem;
	int   i = n ; 
	while (--i) {
		c = p[0];
		p[0] = p[2];
		p[2] = c;
		p += 3;
	}
}               
														
int blend_images(unsigned char *from, int from_w, int from_h, int from_depth, 
		unsigned char *to, int to_w, int to_h, int to_depth, 
		int off_x, int off_y, unsigned char alpha);

void bgr2rgb(unsigned char *b, unsigned int tam) {
        unsigned char temp;
        while (tam) {
		temp=*b;
		*b=*(b+2);
		*(b+2)=temp;
		b+=3;
		tam--;
		}
}
													
#define COLS    320
#define ROWS    240

//#define COLS    640
//#define ROWS    480
#define DEPTH  3 


struct video_picture      grab_pic;
struct video_capability   grab_cap;
struct video_channel      grab_vid; /* escolher canal e NTSC */
struct video_tuner       grab_tun; /* escolher canal e NTSC */
struct video_mmap         grab_buf;
struct video_mbuf         grab_vm;
unsigned int              grab_fd, grab_size, frame, i, done=1;
static unsigned char  *grab_data= NULL, *buf=NULL, *logo_image;


GtkWidget *status;
GtkWidget *darea;
GtkWidget *label;
int global_frames=0;
gint context_id;

void send_expose(void);
void on_darea_expose (GtkWidget *widget);
int load_pnm(char *, unsigned char *);
void brilho (unsigned char *);

void open_dev (void) {
        

	grab_size= ROWS * COLS * DEPTH;
	
	if ((grab_fd = open("/dev/video0",O_RDWR)) == -1 ) {
		perror("open videodev");
		exit(-1);
	}

	if (ioctl(grab_fd,VIDIOCGCAP,&grab_cap) == -1) {
		fprintf(stderr,"wrong device\n");
		exit(-1);
	}
	
	/* test */
	if (ioctl(grab_fd,  VIDIOCGCHAN, &grab_vid)==-1){
		perror("VIDIOCGCHAN");
	}
	grab_vid.norm=VIDEO_MODE_NTSC;
	grab_vid.channel=0; // Input channel


	if (ioctl(grab_fd,  VIDIOCSCHAN, &grab_vid)==-1){
		perror("VIDIOCSCHAN");
	}
	
        /* Query the actual buffers available */
        if(ioctl(grab_fd, VIDIOCGMBUF, &grab_vm) < 0) {
        	perror("VIDIOCGMBUF");
		exit(-1);
	}
        
	grab_buf.frame  = 0; 
        grab_buf.width  = COLS; 
        grab_buf.height = ROWS; 
        grab_buf.format = VIDEO_PALETTE_RGB24; 
	
        /* MMap all available buffers */
        grab_data = mmap(0, grab_vm.size, PROT_READ|PROT_WRITE, MAP_SHARED, grab_fd, 0);
    	
	if (grab_data==(unsigned char *)-1) {
        	perror("mmap");
		exit(-1);
	}
					    
				
        fprintf (stderr,"buffers: %d - size: %d - grab_size: %d\n", grab_vm.frames, grab_vm.size, grab_size);

        if (ioctl(grab_fd, VIDIOCGTUNER, &(grab_tun)) == -1) {
                   perror("VIDIOCGTUNER");
                   return;
        }
          grab_tun.flags = VIDEO_TUNER_NTSC;
	  grab_tun.mode = VIDEO_MODE_NTSC;
			  
	if (ioctl(grab_fd, VIDIOCSTUNER, &(grab_tun)) == -1) {
                   perror("VIDIOCSTUNER");
                   return;
        }
	

	if (ioctl (grab_fd, VIDIOCGPICT, &grab_pic) < 0) {
	  	perror("VIDIOCGPICT");
	}


	// picture.

	if (ioctl (grab_fd, VIDIOCSPICT, &grab_pic) < 0) {
	  	perror("VIDIOCSPICT");
	}
								  
	

}



/* Obligatory basic callback */
static void print_hello(GtkWidget * w, gpointer data) {
    gtk_statusbar_push (GTK_STATUSBAR (status), 1, "menu opt");
    g_message("Hello, World!\n");
}

void file_ok_sel( GtkWidget*w,  GtkFileSelection *fs ){
	
	g_print ("%s\n", gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs)));

}

void open_file(GtkWidget *widget, gpointer *data) {
	GtkWidget *filew;
	
	filew = gtk_file_selection_new ("File selection");
//	gtk_signal_connect (GTK_OBJECT (filew), "destroy",
//	                   (GtkSignalFunc) destroy, &filew);
	gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (filew)->ok_button),
	               "clicked", (GtkSignalFunc) file_ok_sel, filew );
	
	gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION
				  (filew)->cancel_button),
				  "clicked", (GtkSignalFunc) gtk_widget_destroy,
				  GTK_OBJECT (filew));
	gtk_widget_show(filew);
}
static void ok_about (GtkWidget *widget, gpointer window) {
	gtk_widget_destroy (GTK_WIDGET (window));
}


static void ok_cb (GtkWidget *widget, gpointer window) {
	gchar *entry_text;
	
	entry_text = gtk_entry_get_text(GTK_ENTRY(label));
	gtk_statusbar_push (GTK_STATUSBAR (status), 1, entry_text);
	gtk_widget_destroy (GTK_WIDGET (window));
}


static void ok_cancel (GtkWidget *widget, gpointer window) {
	gtk_widget_destroy (GTK_WIDGET (window));
}

static void process_entry (GtkWidget *widget, gpointer *entry) {
	gchar *entry_text;
	entry_text = gtk_entry_get_text(GTK_ENTRY(entry));
	gtk_statusbar_push (GTK_STATUSBAR (status), 1, entry_text);
}

static void about (GtkWidget *wido, gpointer *data) {
	GtkWidget *win, *vbox, *button, *hbox, *label;
	
	win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (win), "About");
	gtk_widget_set_usize(GTK_WIDGET(win), 200, 90);
	gtk_widget_realize (win);
	
	vbox= gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (win), vbox );
	gtk_widget_show(vbox);

	label = gtk_label_new ("This is a Normal label\nGleicon 2001");
	gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, FALSE, 0);
	gtk_widget_show(label);
	
	
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, FALSE, 0);
	gtk_widget_show (hbox);
	
	button=gtk_button_new_with_label ("Ok");
	gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, FALSE, 10);
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	                   (GtkSignalFunc) ok_about, (gpointer) win);
	gtk_widget_show (button);
	
	gtk_widget_show(win);
	
}


static void effect (GtkWidget *wido, gpointer data) {
	GtkWidget *wida, *button1, *button2, *hbox, *vbox, *frame;

	wida= gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (wida), "ask-o");
    	gtk_widget_set_usize(GTK_WIDGET(wida), 200, 90);
	gtk_widget_realize (wida);

	vbox= gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (wida), vbox );
	gtk_widget_show(vbox);

	frame = gtk_frame_new (" Enter parameters ");

	label= gtk_entry_new ();
	gtk_container_add (GTK_CONTAINER (frame), label);
	gtk_signal_connect (GTK_OBJECT (label), "activate", 
			   (GtkSignalFunc) process_entry, (gpointer) label);
	
	gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, FALSE, 0);
	gtk_widget_show(frame);
	
	
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, FALSE, 0);
	gtk_widget_show (hbox);

	
	button1=gtk_button_new_with_label ("Ok");
	gtk_box_pack_start (GTK_BOX (hbox), button1, TRUE, FALSE, 10);
	gtk_signal_connect (GTK_OBJECT (button1), "clicked",
	                   (GtkSignalFunc) ok_cb, (gpointer) wida);
	gtk_widget_show (button1);
		
	button2=gtk_button_new_with_label ("Cancel");
	gtk_box_pack_start (GTK_BOX (hbox), button2, TRUE, FALSE, 10);
	gtk_signal_connect (GTK_OBJECT (button2), "clicked",
	                   (GtkSignalFunc) ok_cancel, (gpointer) wida);
	gtk_widget_show (button2);
	
	gtk_widget_show(wida);

	
}

static void change_input (GtkWidget *wido, gpointer data) {
	char buff[25];
	
	if (ioctl (grab_fd, VIDIOCGCHAN, &grab_vid) == -1) {
		fprintf(stderr,"Erro VIDIOCGCHAN\n");
		exit(0);
	}
	
	grab_vid.norm=1; // NTSC

	grab_vid.channel=(gulong *) data;
	g_snprintf(buff, 20, "Input %d", grab_vid.channel);
	
	fprintf (stderr,"Input %d\n", grab_vid.channel);
    	gtk_statusbar_push (GTK_STATUSBAR (status), context_id, buff);

	if (ioctl (grab_fd, VIDIOCSCHAN, &grab_vid) == -1) {
		fprintf(stderr,"Erro VIDIOCSCHAN\n");
		exit(0);
	}

}


/* This is the GtkItemFactoryEntry structure used to generate new menus.
Item 1: The menu path. The letter after the underscore indicates an
accelerator key once the menu is open.
Item 2: The accelerator key for the entry
Item 3: The callback function.
Item 4: The callback action.  This changes the parameters with
which the function is called.  The default is 0.
Item 5: The item type, used to define what kind of an item it is.
Here are the possible values:
NULL               -> "<Item>"
""                 -> "<Item>"
"<Title>"          -> create a title item
"<Item>"           -> create a simple item
"<CheckItem>"      -> create a check item
"<ToggleItem>"     -> create a toggle item
"<RadioItem>"      -> create a radio item
<path>             -> path of a radio item to link against
"<Separator>"      -> create a separator
"<Branch>"         -> create an item to hold sub items (optional)
"<LastBranch>"     -> create a right justified branch 
*/

static GtkItemFactoryEntry menu_items[] = {
    {"/_File", NULL, NULL, 0, "<Branch>"},
    {"/File/_New", "<control>N", print_hello, 0, NULL},
    {"/File/_Open", "<control>O", open_file, 0, NULL},
    {"/File/_Save", "<control>S", print_hello, 0, NULL},
    {"/File/Save _As", NULL, NULL, 0, NULL},
    {"/File/sep1", NULL, NULL, 0, "<Separator>"},
    {"/File/Quit", "<control>Q", gtk_main_quit, 0, NULL},
    {"/_Options", NULL, NULL, 0, "<Branch>"},
    {"/Options/Test", NULL, effect, 0, NULL},
    {"/Options/Input 0", NULL, change_input, 0, NULL},
    {"/Options/Input 1", NULL, change_input, 1, NULL},
    {"/Options/Input 2", NULL, change_input, 2, NULL},
    {"/Options/Input 3", NULL, change_input, 3, NULL},
    {"/_Help", NULL, NULL, 0, "<LastBranch>"},
    {"/_Help/About", NULL, about, 0, NULL},
};


void get_main_menu(GtkWidget * window, GtkWidget ** menubar) {
    GtkItemFactory *item_factory;
    GtkAccelGroup *accel_group;
    gint nmenu_items = sizeof(menu_items) / sizeof(menu_items[0]);

    accel_group = gtk_accel_group_new();

    /* This function initializes the item factory.
       Param 1: The type of menu - can be GTK_TYPE_MENU_BAR, GTK_TYPE_MENU,
       or GTK_TYPE_OPTION_MENU.
       Param 2: The path of the menu.
       Param 3: A pointer to a gtk_accel_group.  The item factory sets up
       the accelerator table while generating menus.
     */

    item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>",
					accel_group);

    /* This function generates the menu items. Pass the item factory,
       the number of items in the array, the array itself, and any
       callback data for the the menu items. */
    gtk_item_factory_create_items(item_factory, nmenu_items, menu_items,
				  NULL);

    /* Attach the new accelerator group to the window. */
    gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

    if (menubar)
	/* Finally, return the actual menu bar created by the item factory. */
	*menubar = gtk_item_factory_get_widget(item_factory, "<main>");
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *main_vbox;
    GtkWidget *menubar;
    GtkWidget *vbox;

 
    g_thread_init(NULL);
    gtk_init(&argc, &argv);
    gdk_init (&argc, &argv);
    gdk_rgb_init();
	

  //  buf= (unsigned char *)malloc (ROWS * COLS * DEPTH * sizeof (unsigned char));
    //if (!buf) {
    //	perror ("malloc buf");
//	exit(0);
  //  }

    open_dev(); /* abre e inicializa video */
    

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_signal_connect(GTK_OBJECT(window), "destroy",
		       GTK_SIGNAL_FUNC(gtk_main_quit), "WM destroy");
    
    gtk_window_set_title(GTK_WINDOW(window), "NutoScope");
    //gtk_widget_set_usize(GTK_WIDGET(window), COLS, ROWS);

    main_vbox = gtk_vbox_new(FALSE, 1);
    gtk_container_border_width(GTK_CONTAINER(main_vbox), 1);
    gtk_container_add(GTK_CONTAINER(window), main_vbox);
    gtk_widget_show(main_vbox);

    get_main_menu(window, &menubar);
    gtk_box_pack_start(GTK_BOX(main_vbox), menubar, FALSE, TRUE, 0);
    gtk_widget_show(menubar);
    

    darea = gtk_drawing_area_new();
    gtk_box_pack_start (GTK_BOX (main_vbox), darea, FALSE, FALSE, 0);
    gtk_signal_connect (GTK_OBJECT (darea), "expose-event", GTK_SIGNAL_FUNC (on_darea_expose), (gpointer) darea);
    gtk_drawing_area_size (GTK_DRAWING_AREA (darea), COLS, ROWS);
    gtk_widget_show(darea);

    status = gtk_statusbar_new ();
    gtk_box_pack_start (GTK_BOX (main_vbox), status, FALSE, FALSE, 0);
    context_id = gtk_statusbar_get_context_id(
                              GTK_STATUSBAR(status), "Ready");

    gtk_widget_show (status);

    
    
    gtk_widget_show(window);
    

    // load 50x50x3 logo
    logo_image=malloc(50*50*3);
    if (!logo_image) {
    	perror("malloc logo ");
	exit;
    }
    load_pnm("logo.pnm", logo_image);
    // Queue frames

	for(frame=0; frame<grab_vm.frames; frame++) {
		grab_buf.frame = frame;
		if(ioctl(grab_fd, VIDIOCMCAPTURE, &grab_buf)<0) {
			perror("VIDIOCMCAPTURE");
			exit(-1);
		}
	}
    
   
    gtk_timeout_add(25, send_expose, NULL);
   
    gdk_threads_enter();
    gtk_main();
    gdk_threads_leave();

    return (0);
}

void send_expose(void) {
	static int frame, running, num;
	
	if (running) return;
	fprintf(stderr,"Capturing frame: %d ", frame);
	running=1;
	
	while ((ioctl(grab_fd, VIDIOCSYNC, &frame)) < 0) {
         	fprintf(stderr, "Waiting frame: %d\n", frame);
	}

	/* refer the frame */
	buf = grab_data + grab_vm.offsets[frame];
	blend_images(logo_image, 50, 50, 3, buf, COLS, ROWS, DEPTH, 150, 80, 180);
	
	gdk_threads_enter();
	on_darea_expose(darea); // exibe
	gdk_threads_leave();
	
// test
	//grab_buf.frame = (frame==0)?1:0;
	grab_buf.frame = frame;
        if(ioctl(grab_fd, VIDIOCMCAPTURE, &grab_buf)<0) {
		perror("VIDIOCMCAPTURE");
		exit(-1);
	}
	fprintf(stderr,"Queing frame: %d\n ", frame);
	frame++;
	if (frame>=grab_vm.frames) frame = 0;	
	running=0;
}

void on_darea_expose (GtkWidget *widget) {
	
	GdkRectangle rect_oi;
//	static time_t init, end;
//	static frames;
//	unsigned char title[20];

//	if (!frames) {
//		time (&init);
//	}
	
	if (!buf) {
		return;
	}
	//brilho(buf);
	
	//rect_oi.x
	//rect_oi.y
//	rect_oi.width=COLS;
//	rect_oi.height=ROWS;
//	bgr2rgb(buf, COLS*ROWS);
	
//	gdk_threads_enter();
	swap_rgb24(buf, grab_size);
	gdk_draw_rgb_image (widget->window, 
			    //widget->style->white_gc, 0, 0, 
			    widget->style->fg_gc[GTK_WIDGET_STATE(widget)], 0, 0, 
			    COLS, ROWS, GDK_RGB_DITHER_NORMAL, 
			    buf, COLS * DEPTH);
	
//	gtk_widget_draw(widget, &rect_oi);
//	gdk_threads_leave();
	
//	frames++;
//	time (&end);
//	if (end - init >= 1) {
//		global_frames=frames;
//		frames=0;
  //  		sprintf(title, "FPS %d - %d", global_frames, end - init);
//		gtk_statusbar_push (GTK_STATUSBAR (status), 1, title);
//		fprintf(stderr,"%s\n", title);
//	}

}



int load_pnm(char *wot, unsigned char *bufo){
        FILE *fp;
        int x, y, m;
        unsigned char ui[4];
        if (!bufo) {
               fprintf(stderr,"Buffer is null \n");
               exit(0);
        }
        if (!(fp=fopen(wot, "rb"))) {
                perror("file");
                exit(0);
        }
										        fscanf(fp, "%s\n%d %d\n%d\n", ui, &x, &y, &m); // header
        fread(bufo, x * y * DEPTH, 1, fp);
        fprintf(stderr,"X: %d\nY: %d\n",  x , y );
        fclose(fp);
}
																		
void brilho (unsigned char *buf) {

	int a;
	for (a=0; a< ROWS * COLS * DEPTH; a++) buf[a]=buf[a]+10;
}







/* 
 *	alpha blend 
 *	(c) gleicon - 2003 
 *
 *	*_from - smallest image
 *	*_to - destination image
 *	alpha - 0 to 255 (0 = opaque, 255 almost invisible)
*/

/* blends 2 pixels using alpha factor */

int alphablend (unsigned char p1, unsigned char p2, unsigned char alpha ) {
	unsigned char diff ;
	float alphaFactor = alpha/255.0;
	unsigned char value;
	
	diff = (p1>p2)?p1-p2:p2-p1;
	
	if (diff == 0) return p1;
	
	diff = (unsigned char)(diff * alphaFactor);
//	if (p1 == 255) return p2 + 20; // test invisible layer
	if (p1>p2) value = p1 - diff;
	else value = p1 + diff;
	
	return value;
}


int blend_images(unsigned char *from, int from_w, int from_h, int from_depth, 
		unsigned char *to, int to_w, int to_h, int to_depth, 
		int off_x, int off_y, unsigned char alpha){

	       int a, b, from_size, to_size, sub_pixel;
	       unsigned char *from_pix, *to_pix;

		if (to_depth != from_depth) {
			// unmatched depths
			return -1;
		}
		if (from_w > to_w || from_h > to_h){
			// "from" image is bigger than "to"
			return -2;
		}
		if (((off_x + from_w) > to_w) || ((off_y + from_h) > to_h)) {
			// offset out of range
			return -3;
		}

		from_size=from_w*from_h* from_depth;
		to_size=to_w*to_h* to_depth;

		for (a=0; a<from_h; a++) { //height
			for (b=0; b<from_w * from_depth; b+=from_depth) { // width
				for (sub_pixel=0; sub_pixel < to_depth; sub_pixel++) {
					from_pix= from + (a * from_w * from_depth) + b + sub_pixel;
					to_pix= to + ((a+off_y) * to_w * to_depth) + (b + off_x)+ sub_pixel ;
					//to_pix= to + b + sub_pixel + (a * to_w * to_depth) ;
					*(to_pix)=alphablend(*(from_pix), *(to_pix), alpha);
				}
			}
		}

		
	

}
