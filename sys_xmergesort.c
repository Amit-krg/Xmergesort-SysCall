#include <linux/linkage.h>
#include <linux/moduleloader.h>
#include<linux/fs.h>
#include<linux/namei.h>
#include<asm/page.h>
#include<linux/uaccess.h>
#include<linux/kernel.h>
#include<linux/slab.h>
#include<linux/string.h>
#include<asm/uaccess.h>
#include<linux/stat.h>
#include<linux/errno.h>
//#include<stdio.h>

#define DEBUGON		1
#define flagu 0x01
#define flaga 0x02
#define flagi 0x04
#define flagt 0x10
#define flagd 0x20

asmlinkage extern long (*sysptr)(void *arg);

typedef struct myargs{
    char  files[3][1024];
	char flags;
	int *data;
}myargs;

typedef struct internal{
	 int records;
	int err;
}internal;


/*
*
* Define
* if no '\n' present but there are some character in the buffer return the count of those left characters
* if '\n' is present copy the string in tar and return 0
* else return -1, problem with source or no more memory available
*
* 
*/
int nextString(char *tar,char *source,int tar_from,int source_from,int last)
{

	int i=0,to=source_from;
	if(NULL==source || NULL ==tar)
		return -1;
	
	while( source[to]!='\n' && to<last )
	{
		to++;
		//printk(KERN_ALERT"to currently");
	}
	
	//if a string with \n found append target with the characters till that \n else go to else condition and fill the left out bytes.
	
	if(source[to]=='\n')
		{	
		   if(to==source_from)
			{
				tar[tar_from]='\n';
				tar[tar_from+1]='\0';
				printk(KERN_ALERT"size 1 wala case\n");
				return 0;
			}	
			printk(KERN_ALERT"\n String : ");
			for(i=source_from;i<to ;i++)
			{
				tar[tar_from++]=source[i];
				
				//printk(KERN_ALERT"%c",tar[i]);
			}
			
			tar[tar_from]='\n';
			tar[tar_from+1]='\0';
			
			
			printk(KERN_ALERT"current string %s  Length = %d \n",tar,(to-source_from));
			return 0;
		}
	else if(to==last&& to!=source_from)
		{
			for(i=source_from;i<last;i++,tar_from++)
				{
					tar[tar_from]=source[i];
				}
				tar[tar_from+1]='\0';
			return source_from-to;
		}
	else if(source_from==last)
	{
		tar[0]='\0';
		return -1;
	}
	else
	{
	  printk("from nextString: control should not be here\n");
	}
    return -1;
}

/*
* internal sorting function

*/

