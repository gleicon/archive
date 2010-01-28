/* cc mpegtuna.c -o mpegtuna -O2 -lmpeg
 * records frame by frame from a mpeg1 file to pnm
 * mpeg part based on 
 * easympeg by Greg Ward, 94/8/15
 */

#include <stdlib.h>
#include <errno.h>
#include <mpeg.h>

void _save_test_pnm (unsigned char *buffer, int wido, int heio, unsigned long si) {

        static int indo=0;
	FILE *fp;
	char bewf[128];

	sprintf (bewf, "image-%d.pnm", indo);
	indo++;
	if ((fp = fopen (bewf, "w")) == NULL) return;
	fprintf (fp, "P5\n%d %d\n255\n", wido, heio); // grayscale
	fwrite (buffer, si, 1, fp);
	fclose (fp);
	}
	
			
int main (int argc, char **argv){
   FILE       *mpeg;
   ImageDesc   img;
   char       *pixels;
   Boolean     moreframes = TRUE;
   Boolean     full_color = FALSE; // era TRUE
   Boolean     loop = TRUE;
   int framo=0;

   if (argc != 2) {
      fprintf (stderr, "Usage: %s mpegfile\n", argv[0]);
      exit (1);
   }

   mpeg = fopen (argv[1], "rb");
   if (!mpeg)  {
      perror (argv[1]);
      exit (1);
   }
      
   SetMPEGOption (MPEG_DITHER, 
		  (full_color) ? (int) FULL_COLOR_DITHER : 
		                 (int) ORDERED_DITHER);
   if (!OpenMPEG (mpeg, &img))   {
      fprintf (stderr, "OpenMPEG on %s failed\n", argv[1]);
      exit (1);
   }

   pixels = (char *) malloc (img.Size * sizeof(char));
   fprintf (stderr,"Movie is %d x %d pixels (%d bits depth) \n", img.Width, img.Height, img.PixelSize);
   fprintf (stderr, "Required picture rate = %d, required bit rate = %d\n",
	   img.PictureRate, img.BitRate);

   
   
      while (moreframes) {	/* play frames until the movie ends */
      
	 moreframes = GetMPEGFrame (pixels);
	 fprintf (stderr, "frame: %d\n", framo++);
	 _save_test_pnm	(pixels, img.Width, img.Height, img.Width * img.Height * (img.PixelSize/8));

      }

      RewindMPEG (mpeg, &img);
      moreframes = TRUE;
   
}
