/* 
 * cc select_test.c -o sel_test -lmysqlclient -I/usr/local/include/mysql/ -L/usr/local/lib/mysql/
 */

#include <stdio.h>
#include <stdlib.h>
#include "mysql.h"

#define SELECT_QUERY "select * from tbMyTable where Id=\'%s\'"
#define DBHOST		localhost
#define DBUSER		root		/* bad ideia */
#define	DBPASSWD	elmer
#define	DBNAME		mydb

unsigned char *sed_o(unsigned char *wot) {		/* s/\r/ /g */

	while (*wot !=0) {
		if (*wot=='\r') *wot=' ';
		wot++;
		}
	return wot;
	}

int main(int argc, char **argv) {
  int	count, num;
  MYSQL mysql,*sock;
  MYSQL_RES *res;
  MYSQL_ROW row;
  unsigned int num_fields;
  unsigned int i;
  

  char	qbuf[160];

	if (argc <2) {
		fprintf (stderr, "./ %s id_cidade \n", argv[0]);
		exit(1);
		}

  	mysql_init(&mysql);
  	if (!(sock = mysql_real_connect(&mysql,DBHOST, DBUSER, DBPASSWD, DBNAME,0,NULL,0))) {
    		fprintf(stderr,"Couldn't connect to engine!\n%s\n\n",mysql_error(&mysql));
    		perror("");
    		exit(1);
  		}

	count = 0;
  	num = 3; 

	sprintf(qbuf,SELECT_QUERY, argv[1]);
        if(mysql_query(sock,qbuf)) {
		fprintf(stderr,"Query failed (%s)\n",mysql_error(sock));
		exit(1);
		}

	if (!(res=mysql_store_result(sock))) {
        	fprintf(stderr,"Couldn't get result from %s\n", mysql_error(sock));
	        exit(1);
		}
	
	num_fields = mysql_num_fields(res);
	
	while ((row = mysql_fetch_row(res))) {
		unsigned long *lengths;
	      	lengths = mysql_fetch_lengths(res);
	        for(i = 0; i < num_fields; i++){
			if (row[i]) sed_o(row[i]);

			printf("[%.*s] ", (int) lengths[i], row[i] ? row[i]: "NULL");
			}
		printf("\n");
		}



  	mysql_close(sock);
  	return 0;					
	}
