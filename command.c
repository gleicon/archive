/* fork.c - example of a fork in a program */
/* The program asks for UNIX commands to be typed and inputted to a string */
/* The string is then "parsed" by locating blanks etc. */
/* Each command and sorresponding arguments are put in a args array */
/* execvp is called to execute these commands in child process */
/* spawned by fork() */

									  /* cc -o fork fork.c */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>


char *getusername() {

	static struct passwd *ent;
	static int cur_uid=-1;
	
	
	if (getuid()==cur_uid) { 
		return ent->pw_name;
		}
	if (cur_uid==-1) cur_uid=getuid();
	
	setpwent();
	ent=getpwuid(cur_uid);
	endpwent();
	
	if (!ent) return "(null)";
			
	return ent->pw_name;
	
	}

char *getuserdir() {

	static struct passwd *ent;
	static int cur_uid=-1;
	
	
	if (getuid()==cur_uid) { 
		return ent->pw_dir;
		//return "~";
		}
	
	if (cur_uid==-1) cur_uid=getuid();
	
	setpwent();
	ent=getpwuid(cur_uid);
	endpwent();
	
	if (!ent) {
//		fprintf (stdout, "whoo\n");
		
		return "/"; // a std dir
		}	
	return ent->pw_dir;
//	return "~";
	
	
	}

	 
char *gethost() {
	static char nome[100];	
	
	if (gethostname (nome, 12)) return "(null)";
	return nome;
	 
}
	
void init_banner() {
	fprintf (stdout,"Tosco Shell v 0.00001 - _extended_\n\n");
	}

void main () {
  
  char buf[1024];
  char *args[64];
  char pwd[128];
  
  init_banner();
  for (;;)
    {
      /*
       * Prompt for and read a command.
       */
      // user@host[pwd] : 
      printf ("%s@%s[%s]: ", getusername() , gethost(), getcwd(pwd, sizeof(pwd)) );

      if (gets (buf) == NULL)
	{
	  printf ("\n");
	  exit (0);
	}

      /*
       * Split the string into arguments.
       */
      
      parse (buf, args);

      /*
       * Execute the command.
       */
      
      execute (args);
    }
}

																																																																																	     /*
																																																																																	        * parse--split the command in buf into
																																																																																	        *         individual arguments.
																																																																																	      */
parse (buf, args)
     char *buf;
     char **args;
{
  while (*buf != (long) NULL)
    {
      /*
       * Strip whitespace.  Use nulls, so
       * that the previous argument is terminated
       * automatically.
       */
      
      while ((*buf == ' ') || (*buf == '\t')) *buf++ = (long) NULL;

      /*
       * Save the argument.
       */
      
      *args++ = buf;

      /*
       * Skip over the argument.
       */
      
      while ((*buf != (long) NULL) && (*buf != ' ') && (*buf != '\t')) buf++;
    }

  *args = NULL;
}

																																																																																																																																																										/*
																																																																																																																																																										   * execute--spawn a child process and execute
																																																																																																																																																										   *           the program.
																																																																																																																																																										 */
execute (args)
     char **args;
{
  int pid, status;

  /*
   * check exit.
   */
  
//  fprintf (stdout,"Debug: %s - %s %s\n", *args, args[0], args[1]);
  
  if ( *args==NULL ) return;


// echo
   
  if ( strncmp (args[0], "echo" , 4) == 0) {
  fprintf (stdout, "%s\n", args[1]);
  return;
  }


// exit
  
    
  if ( strncmp (args[0], "exit" , 4) == 0) {
  fprintf (stdout, "Quit. \n");
  exit ();
  }

    
// chdir

  if ( strncmp (args[0], "cd", 2) == 0 ) {
  	if (!args[1]) args[1]=(char *) getuserdir();
   	chdir (args[1]);
  	return;
  }
 
// setenv e getenv 

  if ( strncmp (args[0], "setenv", 6) == 0 ) {
  	
  	if (strsep (args[1], "=") !=0 ) {
  		fprintf (stdout, "No = allowed punx\n");
  		return;
  		}
  	
  	if ( strncmp (args[2], "=", 1)==0) {
  		fprintf (stdout, "Warning, the use of = is deprecated !\n");
  		fprintf (stdout, "%s=%s\n",args[1], args[3]);
  		setenv (args[1], args[3], 1);
  		return;
  		}
  	
  	fprintf (stdout, "%s=%s\n",args[1], args[2]);
  	setenv (args[1], args[2], 1);
  	return;
  	}
  	
  if ( strncmp (args[0], "getenv", 6) == 0 ) {
 	
	fprintf (stdout, "%s=%s\n", args[1], getenv(args[1]));
  	return ;
  	}
  	
  /*
   * Get a child process.
   */
  
  if ((pid = fork ()) < 0)
    {
      perror ("fork");
      exit (1);

      /* NOTE: perror() produces a short  error  message  on  the  standard
         error describing the last error encountered during a call to
         a system or library function.
       */
    }

  /*
   * The child executes the code inside the if.
   */
  
  if (pid == 0)
    {
      execvp (*args, args);
      perror (*args);
      exit (1);

      /* NOTE: The execv() vnd execvp versions of execl() are useful when the
         number  of  arguments is unknown in advance;
         The arguments to execv() and execvp()  are the name
         of the file to be executed and a vector of strings  contain-
         ing  the  arguments.   The last argument string must be fol-
         lowed by a 0 pointer. 

         execlp() and execvp() are called with the same arguments  as
         execl()  and  execv(),  but duplicate the shell's actions in
         searching for an executable file in a list  of  directories.
         The directory list is obtained from the environment.
       */
    }

  
  /*
   * The parent executes the wait.
   */
  
  while (wait (&status) != pid)
    
    /* empty */ ;
}
