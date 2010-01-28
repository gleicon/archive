/*
 * a small test program to change xterm title on-the-fly.
 * based in a text I read somewhere
 * set your TERM env to xterm, xterm-color, etc. 
 * 
 * gleicon - 99
 *
 * compile:
 * cc xtset.c -o xtset (or whatever name you want)
 * strip that_name
 *
 * P.S: its worked ok for my xterm, rxvt, aterm etc...
 *
 */
 

#include <stdio.h>

int main (int argc, char **argv) {
	int numbah;
	
	if (argc  < 2 ) {
		fprintf (stderr,"Usage: %s  [string to put in title bar] \ne.g.:  %s Hello\n       %s Hello world !\n", 
			 argv[0],argv[0],argv[0]);
		return;
		}
	fprintf (stdout, "%c]0;",'\033');
	
	if (!strncmp (getenv("TERM"), "xter",4)) {
		for (numbah=1;numbah < argc;numbah++) 
				fprintf (stdout,"%s ", argv[numbah]);
		fprintf (stdout,"%c",'\007');
		}
	else {
		fprintf (stderr,"This term doesn't appears to be a xterm\n");
		exit(1);
		fflush (stdout);
		}
	}
