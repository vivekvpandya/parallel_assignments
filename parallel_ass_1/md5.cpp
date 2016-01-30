#include <cstdio>
#include <openssl/md5.h>
#include <cerrno>
#include <cstring>
#include <string>

int main(int argc, char **argv)
{
	unsigned char c[MD5_DIGEST_LENGTH + 1];
	unsigned char d[MD5_DIGEST_LENGTH + 1];
	int i ;
	FILE *inFile = fopen(argv[1],"rb");
	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];
	if(inFile == NULL){
		perror("File can not be opened");
		return 0;
	}
	MD5_Init(&mdContext);
	while((bytes = fread(data, 1,1024,inFile)) != 0)
		MD5_Update(&mdContext,data,bytes);
	MD5_Final(c,&mdContext);
	//c[MD5_DIGEST_LENGTH] = '\0';
	//printf("%s \t %s", c,argv[1]);
	fclose(inFile);
	inFile = fopen(argv[2],"rb");
	if(inFile == NULL){
		perror("Second File can not be opened");
		return 0;
	}
	MD5_Init(&mdContext);
	while((bytes = fread(data, 1, 1024,inFile)) != 0)
		MD5_Update(&mdContext,data,bytes);
	MD5_Final(d,&mdContext);
	//d[MD5_DIGEST_LENGTH] = '\0';
	for( i=0; i < MD5_DIGEST_LENGTH; i++)
	printf("%02x", c[i]);
	
	printf("\n");
	
	for( i=0; i < MD5_DIGEST_LENGTH; i++)
	printf("%02x", d[i]);
	
	printf("\n");

	if(memcmp(c,d,MD5_DIGEST_LENGTH) == 0)
	{
		printf("Content of both files matches\n");
	}
	else{
		printf("both files are different \n");
	}
	//printf("\n %s ", argv[i]);
	return 0;
}
