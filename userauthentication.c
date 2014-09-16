#include<stdio.h>
#include<security/pam_appl.h>
#include<security/pam_misc.h>
#include<security/pam_modules.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>	//write , close 
#include<stdlib.h>	//exit
#include<pwd.h>

//cc Ex-linuxfoundation.org.c -lpam -ldl -lpam_misc

/*
 * return 1 if auth succeds else return 0
 */

static struct pam_conv conv = {
	    misc_conv,
	        NULL
};

int userAuth(const char *uname,char *password)
{
	int ofd;
	pam_handle_t *pamh;
	int pam_status;
	int ret;
	char t_uname[256];

	struct passwd *p1;
	strcpy(t_uname,uname);

	printf("******userAuth.c******************************\n");
	printf("UserName:%s and password:%s\n",uname,password);
	printf("**********************************************\n");

	/* quick fix password is stored in flat file and deleted after performing the action */
	
	if ( (ofd=open("/tmp/passfile",O_WRONLY|O_CREAT,S_IRWXU)) < 0 ) {
		perror("open ");
		exit(0);
	}
	write(ofd,password,strlen(password));
	close(ofd);
	dup2(0,10);
	close(0);
	if ( (ofd=open("/tmp/passfile",O_RDONLY)) < 0 ) {
		perror("reading password file ");
		printf("uid of this process...:%d\n",getuid());
		exit(0);
	}
	
	/* PAM AUTHENTICATION MODULE */
	pam_status = pam_start("lsb_login",NULL, &conv, &pamh);
	pam_status = pam_set_item(pamh,PAM_USER,uname);

	if (pam_status == PAM_SUCCESS)
		pam_status = pam_authenticate(pamh, PAM_SILENT);
	printf("pam_authenticate call done...\n");
	if (pam_status == PAM_SUCCESS) {
		pam_get_item(pamh, PAM_USER, (const void **)&uname);
		fprintf(stdout, "Greetings %s\n", uname);
		ret = 1;
	} else {
		printf("%s\n", pam_strerror(pamh, pam_status));
		ret = 0;
	}
	pam_end(pamh, pam_status);
	
	/*Finally remove the temp pass file */	
	unlink("/tmp/passfile");
	close(ofd);
	dup2(10,0);
	if ( ret == 0)	//authentication failed
		return -1;
	else {		//authentication success
		/* get the userid for this user and return this val */
		printf("Getting userid...\n");
		if ( (p1=getpwnam(t_uname)) != NULL ) {
			printf("Userid for %s is :%d\n",t_uname,p1->pw_uid);
			return(p1->pw_uid);
		}
	}
	return -1;
}

