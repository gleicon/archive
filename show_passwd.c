/* teste com passwd */

#include <pwd.h>
#include <stdio.h>
#include <sys/types.h>
#include <shadow.h>
#include <stdlib.h>


//#define USER="adm"
 
struct spwd *s;

void main ()
 {
 	struct passwd *p;
  	
//  	struct spwd *s;
  	
  	char user[12]="tosco\0";
  	      
        
        if (getuid () !=0 ) 
        {
         fprintf (stderr,"You must be root...\n");
         exit (0);
         }
        
        setpwent();
        setspent();
        
        
        while ( p=getpwent()) 	
        {
        	if (strcmp (p->pw_name, user)==0)  fprintf (stdout, "%s:%s:%d:%d:%s:%s:%s;\n", p->pw_name, p->pw_passwd, p->pw_uid, 
        				p->pw_gid, p->pw_gecos, p->pw_dir, p->pw_shell);
         
         }     

 	p=getpwnam(user);
 	s=getspnam (user);
 	
 	fprintf (stdout, "%s:%s:%d:%d:%s:%s:%s;\n", p->pw_name, p->pw_passwd, p->pw_uid, 
        				p->pw_gid, p->pw_gecos, p->pw_dir, p->pw_shell);
         
        fprintf (stdout,"%s:%s:%d:%d:%d:%d:%d:%d:%d\n" , s->sp_namp, s->sp_pwdp,
         				s->sp_lstchg, s->sp_min, s->sp_max, s->sp_warn,
         				s->sp_inact, s->sp_expire, s->sp_flag);
         				 
              	
	endpwent();
	endspent();
	
 	
 }
              

                    