// Simple program to read content of diretory 

#include <sys/types.h>
#include <sys/stat.h>
#include <cstring>
#include <dirent.h>
#include <cerrno>
#include <cstdio>
#include <limits.h>
#include <cstdlib>
#include <pthread.h>
#include <openssl/md5.h>
#include <map>
#include <string>
#include <vector>

typedef struct argument
{
	char *dirPath;
	int argNum;
} argS; 


std::map<int,std::string> dirRootMap;
typedef struct stat fStat;
typedef std::map<int,std::string>::iterator dr_it;
std::map<std::string,int> fileHash;
typedef std::map<std::string, int>::iterator fh_it;
pthread_mutex_t lock;


/*
	simple function that compares size of two files 
	returns true if both are equal in size and false 
	otherwise. 
*/
bool compareFileSize(const char * path1, const char * path2)
{	bool retVal = true;
	fStat *pathName1Attr = (fStat *)malloc(sizeof(fStat));
	fStat *pathName2Attr = (fStat *)malloc(sizeof(fStat));
	stat(path1,pathName1Attr);
	stat(path2,pathName2Attr);
	if(pathName1Attr->st_size != pathName2Attr->st_size )
		retVal = false;
	free(pathName2Attr);
	free(pathName1Attr);
	return retVal;
}

/*
	simple function that compares MD5 signature of two files
	returns true if both files have indentical content 
	and false otherwise
*/
bool compareFileContent(const char * path1, const char * path2)
{	bool retVal = false;
	unsigned char c[MD5_DIGEST_LENGTH + 1];
	unsigned char d[MD5_DIGEST_LENGTH + 1];
	int i ;
	FILE *inFile = fopen(path1,"rb");
	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];
	if(inFile == NULL){
		perror("path1 can not be opened");
		return false;
	}
	MD5_Init(&mdContext);
	while((bytes = fread(data, 1,1024,inFile)) != 0)
		MD5_Update(&mdContext,data,bytes);
	MD5_Final(c,&mdContext);
	fclose(inFile);
	inFile = fopen(path2,"rb");
	if(inFile == NULL){
		perror("path2 can not be opened");
		return false;
	}
	MD5_Init(&mdContext);
	while((bytes = fread(data, 1, 1024,inFile)) != 0)
		MD5_Update(&mdContext,data,bytes);
	MD5_Final(d,&mdContext);
	fclose(inFile);
	// for( i=0; i < MD5_DIGEST_LENGTH; i++)
	// printf("%02x", c[i]);
	// printf("\n");
	// for( i=0; i < MD5_DIGEST_LENGTH; i++)
	// printf("%02x", d[i]);
	// printf("\n");
	if(memcmp(c,d,MD5_DIGEST_LENGTH) == 0)
	{
		retVal = true;
	}
	return retVal;
}

/* 
	simple function to read the directory content.
	This function will be executed by a thread for each directory.
	It updates the gloab hash table at the end of the traversal.
*/
void *readDir(void *arg)
{	DIR *dirP;
	struct dirent *dirp;
	argS *sarg = (argS *)arg;

	char *dirPath = sarg->dirPath;
	int argNum = sarg->argNum;

	std::string strDirPath(dirPath);
	printf("%s\n", dirPath);
	//printf("%lu Thread Id\n", );
	pthread_mutex_lock(&lock);
	dirRootMap[argNum] = strDirPath;
	pthread_mutex_unlock(&lock);
	std::vector<std::string> files;
	if((dirP = opendir(dirPath)) == NULL) // can't read directory
	{	
		perror(dirPath);	
	}
	else
	{	//printf("reading directory : %s\n",argv[i]);
		
		while((dirp = readdir(dirP) )!= NULL)
		{
			if(strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name,"..") == 0)
				continue;
			else
				files.push_back(std::string(dirp->d_name));
				//printf("%s \t in directory %s\n\v",dirp->d_name,dirPath);
		}
	}
	if(dirP != NULL)
	closedir(dirP);	
	pthread_mutex_lock(&lock);
	std::map<std::string,int>::iterator it;
	for(auto t : files)
	{	
		it = fileHash.find(t);
		if(it != fileHash.end())
		{
			// some other directory has file with same name
			//printf("%s duplicate int val %d\n", t.c_str(),fileHash[t]);
			fileHash[t] = fileHash[t] + argNum;
			//printf("new val %d\n",fileHash[t] );
		}
		else{
			fileHash[t] = argNum;
		}
	}
	pthread_mutex_unlock(&lock);
	return ((void *)0);
}


int main(int argc, char **argv){
	if(argc < 2)
	{
		printf("please provide atleast one directory path. \nTry passing --help option for more details. \n");
	}
	if((argc == 2) && (strcmp("--help", argv[1]) == 0))
	{
		printf("read_dir : simply reads the content of specified directories.\n");
		printf("syntax: read_dir dir_path1 [dir_path2] ... \n");
	}
	else
	{	
		int err;
		pthread_t ntid[argc];
		pthread_mutex_init(&lock,NULL);
		for(int i = 1; i < argc ; i++)
		{	argS *sarg = (argS *)malloc(sizeof(argS));
			sarg->dirPath = argv[i];
			sarg->argNum = i;
			err = pthread_create(&ntid[i],NULL,readDir,(void*)sarg);
			if(err != 0)
			{
				printf("can not create threads\n");
				exit(-1);
			}
			else{
				pthread_join(ntid[i],NULL);
			}
		}
		printf("Reading from hashtable: \n");
		std::string dirPath;
		// for(int i =0 ; i<argc ; i++)
		// {	printf("Thread Id %ld : ",(long)ntid[i]);
		// 	dirPath = dirRootMap[i];
		// 	printf("%s \n",dirPath.c_str());
		// }
		int i ;
		for(fh_it fhIt = fileHash.begin(); fhIt != fileHash.end(); fhIt++)
		{
			for(i = 0; i < argc; i++)
			{
				if(i == 0)
				{
					if(fhIt->second == 3) // hard code value for k = 2 case 
					{ // This means both threads have reported this file name
						printf("%s in both dir ",fhIt->first.c_str());
						// compare size followed by content to check if both files are same of not.
						std::string pathName1 = dirRootMap[1] + "/" + fhIt->first;
						std::string pathName2 = dirRootMap[2] + "/" + fhIt->first;
						if(!compareFileSize(pathName1.c_str(),pathName2.c_str()))
						{	
							printf("size : differs\n");
							break;
						}
						else
						{
							// File size matches , noe need to compare MD5 signature of the contents
							if(compareFileContent(pathName1.c_str(),pathName2.c_str()))
								printf("content : matches\n");
							else
								printf("content : differs\n");
						}
					}
				}
				else
				{
					if(fhIt->second == i)
					{	
						printf("%s \t ---> \t %s\n",fhIt->first.c_str(),dirRootMap[i].c_str());
						break;
					}
				}
			}
		}
				

	}
	return 0;
		
}
		