internal xmergesortInternal(struct file *filp1,struct file *filp2,struct file *filp3,myargs *input)
{
	internal result;
	char *kbuf1=NULL;
	char *kbuf2=NULL;
	char *kbuf3=NULL;
	char *str_from1=NULL;
	char *str_from2=NULL;
	char *temp_buf=NULL;
	//char *temp_bug2=NULL;
	size_t byte1=0,byte2=0,byte3=0;	
	loff_t pos1=0,pos2=0,pos3=0;
	//int records=0; //To return no. of records written
	int ret1,ret2,ret4,temp,index1,index2,index3,index4,i=0;
	result.err=0;
	result.records=0;   //to track the result from xmergesortInternal
	
	kbuf1=(char*)kmalloc( sizeof(char)*PAGE_SIZE,GFP_KERNEL);
	if(!kbuf1)
	{
		printk(KERN_ALERT"failed to allocate memory for buffer one");
		result.err=-ENOMEM;
		goto out;
	}
	kbuf2=(char*)kmalloc( sizeof(char)*PAGE_SIZE,GFP_KERNEL);
	if(!kbuf2)
	{
		printk(KERN_ALERT"failed to allocate memory for buffer2");
		result.err=-ENOMEM;
		goto out;
	}
	kbuf3=(char*)kmalloc( sizeof(char)*PAGE_SIZE,GFP_KERNEL);
	if(!kbuf3)
	{
		printk(KERN_ALERT"failed to allocate memory for buffer2");
		result.err=-ENOMEM;
		goto out;
	}
	str_from1=(char*)kmalloc( sizeof(char)*PAGE_SIZE,GFP_KERNEL);
	if(!str_from1)
	{
		printk(KERN_ALERT"failed to allocate memory for string 1 buffer");
		result.err=-ENOMEM;
		goto out;
	}
	str_from2=(char*)kmalloc( sizeof(char)*PAGE_SIZE,GFP_KERNEL);
	if(!str_from2)
	{
		printk(KERN_ALERT"failed to allocate memory for string2 buffer");
		result.err=-ENOMEM;
		goto out;
	}
	temp_buf=(char*)kmalloc( sizeof(char)*PAGE_SIZE,GFP_KERNEL);
	if(!temp_buf)
	{
		printk(KERN_ALERT"failed to allocate memory for temp_buf1 buffer");
		result.err=-ENOMEM;
		goto out;
	}
	
	memset(kbuf1,'\0',PAGE_SIZE*sizeof(char));
    memset(kbuf2,'\0',PAGE_SIZE*sizeof(char));
    memset(kbuf3,'\0',PAGE_SIZE*sizeof(char));
	memset(str_from1,'\0',PAGE_SIZE*sizeof(char));
	memset(str_from2,'\0',PAGE_SIZE*sizeof(char));
	memset(temp_buf,'\0',PAGE_SIZE*sizeof(char));
	
	
	
	printk("Mergesort:starting read\n");
	
	byte1=vfs_read(filp1,kbuf1,PAGE_SIZE,&pos1);
	if(byte1<=0) 
	{
		printk(KERN_ALERT "buffer read error in file1\n");
		result.err=-EINVAL;
		goto out;
	}
	
	kbuf1[byte1]='\0';
	byte2=vfs_read(filp2,kbuf2,PAGE_SIZE,&pos2);
	if(byte2<=0) 
	{
		printk(KERN_ALERT"buffer read error in file2\n ");
		result.err=-EINVAL;
		goto out;
	}
	
	 kbuf2[byte2]='\0';
	//index1 starting index of buf1 ,index2 starting index of buf2 ,index3 starting index of buf3
	
	/*compares two and write smaller one first and then greater,include duplicates*/
	 index3=0;
		//#ifdef DEBUGON
		
		//#endif
		index1=0;index2=0,index4=0;
		ret1=nextString(str_from1,kbuf1,0,index1,byte1);
		if(str_from1==NULL)
		{
			result.err=-ENOMEM;
			printk(KERN_ALERT "str_from1 is empty\n");
			goto out;
		}
		ret2=nextString(str_from2,kbuf2,0,index2,byte2);
		if(str_from2==NULL)
		{
			result.err=-ENOMEM;
			printk(KERN_ALERT"str_from2 is empty\n");
			goto out;
		}	
	do
	{		
		
		i=0;
		printk(KERN_ALERT"length of %s=%zu and  %s=%zu\n",str_from1,strlen(str_from1),str_from2,strlen(str_from2));		
		while( (index1<byte1 && index2<byte2)&& (0==ret1 && 0==ret2)  )
		{ 
		
			if(input->flags& flaga)     //nomal compare and input duplicate records
			{
				if(input->flags & flagi)   // 
				{
					if(input->flags & flagt) //flag a,i,t  case insenstive compare and input duplicate but sorted
					{
						if(strcasecmp(str_from1,str_from2)<=0)
						{
							if(strcasecmp(temp_buf,str_from1)<=0) //duplicates allowed hence using equal ,write temp_buf to outbuffer, fill temp_buf with current string, fill current string from kbuf1
								{
									if( (PAGE_SIZE-index3)> strlen(str_from1))
									{	
										strcat(kbuf3,str_from1);
										index3=index3+strlen(str_from1);
										kbuf3[index3++]='\0';
										result.records++; 				//increment records
										printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
								
										//fill temp_buf
										ret4=nextString(temp_buf,str_from1,0,0,strlen(str_from1));
										index4=strlen(str_from1);
										temp_buf[index4++]='\0';
								
										//update index and check for new string
										index1=index1+strlen(str_from1);
										ret1=nextString(str_from1,kbuf1,0,index1,byte1);  //str_from1 is already parsed so fill in the new string
										#ifdef DEBUGON
										printk("new ret1 value is %d for %drun\n",ret1,i+1);	
										#endif	
									}
									else
									{
										temp=vfs_write(filp3, kbuf3,strlen(kbuf3), &pos3);
										if(temp<0)
										{
											printk(KERN_ALERT "File write error writing from str_from1\n");
											goto out;
										}
										else
										{
											index3=0;
											strcat(kbuf3,str_from1);
											index3=index3+strlen(str_from1);
											kbuf3[index3++]='\0';
											result.records++; 				//increment records
											#ifdef DEBUGON
											printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
											#endif
								
											//fill temp_buf
											ret4=nextString(temp_buf,str_from1,0,0,strlen(str_from1));
											index4=strlen(str_from1);
											temp_buf[index4++]='\0';
								
											//update index and check for new string
											index1=index1+strlen(str_from1);
											ret1=nextString(str_from1,kbuf1,0,index1,byte1);  //str_from1 is already parsed so fill in the new string
											#ifdef DEBUGON
											printk("new ret1 value is %d for %drun\n",ret1,i+1);
											#endif	
										}
									}	
								}
								else // str_from1 should be greater than temp_buf for t ,error case exiting
								{
									result.err=-EINVAL;
									printk("str_from1:%s, found to be lesser than temp_buf:%s, error case in a->i->t\n",str_from1,temp_buf);
									goto out;
								}
							
						} 
						else  //str2 smaller than str1 a,i & t 
						{
							if(strcasecmp(temp_buf,str_from2)<=0) //duplicates allowed hence using equal ,append current string to outbuffer and update temp_buf with it
								{
									if( (PAGE_SIZE-index3)> strlen(str_from1))  //case when outbuffer has insufficient space
									{	
										strcat(kbuf3,str_from2);
										index3=index3+strlen(str_from2);
										kbuf3[index3++]='\0';
										result.records++; 				//increment records
										#ifdef DEBUGON
										printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
										#endif
								
										//fill temp_buf
										ret4=nextString(temp_buf,str_from2,0,0,strlen(str_from2));
										index4=strlen(str_from2);
										temp_buf[index4++]='\0';
								
										//update index and check for new string
										index2+=strlen(str_from2);
										ret2=nextString(str_from2,kbuf2,0,index2,byte2);  //str_from1 is already parsed so fill in the new string
										#ifdef DEBUGON
										printk("new ret2 value is %d for %d run\n",ret2,i+1);
										#endif	
									}
									else
									{
										temp=vfs_write(filp3, kbuf3,strlen(kbuf3), &pos3);
										if(temp<0)
										{
											printk(KERN_ALERT "File write error writing from str_from2\n");
											goto out;
										}
										else
										{
											index3=0;
											strcat(kbuf3,str_from2);
											index3=index3+strlen(str_from2);
											kbuf3[index3++]='\0';
											result.records++; 				//increment records
											#ifdef DEBUGON
											printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
											#endif
								
											//fill temp_buf
											ret4=nextString(temp_buf,str_from2,0,0,strlen(str_from2));
											index4=strlen(str_from2);
											temp_buf[index4++]='\0';
								
											//update index and check for new string
											index2+=strlen(str_from2);
											ret2=nextString(str_from2,kbuf2,0,index2,byte2);  //str_from1 is already parsed so fill in the new string
											#ifdef DEBUGON
											printk("new ret2 value is %d for %d run\n",ret2,i+1);
											#endif
										}
										
									}	
						
								}
								else  // case when incoming string is smaller than already existing in the outbuffer, tflag will return an error
								{				
								
									result.err=-EINVAL;
									printk("str_from2:%s, found to be lesser than temp_buf:%s, error case in a->i->t\n",str_from2,temp_buf);
									goto out;				
								}
						}
					} //end of a,i,t	
					
					else // a,i but no t meaning duplicate records are allowed and smaller one will be skipped
					{
					  if(strcasecmp(str_from1,str_from2)<=0)
							{
								if(strcmp(temp_buf,str_from1)<=0) //duplicates allowed hence using equal ,write temp_buf to outbuffer, fill temp_buf with current string, fill current string from kbuf1
								{
									if( (PAGE_SIZE-index3)> strlen(str_from1))
									{	
										strcat(kbuf3,str_from1);
										index3=index3+strlen(str_from1);
										kbuf3[index3++]='\0';
										result.records++; 				//increment records
										#ifdef DEBUGON
										printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
										#endif
								
										//fill temp_buf
										ret4=nextString(temp_buf,str_from1,0,0,strlen(str_from1));
										index4=strlen(str_from1);
										temp_buf[index4++]='\0';
								
										//update index and check for new string
										index1=index1+strlen(str_from1);
										ret1=nextString(str_from1,kbuf1,0,index1,byte1);  //str_from1 is already parsed so fill in the new string
										#ifdef DEBUGON
										printk("new ret1 value is %d for %drun\n",ret1,i+1);
										#endif
									}
									else
									{
										temp=vfs_write(filp3, kbuf3,strlen(kbuf3), &pos3);
										if(temp<0)
										{
											printk(KERN_ALERT "File write error writing from str_from1\n");
											goto out;
										}
										else
										{
											index3=0;
											strcat(kbuf3,str_from1);
											index3=index3+strlen(str_from1);
											kbuf3[index3++]='\0';
											result.records++; 				//increment records
											#ifdef DEBUGON
											printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
											#endif
								
											//fill temp_buf
											ret4=nextString(temp_buf,str_from1,0,0,strlen(str_from1));
											index4=strlen(str_from1);
											temp_buf[index4++]='\0';
								
											//update index and check for new string
											index1=index1+strlen(str_from1);
											ret1=nextString(str_from1,kbuf1,0,index1,byte1);  //str_from1 is already parsed so fill in the new string
											#ifdef DEBUGON
											printk("new ret1 value is %d for %drun\n",ret1,i+1);
											#endif
										}
									}	
								}
								else // str_from1 should be greater than temp_buf for a,i skipping the smaller content and filling str_from1 again
								{
									index1=index1+strlen(str_from1);
									ret1=nextString(str_from1,kbuf1,0,index1,byte1);
									printk("new ret1 value is %d for %drun\n",ret1,i+1);
								}
							}
						else  //str2 smaller than str1 a,i & t 
						{
							
							if(strcasecmp(temp_buf,str_from2)<=0) //duplicates allowed hence using equal ,append current string to outbuffer and update temp_buf with it
								{
									if( (PAGE_SIZE-index3)> strlen(str_from1))  //case when outbuffer has insufficient space
									{	
										strcat(kbuf3,str_from2);
										index3=index3+strlen(str_from2);
										kbuf3[index3++]='\0';
										result.records++; 				//increment records
										#ifdef DEBUGON
										printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
										#endif
								
										//fill temp_buf
										ret4=nextString(temp_buf,str_from2,0,0,strlen(str_from2));
										index4=strlen(str_from2);
										temp_buf[index4++]='\0';
								
										//update index and check for new string
										index2+=strlen(str_from2);
										ret2=nextString(str_from2,kbuf2,0,index2,byte2);  //str_from1 is already parsed so fill in the new string
										#ifdef DEBUGON
										printk("new ret2 value is %d for %d run\n",ret2,i+1);
										#endif	
									}
									else
									{
										temp=vfs_write(filp3, kbuf3,strlen(kbuf3), &pos3);
										if(temp<0)
										{
											printk(KERN_ALERT "File write error writing from str_from2\n");
											goto out;
										}
										else
										{
											index3=0;
											strcat(kbuf3,str_from2);
											index3=index3+strlen(str_from2);
											kbuf3[index3++]='\0';
											result.records++; 				//increment records
											printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
								
											//fill temp_buf
											ret4=nextString(temp_buf,str_from2,0,0,strlen(str_from2));
											index4=strlen(str_from2);
											temp_buf[index4++]='\0';
								
											//update index and check for new string
											index2+=strlen(str_from2);
											ret2=nextString(str_from2,kbuf2,0,index2,byte2);  //str_from1 is already parsed so fill in the new string
											#ifdef DEBUGON
											printk("new ret2 value is %d for %d run\n",ret2,i+1);
											#endif
										}
									}	
						
								}
								else  // str_from2 should be greater than temp_buf for a,i skipping the smaller content and filling str_from2 again
								{				
								
									index2=index2+strlen(str_from2);
									ret2=nextString(str_from2,kbuf2,0,index2,byte2);
									#ifdef DEBUGON
									printk("new ret2 value is %d for %d run\n",ret2,i+1);
									#endif									
								}
		
						}
						
					}//end of a & i but no t
				}
				if(input->flags & flagt)// a and t are there
				{
					if(strcmp(str_from1,str_from2)<=0)
						{
							if(strcmp(temp_buf,str_from1)<=0) //duplicates allowed hence using equal ,write temp_buf to outbuffer, fill temp_buf with current string, fill current string from kbuf1
								{
									if( (PAGE_SIZE-index3)> strlen(str_from1))
									{	
										strcat(kbuf3,str_from1);
										index3=index3+strlen(str_from1);
										kbuf3[index3++]='\0';
										result.records++; 				//increment records
										#ifdef DEBUGON
										printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
										#endif
										//fill temp_buf
										ret4=nextString(temp_buf,str_from1,0,0,strlen(str_from1));
										index4=strlen(str_from1);
										temp_buf[index4++]='\0';
								
										//update index and check for new string
										index1=index1+strlen(str_from1);
										ret1=nextString(str_from1,kbuf1,0,index1,byte1);  //str_from1 is already parsed so fill in the new string
										#ifdef DEBUGON
										printk("new ret1 value is %d for %drun\n",ret1,i+1);
										#endif
									}
									else
									{
										temp=vfs_write(filp3, kbuf3,strlen(kbuf3), &pos3);
										if(temp<0)
										{
											printk(KERN_ALERT "File write error writing from str_from1\n");
											goto out;
										}
										else
										{
											index3=0;
											strcat(kbuf3,str_from1);
											index3=index3+strlen(str_from1);
											kbuf3[index3++]='\0';
											result.records++; 				//increment records
											#ifdef DEBUGON
											printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
											#endif
								
											//fill temp_buf
											ret4=nextString(temp_buf,str_from1,0,0,strlen(str_from1));
											index4=strlen(str_from1);
											temp_buf[index4++]='\0';
								
											//update index and check for new string
											index1=index1+strlen(str_from1);
											ret1=nextString(str_from1,kbuf1,0,index1,byte1);  //str_from1 is already parsed so fill in the new string
											#ifdef DEBUGON
											printk("new ret1 value is %d for %drun\n",ret1,i+1);
											#endif
										}
									}	
								}
								else // str_from1 should be greater than temp_buf for t ,error case exiting
								{
									result.err=-EINVAL;
									printk("str_from1:%s, found to be lesser than temp_buf:%s, error case in a->i->t\n",str_from1,temp_buf);
									goto out;
								}
							
						} 
						else  //str2 smaller than str1 in a& t 
						{
							if(strcmp(temp_buf,str_from2)<=0) //duplicates allowed hence using equal ,append current string to outbuffer and update temp_buf with it
								{
									if( (PAGE_SIZE-index3)> strlen(str_from1))  //case when outbuffer has insufficient space
									{	
										strcat(kbuf3,str_from2);
										index3=index3+strlen(str_from2);
										kbuf3[index3++]='\0';
										result.records++; 				//increment records
										#ifdef DEBUGON
										printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
										#endif
								
										//fill temp_buf
										ret4=nextString(temp_buf,str_from2,0,0,strlen(str_from2));
										index4=strlen(str_from2);
										temp_buf[index4++]='\0';
								
										//update index and check for new string
										index2+=strlen(str_from2);
										ret2=nextString(str_from2,kbuf2,0,index2,byte2);  //str_from1 is already parsed so fill in the new string
										#ifdef DEBUGON
										printk("new ret2 value is %d for %d run\n",ret2,i+1);
										#endif
										
									}
									else
									{
										temp=vfs_write(filp3, kbuf3,strlen(kbuf3), &pos3);
										if(temp<0)
										{
											printk(KERN_ALERT "File write error writing from str_from2\n");
											goto out;
										}
										else
										{
											index3=0;
											strcat(kbuf3,str_from2);
											index3=index3+strlen(str_from2);
											kbuf3[index3++]='\0';
											result.records++; 				//increment records
											#ifdef DEBUGON
											printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
											#endif
								
											//fill temp_buf
											ret4=nextString(temp_buf,str_from2,0,0,strlen(str_from2));
											index4=strlen(str_from2);
											temp_buf[index4++]='\0';
								
											//update index and check for new string
											index2+=strlen(str_from2);
											ret2=nextString(str_from2,kbuf2,0,index2,byte2);  //str_from2 is already parsed so fill in the new string
											#ifdef DEBUGON
											printk("new ret2 value is %d for %d run\n",ret2,i+1);
											#endif
										}
									}	
						
								}
								else  // case when incoming string is smaller than already existing in the outbuffer, tflag will return an error
								{				
								
									result.err=-EINVAL;
									printk("str_from2:%s, found to be lesser than temp_buf:%s, error case in a->i->t\n",str_from2,temp_buf);
									goto out;				
								}
						} // end of str2 smaller than str1 case
						
				}
				
				else  // only a is there
				{
					if(strcmp(str_from1,str_from2)<=0)
							{
								if(strcmp(temp_buf,str_from1)<=0) //duplicates allowed hence using equal ,write temp_buf to outbuffer, fill temp_buf with current string, fill current string from kbuf1
								{
									if( (PAGE_SIZE-index3)> strlen(str_from1))
									{	
										strcat(kbuf3,str_from1);
										index3=index3+strlen(str_from1);
										kbuf3[index3++]='\0';
										result.records++; 				//increment records
										#ifdef DEBUGON
										printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
										#endif
										//fill temp_buf
										ret4=nextString(temp_buf,str_from1,0,0,strlen(str_from1));
										index4=strlen(str_from1);
										temp_buf[index4++]='\0';
								
										//update index and check for new string
										index1=index1+strlen(str_from1);
										ret1=nextString(str_from1,kbuf1,0,index1,byte1);  //str_from1 is already parsed so fill in the new string
										#ifdef DEBUGON
										printk("new ret1 value is %d for %drun\n",ret1,i+1);	
										#endif	
									}
									else
									{
										temp=vfs_write(filp3, kbuf3,strlen(kbuf3), &pos3);
										if(temp<0)
										{
											printk(KERN_ALERT "File write error writing from str_from1\n");
											goto out;
										}
										else
										{
											index3=0;
											strcat(kbuf3,str_from1);
											index3=index3+strlen(str_from1);
											kbuf3[index3++]='\0';
											result.records++; 				//increment records
											printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
								
											//fill temp_buf
											ret4=nextString(temp_buf,str_from1,0,0,strlen(str_from1));
											index4=strlen(str_from1);
											temp_buf[index4++]='\0';
								
											//update index and check for new string
											index1=index1+strlen(str_from1);
											ret1=nextString(str_from1,kbuf1,0,index1,byte1);  //str_from1 is already parsed so fill in the new string
											#ifdef DEBUGON
											printk("new ret1 value is %d for %drun\n",ret1,i+1);
											#endif
										}
									}	
								}
								else // str_from1 should be greater than last written string for a skipping the smaller content and filling str_from1 again
								{
									index1=index1+strlen(str_from1);
									ret1=nextString(str_from1,kbuf1,0,index1,byte1);
									printk("new ret1 value is %d for %drun\n",ret1,i+1);
								}
							}
						else  //str2 smaller than str1 a,i & t 
						{
							
							if(strcasecmp(temp_buf,str_from2)<=0) //duplicates allowed hence using equal ,append current string to outbuffer and update temp_buf with it
								{
									if( (PAGE_SIZE-index3)> strlen(str_from1))  //case when outbuffer has insufficient space
									{	
										strcat(kbuf3,str_from2);
										index3=index3+strlen(str_from2);
										kbuf3[index3++]='\0';
										result.records++; 				//increment records
										#ifdef DEBUGON
										printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
										#endif
										//fill temp_buf
										ret4=nextString(temp_buf,str_from2,0,0,strlen(str_from2));
										index4=strlen(str_from2);
										temp_buf[index4++]='\0';
								
										//update index and check for new string
										index2+=strlen(str_from2);
										ret2=nextString(str_from2,kbuf2,0,index2,byte2);  //str_from1 is already parsed so fill in the new string
										#ifdef DEBUGON
										printk("new ret2 value is %d for %d run\n",ret2,i+1);	
										#endif
									}
									else
									{
										temp=vfs_write(filp3, kbuf3,strlen(kbuf3), &pos3);
										if(temp<0)
										{
											printk(KERN_ALERT "File write error writing from str_from2\n");
											goto out;
										}
										else
										{
											index3=0;
											strcat(kbuf3,str_from2);
											index3=index3+strlen(str_from2);
											kbuf3[index3++]='\0';
											result.records++; 				//increment records
											#ifdef DEBUGON
											printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
											#endif
								
											//fill temp_buf
											ret4=nextString(temp_buf,str_from2,0,0,strlen(str_from2));
											index4=strlen(str_from2);
											temp_buf[index4++]='\0';
								
											//update index and check for new string
											index2+=strlen(str_from2);
											ret2=nextString(str_from2,kbuf2,0,index2,byte2);  //str_from1 is already parsed so fill in the new string
											#ifdef DEBUGON
											printk("new ret2 value is %d for %d run\n",ret2,i+1);
											#endif
										}
									}	
						
								}
								else  // str_from2 should be greater than temp_buf for a,i skipping the smaller content and filling str_from2 again
								{				
								
									index2=index2+strlen(str_from2);
									ret2=nextString(str_from2,kbuf2,0,index2,byte2);
									#ifdef DEBUGON
									printk("new ret2 value is %d for %d run\n",ret2,i+1);
									#endif	
								}
		
						}
						
				}//end of a	
			}	
			else if(input->flags & flagu)
			{
				if(input->flags & flagi)
				{
					if(input->flags & flagt)  //case of u,i & t
					{
						if(strcasecmp(str_from1,str_from2)<=0)  //both strings can be same as well, remove one instance following u flag constraint
						{
							 if(strcasecmp(str_from1,str_from2)==0) // remove one of the instance,removing str_From2
								{
									index2=index2+strlen(str_from2);
									ret2=nextString(str_from2,kbuf2,0,index2,byte2);  //str_from1 is already parsed so fill in the new string
									#ifdef DEBUGON
									printk("new ret2 value is %d for %d run\n",ret2,i+1);	
									#endif
								}
								if(strcasecmp(temp_buf,str_from1)<=0)
								{
									if(strcasecmp(temp_buf,str_from1)==0) // remove one of the instance,updating str_from1
									{
										index1=index1+strlen(str_from1);
										ret1=nextString(str_from1,kbuf1,0,index1,byte1);  //str_from1 is already parsed so fill in the new string
										#ifdef DEBUGON
										printk("new ret2 value is %d for %d run\n",ret2,i+1);
										#endif	
									}
									if( (PAGE_SIZE-index3)> strlen(str_from1))
									{	
										strcat(kbuf3,str_from1);
										index3=index3+strlen(str_from1);
										kbuf3[index3++]='\0';
										result.records++; 				//increment records
										#ifdef DEBUGON
										printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
										#endif
								
										//fill temp_buf
										ret4=nextString(temp_buf,str_from1,0,0,strlen(str_from1));
										index4=strlen(str_from1);
										temp_buf[index4++]='\0';
								
										//update index and check for new string
										index1=index1+strlen(str_from1);
										ret1=nextString(str_from1,kbuf1,0,index1,byte1);  //str_from1 is already parsed so fill in the new string
										#ifdef DEBUGON
										printk("new ret1 value is %d for %drun\n",ret1,i+1);		
										#endif	
									}
									else
									{
										temp=vfs_write(filp3, kbuf3,strlen(kbuf3), &pos3);
										if(temp<0)
										{
											printk(KERN_ALERT "File write error writing from str_from1\n");
											goto out;
										}
										else
										{
											index3=0;
											strcat(kbuf3,str_from1);
											index3=index3+strlen(str_from1);
											kbuf3[index3++]='\0';
											result.records++; 				//increment records
											#ifdef DEBUGON
											printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
											#endif
											//fill temp_buf
											ret4=nextString(temp_buf,str_from1,0,0,strlen(str_from1));
											index4=strlen(str_from1);
											temp_buf[index4++]='\0';
								
											//update index and check for new string
											index1=index1+strlen(str_from1);
											ret1=nextString(str_from1,kbuf1,0,index1,byte1);  //str_from1 is already parsed so fill in the new string
											#ifdef DEBUGON
											printk("new ret1 value is %d for %drun\n",ret1,i+1);
											#endif
										}
									}	
								}
								else // str_from1 should be greater than temp_buf for t ,error case exiting
								{
									result.err=-EINVAL;
									printk("str_from1:%s, found to be lesser than temp_buf:%s, error case in a->i->t\n",str_from1,temp_buf);
									goto out;
								}
							
						} 
						else  //str2 smaller than str1 u,i & t 
						{
							if(strcasecmp(temp_buf,str_from2)<=0) //duplicates not allowed hence using equal 
								{
									if(strcasecmp(temp_buf,str_from2)==0) // remove one of the instance,removing str_From2
									{
										index2=index2+strlen(str_from2);
										ret2=nextString(str_from2,kbuf2,0,index2,byte2);  //str_from1 is already parsed so fill in the new string
										#ifdef DEBUGON
										printk("new ret2 value is %d for %d run\n",ret2,i+1);	
										#endif
									}
									if( (PAGE_SIZE-index3)> strlen(str_from1))  //case when outbuffer has insufficient space
									{	
										strcat(kbuf3,str_from2);
										index3=index3+strlen(str_from2);
										kbuf3[index3++]='\0';
										result.records++; 				//increment records
										#ifdef DEBUGON
										printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
										#endif
										//fill temp_buf
										ret4=nextString(temp_buf,str_from2,0,0,strlen(str_from2));
										index4=strlen(str_from2);
										temp_buf[index4++]='\0';
								
										//update index and check for new string
										index2+=strlen(str_from2);
										ret2=nextString(str_from2,kbuf2,0,index2,byte2);  //str_from1 is already parsed so fill in the new string
										#ifdef DEBUGON
										printk("new ret2 value is %d for %d run\n",ret2,i+1);
										#endif	
									}
									else
									{
										temp=vfs_write(filp3, kbuf3,strlen(kbuf3), &pos3);
										if(temp<0)
										{
											printk(KERN_ALERT "File write error writing from str_from2\n");
											goto out;
										}
										else
										{
											index3=0;
											strcat(kbuf3,str_from2);
											index3=index3+strlen(str_from2);
											kbuf3[index3++]='\0';
											result.records++; 				//increment records
											#ifdef DEBUGON
											printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
											#endif
											//fill temp_buf
											ret4=nextString(temp_buf,str_from2,0,0,strlen(str_from2));
											index4=strlen(str_from2);
											temp_buf[index4++]='\0';
								
											//update index and check for new string
											index2+=strlen(str_from2);
											ret2=nextString(str_from2,kbuf2,0,index2,byte2);  //str_from1 is already parsed so fill in the new string
											#ifdef DEBUGON
											printk("new ret2 value is %d for %d run\n",ret2,i+1);
											#endif
										}
									}	
						
								}
								else  // case when incoming string is smaller than already existing in the outbuffer, tflag will return an error
								{				
								
									result.err=-EINVAL;
									printk("str_from2:%s, found to be lesser than temp_buf:%s, error case in a->i->t\n",str_from2,temp_buf);
									goto out;				
								}
						}
					} // end of u,i,t
				else // u,i but no t
					{
						if(strcasecmp(str_from1,str_from2)<=0)  //both strings can be same as well, remove one instance following u flag constraint
						{
							 if(strcasecmp(str_from1,str_from2)==0) // remove one of the instance,removing str_From2
								{
								index2=index2+strlen(str_from2);
								ret2=nextString(str_from2,kbuf2,0,index2,byte2);  //str_from1 is already parsed so fill in the new string
								#ifdef DEBUGON
								printk("new ret2 value is %d for %d run\n",ret2,i+1);
								#endif	
								}
								if(strcasecmp(temp_buf,str_from1)<=0)
								{
									if(strcasecmp(temp_buf,str_from1)==0) // remove one of the instance,removing str_From2
									{
										index1=index1+strlen(str_from1);
										ret1=nextString(str_from1,kbuf1,0,index1,byte1);  //str_from1 is already parsed so fill in the new string
										#ifdef DEBUGON
										printk("new ret2 value is %d for %d run\n",ret2,i+1);
										#endif	
									}
									if( (PAGE_SIZE-index3)> strlen(str_from1))
									{	
										strcat(kbuf3,str_from1);
										index3=index3+strlen(str_from1);
										kbuf3[index3++]='\0';
										result.records++; 				//increment records
										#ifdef DEBUGON
										printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
										#endif
								
										//fill temp_buf
										ret4=nextString(temp_buf,str_from1,0,0,strlen(str_from1));
										index4=strlen(str_from1);
										temp_buf[index4++]='\0';
								
										//update index and check for new string
										index1=index1+strlen(str_from1);
										ret1=nextString(str_from1,kbuf1,0,index1,byte1);  //str_from1 is already parsed so fill in the new string
										#ifdef DEBUGON
										printk("new ret1 value is %d for %drun\n",ret1,i+1);	
										#endif	
									}
									else
									{
										temp=vfs_write(filp3, kbuf3,strlen(kbuf3), &pos3);
										if(temp<0)
										{
											printk(KERN_ALERT "File write error writing from str_from1\n");
											goto out;
										}
										else
										{
											index3=0;
											strcat(kbuf3,str_from1);
											index3=index3+strlen(str_from1);
											kbuf3[index3++]='\0';
											result.records++; 				//increment records
											printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
								
											//fill temp_buf
											ret4=nextString(temp_buf,str_from1,0,0,strlen(str_from1));
											index4=strlen(str_from1);
											temp_buf[index4++]='\0';
								
											//update index and check for new string
											index1=index1+strlen(str_from1);
											ret1=nextString(str_from1,kbuf1,0,index1,byte1);  //str_from1 is already parsed so fill in the new string
											printk("new ret1 value is %d for %drun\n",ret1,i+1);
										}
									}	
								}
								else // str_from1 should be greater than temp_buf for t ,skipping
								{
									index1=index1+strlen(str_from1);
									ret1=nextString(str_from1,kbuf1,0,index1,byte1);  //str_from1 is already parsed so fill in the new string
									printk("new ret1 value is %d for %drun\n",ret1,i+1);
								}
							
						} 
						else  //str2 smaller than str1 u,i & t 
						{
							if(strcasecmp(temp_buf,str_from2)<=0) //duplicates not allowed hence using equal 
								{
									if(strcasecmp(temp_buf,str_from2)==0) // remove one of the instance,removing str_From2
									{
										index2=index2+strlen(str_from2);
										ret2=nextString(str_from2,kbuf2,0,index2,byte2);  //str_from1 is already parsed so fill in the new string
										printk("new ret2 value is %d for %d run\n",ret2,i+1);	
									}
									if( (PAGE_SIZE-index3)> strlen(str_from1))  //case when outbuffer has insufficient space
									{	
										strcat(kbuf3,str_from2);
										index3=index3+strlen(str_from2);
										kbuf3[index3++]='\0';
										result.records++; 				//increment records
										printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
								
										//fill temp_buf
										ret4=nextString(temp_buf,str_from2,0,0,strlen(str_from2));
										index4=strlen(str_from2);
										temp_buf[index4++]='\0';
								
										//update index and check for new string
										index2+=strlen(str_from2);
										ret2=nextString(str_from2,kbuf2,0,index2,byte2);  //str_from1 is already parsed so fill in the new string
										printk("new ret2 value is %d for %d run\n",ret2,i+1);			
									}
									else
									{
										temp=vfs_write(filp3, kbuf3,strlen(kbuf3), &pos3);
										if(temp<0)
										{
											printk(KERN_ALERT "File write error writing from str_from2\n");
											goto out;
										}
										else
										{
											index3=0;
											strcat(kbuf3,str_from2);
											index3=index3+strlen(str_from2);
											kbuf3[index3++]='\0';
											result.records++; 				//increment records
											printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
								
											//fill temp_buf
											ret4=nextString(temp_buf,str_from2,0,0,strlen(str_from2));
											index4=strlen(str_from2);
											temp_buf[index4++]='\0';
								
											//update index and check for new string
											index2+=strlen(str_from2);
											ret2=nextString(str_from2,kbuf2,0,index2,byte2);  //str_from1 is already parsed so fill in the new string
											printk("new ret2 value is %d for %d run\n",ret2,i+1);
										}
									}	
						
								}
								else  // case when incoming string is smaller than already existing in the outbuffer, tflag will return an error
								{				
								index2=index2+strlen(str_from2);
								ret2=nextString(str_from2,kbuf2,0,index2,byte2);  //str_from1 is already parsed so fill in the new string
								printk("new ret2 value is %d for %d run\n",ret2,i+1);												
								}
						}
					}//end of u,i but no t	
				}
				if(input->flags & flagt )  // no duplicates u &t
				{
					if(strcmp(str_from1,str_from2)<=0)  //both strings can be same as well, remove one instance following u flag constraint
						{
							 if(strcmp(str_from1,str_from2)==0) // remove one of the instance,removing str_From2
								{
								index2=index2+strlen(str_from2);
								ret2=nextString(str_from2,kbuf2,0,index2,byte2);  //str_from1 is already parsed so fill in the new string
								printk("new ret2 value is %d for %d run\n",ret2,i+1);	
								}
								if(strcmp(temp_buf,str_from1)<=0)
								{
									if(strcmp(temp_buf,str_from1)==0) // remove one of the instance,updating str_from1
									{
										index1=index1+strlen(str_from1);
										ret1=nextString(str_from1,kbuf1,0,index1,byte1);  //str_from1 is already parsed so fill in the new string
										printk("new ret2 value is %d for %d run\n",ret2,i+1);	
									}
									if( (PAGE_SIZE-index3)> strlen(str_from1))
									{	
										strcat(kbuf3,str_from1);
										index3=index3+strlen(str_from1);
										kbuf3[index3++]='\0';
										result.records++; 				//increment records
										printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
								
										//fill temp_buf
										ret4=nextString(temp_buf,str_from1,0,0,strlen(str_from1));
										index4=strlen(str_from1);
										temp_buf[index4++]='\0';
								
										//update index and check for new string
										index1=index1+strlen(str_from1);
										ret1=nextString(str_from1,kbuf1,0,index1,byte1);  //str_from1 is already parsed so fill in the new string
										printk("new ret1 value is %d for %drun\n",ret1,i+1);			
									}
									else
									{
										temp=vfs_write(filp3, kbuf3,strlen(kbuf3), &pos3);
										if(temp<0)
										{
											printk(KERN_ALERT "File write error writing from str_from1\n");
											goto out;
										}
										else
										{
											index3=0;
											strcat(kbuf3,str_from1);
											index3=index3+strlen(str_from1);
											kbuf3[index3++]='\0';
											result.records++; 				//increment records
											printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
								
											//fill temp_buf
											ret4=nextString(temp_buf,str_from1,0,0,strlen(str_from1));
											index4=strlen(str_from1);
											temp_buf[index4++]='\0';
								
											//update index and check for new string
											index1=index1+strlen(str_from1);
											ret1=nextString(str_from1,kbuf1,0,index1,byte1);  //str_from1 is already parsed so fill in the new string
											printk("new ret1 value is %d for %drun\n",ret1,i+1);
										}
									}	
								}
								else // str_from1 should be greater than temp_buf for t ,error case exiting
								{
									result.err=-EINVAL;
									printk("str_from1:%s, found to be lesser than temp_buf:%s, error case in a->i->t\n",str_from1,temp_buf);
									goto out;
								}
							
						} 
						else  //str2 smaller than str1  case u & t
						{
							if(strcmp(temp_buf,str_from2)<=0) //duplicates not allowed hence using equal 
								{
									if(strcmp(temp_buf,str_from2)==0) // remove one of the instance,removing str_From2
									{
										index2=index2+strlen(str_from2);
										ret2=nextString(str_from2,kbuf2,0,index2,byte2);  //str_from1 is already parsed so fill in the new string
										printk("new ret2 value is %d for %d run\n",ret2,i+1);	
									}
									if( (PAGE_SIZE-index3)> strlen(str_from1))  //case when outbuffer has insufficient space
									{	
										strcat(kbuf3,str_from2);
										index3=index3+strlen(str_from2);
										kbuf3[index3++]='\0';
										result.records++; 				//increment records
										printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
								
										//fill temp_buf
										ret4=nextString(temp_buf,str_from2,0,0,strlen(str_from2));
										index4=strlen(str_from2);
										temp_buf[index4++]='\0';
								
										//update index and check for new string
										index2+=strlen(str_from2);
										ret2=nextString(str_from2,kbuf2,0,index2,byte2);  //str_from1 is already parsed so fill in the new string
										printk("new ret2 value is %d for %d run\n",ret2,i+1);			
									}
									else
									{
										temp=vfs_write(filp3, kbuf3,strlen(kbuf3), &pos3);
										if(temp<0)
										{
											printk(KERN_ALERT "File write error writing from str_from2\n");
											goto out;
										}
										else
										{
											index3=0;
											strcat(kbuf3,str_from2);
											index3=index3+strlen(str_from2);
											kbuf3[index3++]='\0';
											result.records++; 				//increment records
											printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
								
											//fill temp_buf
											ret4=nextString(temp_buf,str_from2,0,0,strlen(str_from2));
											index4=strlen(str_from2);
											temp_buf[index4++]='\0';
								
											//update index and check for new string
											index2+=strlen(str_from2);
											ret2=nextString(str_from2,kbuf2,0,index2,byte2);  //str_from1 is already parsed so fill in the new string
											printk("new ret2 value is %d for %d run\n",ret2,i+1);
										}
									}	
						
								}
								else  // case when incoming string is smaller than already existing in the outbuffer, tflag will return an error
								{				
								
									result.err=-EINVAL;
									printk("str_from2:%s, found to be lesser than temp_buf:%s, error case in a->i->t\n",str_from2,temp_buf);
									goto out;				
								}
						}  // end of u & t
				}
				else //just u, normal compare
				{
					if(strcmp(str_from1,str_from2)<=0)  //both strings can be same as well, remove one instance following u flag constraint
						{
							if(strcmp(str_from1,str_from2)==0) // remove one of the instance,removing str_From2
								{
									index2=index2+strlen(str_from2);
									ret2=nextString(str_from2,kbuf2,0,index2,byte2);  //str_from1 is already parsed so fill in the new string
									printk("new ret2 value is %d for %d run\n",ret2,i+1);	
								}
								if(strcmp(temp_buf,str_from1)<=0)
								{
									if(strcmp(temp_buf,str_from1)==0) // remove one of the instance,removing str_From2
									{
										index1=index1+strlen(str_from1);
										ret1=nextString(str_from1,kbuf1,0,index1,byte1);  //str_from1 is already parsed so fill in the new string
										printk("new ret2 value is %d for %d run\n",ret2,i+1);	
									}
									if( (PAGE_SIZE-index3)> strlen(str_from1))
									{	
										strcat(kbuf3,str_from1);
										index3=index3+strlen(str_from1);
										kbuf3[index3++]='\0';
										result.records++; 				//increment records
										printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
								
										//fill temp_buf
										ret4=nextString(temp_buf,str_from1,0,0,strlen(str_from1));
										index4=strlen(str_from1);
										temp_buf[index4++]='\0';
								
										//update index and check for new string
										index1=index1+strlen(str_from1);
										ret1=nextString(str_from1,kbuf1,0,index1,byte1);  //str_from1 is already parsed so fill in the new string
										printk("new ret1 value is %d for %drun\n",ret1,i+1);			
									}
									else
									{
										temp=vfs_write(filp3, kbuf3,strlen(kbuf3), &pos3);
										if(temp<0)
										{
											printk(KERN_ALERT "File write error writing from str_from1\n");
											goto out;
										}
										else
										{
											index3=0;
											strcat(kbuf3,str_from1);
											index3=index3+strlen(str_from1);
											kbuf3[index3++]='\0';
											result.records++; 				//increment records
											printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
								
											//fill temp_buf
											ret4=nextString(temp_buf,str_from1,0,0,strlen(str_from1));
											index4=strlen(str_from1);
											temp_buf[index4++]='\0';
								
											//update index and check for new string
											index1=index1+strlen(str_from1);
											ret1=nextString(str_from1,kbuf1,0,index1,byte1);  //str_from1 is already parsed so fill in the new string
											printk("new ret1 value is %d for %drun\n",ret1,i+1);
										}
									}	
								}
								else // str_from1 should be greater than temp_buf skipping
								{
									index1=index1+strlen(str_from1);
									ret1=nextString(str_from1,kbuf1,0,index1,byte1);  //str_from1 is already parsed so fill in the new string
									printk("new ret1 value is %d for %drun\n",ret1,i+1);
								}
						}
						else
						{
							if(strcasecmp(temp_buf,str_from2)<=0) //duplicates not allowed hence using equal 
								{
									if(strcasecmp(temp_buf,str_from2)==0) // remove one of the instance,removing str_From2
										{
											index2=index2+strlen(str_from2);
											ret2=nextString(str_from2,kbuf2,0,index2,byte2);  //str_from1 is already parsed so fill in the new string
											printk("new ret2 value is %d for %d run\n",ret2,i+1);	
										}
									if( (PAGE_SIZE-index3)> strlen(str_from1))  //case when outbuffer has insufficient space
									{	
										strcat(kbuf3,str_from2);
										index3=index3+strlen(str_from2);
										kbuf3[index3++]='\0';
										result.records++; 				//increment records
										printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
								
										//fill temp_buf
										ret4=nextString(temp_buf,str_from2,0,0,strlen(str_from2));
										index4=strlen(str_from2);
										temp_buf[index4++]='\0';
								
										//update index and check for new string
										index2+=strlen(str_from2);
										ret2=nextString(str_from2,kbuf2,0,index2,byte2);  //str_from1 is already parsed so fill in the new string
										printk("new ret2 value is %d for %d run\n",ret2,i+1);			
									}
									else
									{
										temp=vfs_write(filp3, kbuf3,strlen(kbuf3), &pos3);
										if(temp<0)
											{
												printk(KERN_ALERT "File write error writing from str_from2\n");
												goto out;
											}
										else
										{
											index3=0;
											strcat(kbuf3,str_from2);
											index3=index3+strlen(str_from2);
											kbuf3[index3++]='\0';
											result.records++; 				//increment records
											printk(KERN_ALERT"++++kbuf3+++++ =%s",kbuf3);
								
											//fill temp_buf
											ret4=nextString(temp_buf,str_from2,0,0,strlen(str_from2));
											index4=strlen(str_from2);
											temp_buf[index4++]='\0';
								
											//update index and check for new string
											index2+=strlen(str_from2);
											ret2=nextString(str_from2,kbuf2,0,index2,byte2);  //str_from1 is already parsed so fill in the new string
											#ifdef DEBUGON
											printk("new ret2 value is %d for %d run\n",ret2,i+1);
											#endif
										}
									}	
						
								}
								else  // case when incoming string is smaller than already existing in the outbuffer, update the buffer
								{				
									index2=index2+strlen(str_from2);
									ret2=nextString(str_from2,kbuf2,0,index2,byte2);  //str_from1 is already parsed so fill in the new string
									#ifdef DEBUGON
									printk("new ret2 value is %d for %d run\n",ret2,i+1);
									#endif
								}
					}
				}
			}	
			else
			{
				printk("Control should not come here, meaning none of a & u set\n");
			}
		  			
		} // end of inner while
	// end of inner while, check for cases where kbuf1 and kbuf2 might have some character left
		//Assuming if there is an string with data without \n its vfs_read will return some byte
				
		if(ret1<0)
		{
			byte1=vfs_read(filp1,kbuf1,PAGE_SIZE,&pos1);
			if(byte1)
			{
				index1=0;
				ret1=nextString(str_from1,kbuf1,0,index1,byte1);
			}
			else
			{
				if(ret2 >0 || ret2==0)
					{
						strcat(kbuf3,str_from2);
						index3=index3+strlen(str_from2);
						kbuf3[index3++]='\0';
						result.records++; 	
						#ifdef DEBUGON
						printk(KERN_ALERT"++++kbuf++++3=%s\n",kbuf3);
						#endif
					}
					break;
			}
			
		}
		if(ret2<0)
		{
			
			byte2=vfs_read(filp2,kbuf2,PAGE_SIZE,&pos2);
			if(byte2)
			{
				index2=0;
				ret2=nextString(str_from2,kbuf2,0,index2,byte2);
			}
			else
			{
				if(ret1 >0 || ret1==0)
					{
						strcat(kbuf3,str_from1);
						index3=index3+strlen(str_from1);
						kbuf3[index3++]='\0';
						result.records++; 	
						printk(KERN_ALERT"++++kbuf++++3=%s\n",kbuf3);						
					}
				break;
			}
			
		}
		
		if(ret2>0)
		{
			printk("filling kbuf2 for second file...\n");
			byte2=vfs_read(filp2,kbuf2,PAGE_SIZE,&pos2);
			index2=0;
			if(byte2)
			ret2=nextString(str_from2,kbuf2,strlen(str_from2),index2,byte2);
			else
			{
				printk("syscall: Error point reached buffer2 has no \\n terminated string at the end of the page\n");
				result.err=-1;
				goto out;
				
			}
		}
		
		
		if(ret1>0)  //some part of string 1 is there
		{
			printk("str_from2 has records so filling buffer1\n");
			byte1=vfs_read(filp1,kbuf1,PAGE_SIZE,&pos1);
			index1=0;
			if(byte1)
				ret1=nextString(str_from1,kbuf1,strlen(str_from1),index1,byte1);
			else
			{
				printk("syscall: Error point reached buffer1 has no \\n terminated string at the end of the page\n");
				result.err=-1;
				goto out;
					
			}	
		}	
		
	}while(byte1>0 && byte2>0);   //end of outer 	do-while
	/*if(byte1>0 && byte2>0)
	{
		printk("error occured outer while loop skipped \n");
		result.err=-1;
		goto out;
	}	*/
	#ifdef DEBUGON
	printk(KERN_ALERT"exiting while with kbuf size as %zu\n",strlen(kbuf3));
	#endif
	if( index3>0)
	{
		byte3=vfs_write(filp3, kbuf3, strlen(kbuf3), &pos3);
		memset(kbuf3,'\0',PAGE_SIZE*sizeof(char));
	}
	
	if(byte1>0)
		{
			index1+=strlen(str_from1);
			kbuf1[byte1]='\0';
			strncat(kbuf3,kbuf1+index1,strlen(kbuf1)-index1);
			byte3=vfs_write(filp3, kbuf3,strlen(kbuf1)-index1, &pos3);
			result.records++;  //updating record count
			if(byte3<0)
			{
				result.err=-1;
				goto out;
			}	
			/*while(byte1>0)
				{
					byte1=vfs_read(filp1,kbuf1,PAGE_SIZE,&pos1);
					byte3=vfs_write(filp3, kbuf1,byte1, &pos3);
					result.records++;  //updating record count
				}*/
			
		}
	
	if(byte2>0)
	{
			index2+=strlen(str_from2);
			kbuf2[byte2]='\0';
			strncat(kbuf3,kbuf2+index2,strlen(kbuf2)-index2);
			byte3=vfs_write(filp3, kbuf3,strlen(kbuf2)-index2, &pos3);
			result.records++;  //updating record count
			if(byte3<0)
			{
				result.err=-1;
				goto out;
			}	
			/*while(byte2>0)
				{
					byte1=vfs_read(filp1,kbuf2,PAGE_SIZE,&pos1);
					byte3=vfs_write(filp3, kbuf2,byte2, &pos3);
					result.records++;  //updating record count
				}	*/
	}

	out:
	if(kbuf1 !=NULL)
		kfree(kbuf1);
		
	if(kbuf2!=NULL)
		kfree(kbuf2);
		
	if(kbuf3!=NULL)
		kfree(kbuf3);
		
	if(temp_buf!=NULL)
		kfree(temp_buf);	
		
	if(str_from1!=NULL)
		kfree(str_from1);
		
	if(str_from2!=NULL)
		kfree(str_from2);	

	return result;	
	
}

