/* gets active and total cpu usage */
#include <stdio.h>

struct CPU {
	long user, system, nice, idle;
	float load;
	};
	

void get_stat_cpu() {
	
	struct CPU myCPU;
	float act, tot;
	char temp[128];
	
	static long last_idle;
	
	static FILE *fp_stat;
	static FILE *fp_loadavg;
	
	fp_stat= fopen ("/proc/stat", "r");
	fp_loadavg= fopen ("/proc/loadavg", "r");
	
	if ( (!fp_stat) || (!fp_loadavg) ) {
		fprintf (stderr, "Erro\n");
		exit(0);
		}
	
	while (fgets (temp, 128, fp_stat)) {
		if (strstr (temp, "cpu")) {
			sscanf (temp, "cpu %ld %ld %ld %ld", &myCPU.user, &myCPU.system, &myCPU.nice, &myCPU.idle );
			act = myCPU.user + myCPU.system + myCPU.nice;
			tot = act + myCPU.idle;
			break;
			}
		}
	fscanf (fp_loadavg , "%f", &myCPU.load);
	
	fprintf (stdout, "Active CPU  : %f\n", act);
	fprintf (stdout, "TOTAL CPU   : %f\n", tot);
	fprintf (stdout, "Load Average: %f\n", myCPU.load);

	fclose (fp_stat);
	fclose (fp_loadavg);
	
	}


void main () { get_stat_cpu (); } 
