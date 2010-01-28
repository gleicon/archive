
/* 
 * just get the stdin and try to parse using , as delimiter 
 * a good pointer exercise is to improve it to parse sentences...
 *
 * gleicon (gleicon@uol.com.br)
 *
 */


#include <stdio.h>

parse (char *buf, char **args) {
	
	while (*buf != (long) NULL ) {
				
		while ((*buf == ',')) *buf++ = (long) NULL ;
			
		*args++ = buf;
					
		while ((*buf != (long) NULL) && (*buf != ',')) buf++;
		
		}
	*args = NULL;
	
	}

void main () {
	
	char buf[1024];
	char *args[64];
	
	char **debug_ptr=args;
	int debug_i=0;
	
	if (gets (buf) == NULL) {
		fprintf (stdout,"erro em gets\n");
		exit(0);
		}
	
	parse (buf, args);
	
	while (*debug_ptr != NULL) fprintf (stdout, "args[%d] - %s\n", debug_i++,*debug_ptr++);
	
	exit(0);
	}
	
	