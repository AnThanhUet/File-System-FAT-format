#include "HAL.h"
FILE *fp;   

int hal_open_file(char *fileName)
{
	unsigned char check = 0;
	fp = fopen(fileName,"rb");
	if (fp == NULL)
    	printf("Open file fail !!!");
	else check = 1;	
	return check;
}

void hal_read_multil_sector(int numSector,unsigned int byteRead,unsigned char *DataBuff)
{
	fseek (fp, numSector * byteRead, 0);                  
	fread (DataBuff, 1, byteRead, fp);
}

void hal_close_file()
{
	printf("\n\t\tBye bye !");	
	fclose(fp);
}
