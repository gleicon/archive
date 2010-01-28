/* 
 *  if_stat.c - checa status de uma interface.
 *  		compara a leitura anterior com a atual 
 *		para marcar chegada e envio de pacotes.
 */ 
 
 #include <stdio.h>
 #include <string.h>
  
 #define PROC_PATH	"/proc/net/dev"	/* path do arquivo dev	(2.2.x)	*/
 #define MAXLEN		255 		/* tamanho maximo de cada linha */
 #define INTERFACE	"ppp0"		/* interface a monitorar	*/
 
 
 int check_stat(); /* prototipo */
 void parse_buf (char *buf, char **args); 
 
 void main () {
 	
 	char **args;
 	char buffer[256];
 	int a=0;
 	/* simple loop. */
 	//while (TRUE) 	check_stat();
 	find_line(buffer, INTERFACE);
 	parse_buf (buffer, args);
 	while (args[a] != NULL) {
 		fprintf (stdout, "\n%s", args[a]);
 		a++;
 		}
 	}
 
 int find_line(char *buffer, char *interface) { 	/* abre o arquivo, acha a linha , copia e fecha arquivo */
	
	FILE *fp;
//	char buffer[256];
	
	if ((fp=fopen(PROC_PATH, "r"))==NULL) return (-1); /* erro ao abrir arquivo */
	
	/* pula 2 linhas ( header ) */
	fgets (buffer, MAXLEN, fp);
	fgets (buffer, MAXLEN, fp);
	
	while (!feof(fp)) {
		fgets (buffer, MAXLEN, fp);
		if (strstr (buffer, interface)) {
			fclose (fp);
			fprintf (stdout,buffer);
			break;
			}
		}
	}

	
 void parse_buf (char *buf, char **args) {   /* divide a linha no estilo de argv[] */
 	
 	
 	while ( *buf != 0) {
  		while ((*buf == ' ') || (*buf == '\t')) *buf++ = 0;
 		*args++=buf;
 		while ((*buf != 0) && (*buf != ' ') && (*buf !='\t')) buf++;
 		}
 	*args=NULL;
 	}
	 
 	
	
	
		 
 
 