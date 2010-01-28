/* 
 * cc gtkpnm.c -o gtkpnm `gtk-config --libs` `gtk-config --cflags` `gdk-pixbuf-config --libs` `gdk-pixbuf-config --cflags` 

 */
 

#include <unistd.h>
#include <stdio.h>
#include <glib.h>
#include <errno.h>
#include <gtk/gtk.h>

/* 
 * gleicon@terra.com.br - 07/2001
 * some gtk ideas stolen from Edgar Soter
 */

#define COLS	320
#define ROWS	240
#define DEPTH	3

GtkWidget *window, *darea;
int cam,size,i,frame,q,st;
guchar *bigbuf;
guchar *buf;



void display(GtkWidget *window, GtkWidget *darea);
void on_darea_expose (GtkWidget *widget);
int load_pnm(char *, unsigned char *);

#define DOT fprintf(stderr,".");


int main ( int argc, char **argv ) {
	
	GtkWidget *window, *darea;

	buf=malloc (ROWS * COLS * DEPTH * sizeof (unsigned char));
	if (!buf) {
		perror ("malloc buf");
		exit(0);
	}
	
	if (argc < 2) {
		fprintf(stderr, "Filename needed\nUsage: %s file.pnm\n", argv[0]);
		exit(0);
	}
	load_pnm(argv[1], buf);
	gtk_init (&argc, &argv);
	gdk_init (&argc, &argv);
	gdk_rgb_init();
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	darea = gtk_drawing_area_new(); 
	display (window, darea);
	sleep(10);
	
	free(buf);
}


void display(GtkWidget *window, GtkWidget *darea)  {
	gint x, y;
	static first = 1;
	if (first) {
		gtk_container_add (GTK_CONTAINER (window), darea); 
		gtk_signal_connect (GTK_OBJECT (darea), "expose-event", GTK_SIGNAL_FUNC (on_darea_expose), NULL);
		first = 0;
	}
	gtk_drawing_area_size (GTK_DRAWING_AREA (darea), COLS, ROWS);
	gtk_widget_show_all (window);
	gtk_main();

}
			   


void on_darea_expose (GtkWidget *widget) {
	gdk_draw_rgb_image (widget->window, widget->style->fg_gc[GTK_STATE_NORMAL], 0, 0, COLS, ROWS, GDK_RGB_DITHER_MAX, buf, COLS*3);
	gtk_main_quit();

}
								  


int load_pnm(char *wot, unsigned char *bufo){
	FILE *fp;
	int x, y, m;
	unsigned char ui[4];
	

	if (!(fp=fopen(wot, "rb"))) {
		perror("file");
		return NULL;
	}

	
	fscanf(fp, "%s\n%d %d\n%d\n", ui, &x, &y, &m); // header
	fread(bufo, ROWS * COLS * DEPTH, 1, fp); 
	
	fclose(fp);
}

