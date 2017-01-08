#include <asm/unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/syscall.h>
#include <string.h>
#include <unistd.h>

#ifndef __NR_xmergesort
#error xmergesort system call not defined
#endif

#define flagu 0x01
#define flaga 0x02
#define flagi 0x04
#define flagt 0x10
#define flagd 0x20

typedef struct myargs{
    char  files[3][1024];
	char flags;
	int *data;
}myargs;


int main(int argc, char * const*argv)
{
	myargs *args;
	int rc,index,k,err=0;
	//int i,j;
	args=(myargs *) malloc(sizeof(myargs));
	args->data=(int *) malloc(sizeof(int));
	args->flags=0;

	while ((rc = getopt (argc, argv, "uaitd")) != -1)
    switch (rc)
      {
      case 'u':
        args->flags = args->flags|flagu;
		break;
      case 'a':
		args->flags= args->flags|flaga; 
				
        break;
      case 'i':
        args->flags= args->flags|flagi;
        break;
      case 't':
		args->flags= args->flags| flagt;
        break;
	  case 'd':
		args->flags= args->flags| flagd;	
		break;
	  case '?':
		//args->args =
        printf("Invalid flag found %c\n ",optopt);
		err=-1;
		break;
           }
	if( !args->flags )
	  {
		 printf("Please input the proper flag,aborting....\n");
		 rc=-1;
		 goto out;
	  }  
	if( (args->flags& flagu) && (args->flags&flaga) )
	  {
		 printf("Flag combination a & u not valid,retry\n");
		 rc=-1;
		 goto out;
	  }
	  
	for (index = optind,k=0; index <argc; index++,k++)
	{
		strncpy(args->files[k], argv[index],strlen(argv[index]));
		//printf ("Non-option argument %s\n", argv[index]);
	}
	if(err)
		goto out;
		
	void *dummy = (void *)args;
	err = syscall(__NR_xmergesort, dummy);  //systemcall
	printf("current flag value %d\n",args->flags);
	if (err== 0)
	{
		if(args->flags && flagd)
			printf("syscall successfully returned %d ,records written = %d\n", err, *(args->data));
		else
			printf("syscall successfully returned %d\n", err);
		}
	else
		perror("ERROR");  // to print error
		
	out:
	if(args->data!=NULL)
		free(args->data);
	if(args!=NULL)	
		free(args);
	exit(err);
}
