#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PER_LINE 20



void dump (unsigned char *buffer, int bufsize) {

	int a, b, c;
	
	for (a=0; a <= bufsize ; a++ ) {
	
		
		if (!(a%PER_LINE) && (a) ) { 
			b=a-PER_LINE;
			for (c=b; c<a; c++) fprintf (stdout, "%c", ((buffer[c] < 32) ? '.': buffer[c]));
			fprintf (stdout, "\n");
			}
	 
		if (a!=bufsize) fprintf (stdout, "%02X ", buffer[a]);
		}
		
		if ((a%PER_LINE) !=0 ) {
			for (c=(a%PER_LINE); c <= PER_LINE; c++) fprintf (stdout,"   ");
			b=a-(a%PER_LINE);
			
			for (c=b;c <= bufsize ; c++) fprintf (stdout, "%c", ((buffer[c] < 32) ? '.': buffer[c]));
			
			for (b=(a%PER_LINE) ; b < PER_LINE ; b++) fprintf (stdout, ".");
			
			fprintf (stdout, "\n");	 
			}
	
	/* 
	   Local vars:
	   a,b,c -> temp integers
	   bufsize -> current buffer size
	   buffer  -> guess...
	 */
	 
	}

void main () {

	int fd,x,b;
	unsigned char temp[1000];
	
	fd=open ("/etc/passwd", O_RDONLY);
	
	if (fd == -1) {
		fprintf (stderr," Erro open \n");
		exit (1);
		}
	for (x=0; x < sizeof ( temp) ; x++) {
		b=read (fd, &temp[x], 1);  
		if (!b) break;
		}
	
	close (fd);
	 
	
	
	dump (temp, sizeof(temp));
	}
		
	