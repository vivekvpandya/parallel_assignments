// Simple program to read content of diretory 

#include <sys/types.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <limits.h>

int main(int argc, char **argv){
	if(argc < 2)
	{
		printf("please provide atleast one directory path. try passing --help option for more details. \n");
	}
	if((argc == 2) && (strcmp("--help", argv[1]) == 0))
	{
		printf("read_dir : simply reads the content of specified directories.\n");
		printf("syntax: read_dir dir_path1 [dir_path2] ... \n");
	}
	else
		{	DIR *dirP;
			struct dirent *dirp;
		for(int i = 1; i < argc ; i++)
		{
			if((dirP = opendir(argv[i])) == NULL) // can't read directory
			{	perror(argv[i]);
				
			}
			else
			{	printf("reading directory : %s\n",argv[i]);
				while((dirp = readdir(dirP) )!= NULL)
				{
					if(strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name,"..") == 0)
						continue;
					else
						printf("%s\n",dirp->d_name);
				}
			}
		}
		if(dirP != NULL)
		closedir(dirP);		

	}
	return 0;
}
