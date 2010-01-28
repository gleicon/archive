/* Jpeg compressor
 * Gleicon 	-	05/10/98
 */

#include <stdio.h>
#include <jpeglib.h>
#include <stdlib.h>
#include <error.h>


#define JPEG_QUALITY 50 
#define HEIGHT 1024
#define WIDTH 1024
#define DEPTH 1

int x,y;
char buf[10];
extern int errno;


int write_jpeg(char *filein, char *fileout, int width, int height, int depth) {
	
	struct jpeg_compress_struct cinfo;
        struct jpeg_error_mgr jerr;
        FILE *infile, *outfile;
        int i;
        unsigned char *line;
                    
        if ((outfile = fopen(fileout,"w+"))==NULL) {
        	fprintf(stderr,"can't open %s \n", filein);
        	perror ("fopen");
        	return -1;
                }
                    
        if ((infile = fopen(filein,"r+"))==NULL) {
        	fprintf(stderr,"can't open %s \n", fileout);
          	perror ("fopen");
   	   	return -1;
                }
                                                          
	line = malloc ((width * depth) * sizeof (char)); // linha
	
	if (!line) {
		fprintf (stderr," Erro ao alocar (line) \n");
		return -1;
		}
	
	fscanf (infile, "%s\n%d %d %s", buf, &x, &y, buf);
	fprintf (stderr, "%s %d %d %s", buf, x, y, buf);
	width=x;
	height=y;
		
	
	cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);
        jpeg_stdio_dest(&cinfo, outfile);
	cinfo.image_width = width;
       	cinfo.image_height = height;
	            
	cinfo.input_components = depth;
	
	if (depth == 1) { 
		cinfo.in_color_space = JCS_GRAYSCALE;
		}
	else {
		cinfo.in_color_space = JCS_RGB;
		}
		
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, JPEG_QUALITY, TRUE);
	jpeg_start_compress(&cinfo, TRUE);
	
		        
	                                   
	for (i = 0; i < height; i++) {
	     
	//     fprintf (stderr,"linha: %d\n", i);   
	     memset (line, 0, (width*depth));
	     fread (line, (width*depth), 1, infile);
	     jpeg_write_scanlines(&cinfo, &line, 1);
	     }
	     
	                                    
	jpeg_finish_compress(&(cinfo));
	jpeg_destroy_compress(&(cinfo));
	
	free (line);
	
	fclose (infile);
	fclose (outfile);
		                                                       
	return 0;
	}

void main (int argc, char **argv) {
	
	if (argc < 3) {
		fprintf (stderr, "Numero de parametros insuficiente !\n");
		return -1;
		}
	
	write_jpeg(argv[1], argv[2], WIDTH, HEIGHT, DEPTH);
	
	}

	
	                                                                                                          