/*
*
* Define Main_xmergesort(void *arg)
*
*/
asmlinkage long xmergesort(void *arg)
{
	/* dummy syscall: returns 0 for non null, -EINVAL for NULL */
	
	
	//Declarations:
	char * tempfile;
	umode_t		outmode,inmode;
	myargs *input;	
	internal result;
	mm_segment_t old_fs=get_fs();
	struct kstat ifstat;
	struct file *filp1 =NULL;
	struct file *filp2 =NULL;
	struct file *filp3 =NULL;
	struct file *outfile=NULL; //to create the final file 
	int retval=0;
	filp1 =NULL;filp2 =NULL;filp2 =NULL,input=NULL;
	//char *temp_bug2=NULL;
	
	result.records=0;
	result.err=0;
	
	
	
	printk(KERN_ALERT "xmergesort received arg %p\n", arg);
		
	//1.Validate input
	
	/* Check if the pointer itself is not null and can be accessed */
	if (NULL == arg || !(access_ok(VERIFY_READ, arg,sizeof(arg)))) {
		printk(KERN_ALERT "Arguements from user space are invalid\n");
		retval = -EINVAL;
		goto out;
	}
	
	input=(myargs *)kmalloc(sizeof(myargs),GFP_KERNEL);
	if(!input)
		return -ENOMEM;
	//input->data=(int *)kmalloc(sizeof(int),GFP_KERNEL); //for d 	
		
	retval=copy_from_user((void*)input,arg,sizeof(myargs));
	
	 if(retval>0)
		{
			return -EFAULT;
		}
	#ifdef DEBUGON
	printk("flag received are %d\n",input->flags);	
	#endif
	
	
	//flag validation
	if(!input->flags || input->flags > 57)
	{
		retval=-EINVAL;
		printk("Mergecall:no flags set or invalid, returning \n");
		goto out;
	}
	else
	{
		if((input->flags & flaga)|| (input->flags & flagu))
		{
			if((input->flags & flaga)&&(input->flags & flagu))
			{
				retval=-EINVAL;
				printk("Mergecall:a & u set terminating... \n");
				goto out;
			}
		}
		else
		{
			retval=-EINVAL;
			printk("Mergecall: Either of a or u should be set, terminating\n ");
			goto out;
		}
	}  //flag validation complete

	 //2. check files before open	
	vfs_stat(input->files[1], &ifstat);
	 if((ifstat.mode & S_IFMT)!= S_IFREG)
	 {
		printk( KERN_ALERT"output file is not a regular file\n");
		retval=-1;
		goto out;
	 }
	 
	vfs_stat(input->files[2], &ifstat);
	 if((ifstat.mode & S_IFMT)!= S_IFREG)
	 {
		printk( "input file1 is not a regular file\n");
		retval=-1;
		goto out;
	 }

	 if (input->files[1]==NULL) {
		printk(KERN_ALERT "filename 1 is null. Aborting....\n");
		retval = -EINVAL;
		goto out;
	}
	 filp1 = filp_open(input->files[1], O_RDWR | O_LARGEFILE, 0);
	if (IS_ERR(filp1))
		{
			printk(KERN_ALERT"file1 read error%d\n", (int) PTR_ERR(filp1));
			retval=PTR_ERR(filp1); 		  //-ENOENT
			goto out;
		}
	else
	#ifdef DEBUGON
		printk("File %s opened properly \n",input->files[1]);
	#endif
	
	if (input->files[2]==NULL) {
		printk(KERN_ALERT "filename 2 is not present. Aborting....\n");
		retval = -EINVAL;
		goto out;
	}
	filp2 = filp_open(input->files[2], O_RDWR | O_LARGEFILE, 0);
	if (IS_ERR(filp2))
		{
			printk(KERN_ALERT"file2 read error%d\n", (int) PTR_ERR(filp2));
			retval=PTR_ERR(filp2);
			goto out;
		}	
	else
		printk(KERN_ALERT"file2 opened properly %s\n",input->files[2]);
		
	/*if( (filp1->f_inode==filp2->f_inode) || (filp3->f_inode==filp1->f_inode)||(filp3->f_inode==filp2->f_inode))
	 {
		printk(KERN_ALERT"some of the files are same\n");
		retval=-1;
		goto out;
	 }		
	 */
	 
	 /* Preserve the credentials of input file */
	 inmode=filp1->f_path.dentry->d_inode->i_mode;
	 
	 
	 /* check if output file exists, if yes copy its mode else copy the mode of input file*/
	/* if (!(vfs_stat(input->files[0], &outstat)))
		outmode = outstat.mode;
	else*/
		outmode = inmode;
		
	
	 
	 #ifdef DEBUGON
		printk(KERN_ALERT "Arguements are valid \n");
	#endif
	 
	  
	 
	 /* Create the name for the temporarty file in
		which data is to be written initially
	*/
	tempfile = kmalloc(strlen(input->files[0]) + 5,
					GFP_KERNEL);

	strcpy(tempfile,input->files[0]);
	tempfile[strlen(input->files[0])]		 = '.';
	tempfile[strlen(input->files[0]) + 1] 	 = 't';
	tempfile[strlen(input->files[0]) + 2] 	 = 'm';
	tempfile[strlen(input->files[0]) + 3] 	 = 'p';
	tempfile[strlen(input->files[0]) + 4] 	 = '\0';

	#ifdef DEBUGON
		printk ("Temp file is %s \n", tempfile);
	#endif
	 
	 
	 //
	 
	 filp3 = filp_open(tempfile, O_WRONLY | O_LARGEFILE|O_CREAT | O_TRUNC, outmode);
	if (IS_ERR(filp3))
		{
			printk(KERN_ALERT "output file opening error%d\n", (int) PTR_ERR(filp3));
			retval=PTR_ERR(filp3);
			goto out;
		}	
	else
	#ifdef DEBUGON
		printk("output file opened properly %s\n",input->files[0]);
	#endif	
	
	
	old_fs = get_fs();
	set_fs(KERNEL_DS);	
		
	result=xmergesortInternal(filp1,filp2,filp3,input);
	set_fs(old_fs);
	retval=result.err;
	if(retval<0)		
		goto out;
	
	#ifdef DEBUGON
	printk("Records written %d\n",result.records);	
	#endif
	
	//copying the data to the user space
	if(input->flags & flagd)
	{
	
		if(copy_to_user(input->data,&result.records, sizeof(int)))
		 {
			retval = -EINVAL;
			printk("copy to user failed\n");
			goto out;
			}
	}
	
	
	/* Open the output file now to perform the rename operation.
		Use appropriate credentials as decided before
	 */

	outfile = filp_open(input->files[0],
				O_WRONLY | O_LARGEFILE|O_CREAT | O_TRUNC,outmode);
	if (!outfile) 
	{
		retval = -ENOENT;
		goto out;
	}
	/* Rename the temporary output file to the final output file.
		 Obtain the lock on the parent of the files. The following
			implementation is adabted from
		 ~/fs/wrapfs/inode.c
		 and has been changed as per needs of assignment
	*/
		
	lock_rename(filp3->f_path.dentry->d_parent,
				outfile->f_path.dentry->d_parent);

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	retval = vfs_rename(filp3->f_path.dentry->d_parent->d_inode,
			filp3->f_path.dentry,
			outfile->f_path.dentry->d_parent->d_inode,
			outfile->f_path.dentry, NULL, 0);

	set_fs(old_fs);
	unlock_rename(filp3->f_path.dentry->d_parent,
				 outfile->f_path.dentry->d_parent);
				 
	if (retval) {
		printk(KERN_ALERT "Error in rename temp file to permanent output file\n");
		goto out;
	} 

	retval=0;	

	//6. Exit with status
		
	out:
	
	/* In case of partial failure, the temporary file is
		supposed to be removed
	*/
	if(retval<0)
	{
		if(filp3)
		{
			#ifdef DEBUGON
			printk(KERN_ALERT "Deleting file %s \n", tempfile);
			#endif
			old_fs = get_fs();
			set_fs(KERNEL_DS);
			retval = vfs_unlink(filp3->f_path.dentry->d_parent->d_inode,
					 filp3->f_path.dentry, NULL);
			set_fs(old_fs);
			printk(KERN_ALERT "Could not create file \n");
		}
	}
	
	
	if(filp1)	
		filp_close(filp1,NULL);
		
	if(filp2)
		filp_close(filp2,NULL);
		
	if(filp3)
		filp_close(filp3,NULL);
	if(outfile)
		filp_close(outfile,NULL);
	
	//if(input->data!=NULL)
		//kfree(input->data);
	if(input!=NULL)
		kfree(input);	
	
	return retval;
 	
}

static int __init init_sys_xmergesort(void)
{
	printk("installed new sys_xmergesort module\n");
	if (sysptr == NULL)
		sysptr = xmergesort;
	return 0;
}
static void  __exit exit_sys_xmergesort(void)
{
	if (sysptr != NULL)
		sysptr = NULL;
	printk("removed sys_xmergesort module\n");
}
module_init(init_sys_xmergesort);
module_exit(exit_sys_xmergesort);
MODULE_LICENSE("GPL");
