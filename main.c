#include "HAL.h"
#include "FAT.h"

#define ADD_FIRS_BYTE_BIOS    0x00                   /* The address of the first byte in BIOS Parameter Block */
#define NUMBER_BYTE_BIOS       512                   /* The number byte of BIOS Parameter Block */
#define VALUE_SPECIFIC_FILE     -1                   /*The value specifies the condition as a file */
#define END_CHAIN_INDICATOR    0xFFF                 /* The value End of chain indicator in FAT*/

static unsigned char *s_DataBuff;                    /* The Buffer array for storing the address mapping values of a Sector */
/*The variable stores the total value of the file in a directory, if the directory is a file then the variable has the value of VALUE_SPECIFIC_FILE*/
static int s_SumFile;
/* The variables store initial parameters */
static struct Paremeter s_FAT12;
/* The function enters the file name to be read */
static void File_Init(char *fileName)
{
	printf("\t\t\tFAT file system\n");
	printf("********************************************\n");
	printf("\nEnter name file: ");
	fflush(stdin);
	scanf("%s",fileName); 
}
/* finds the Cluster the address mapping in the FAT region */
static unsigned int Find_Clustermap(unsigned int ValueCluster)
{
	unsigned char *FATBuff;                                                 /* The Buffer array for storing the address mapping values of Cluster in FAT */
	/*The Sector address variable need read in the FAT region */
	unsigned int  addSector = ((ValueCluster + (ValueCluster >> 1)) / s_FAT12.byteCluster) * s_FAT12.numSectorPerCluster;
	/* The address variable of the Cluster mapping value in the FAT */
	unsigned int addClusterMap = (ValueCluster + (ValueCluster >> 1)) % s_FAT12.byteSector;
	/* The statement checks whether the position condition of the Sector exceeds the FAT area or not */
	if(addSector < s_FAT12.sectorFirstRootDirec)                           /* If not, then execute the script to read value */
	{
		/* The statement checks if the position is at the END of a sector */
		if(ValueCluster == s_FAT12.byteSector) /* If not, read 2 Sector */
		{
			FATBuff = (unsigned char *)malloc(2 * s_FAT12.byteSector * sizeof(unsigned char));
			hal_read_multil_sector(s_FAT12.sectorFirstFAT + addSector,2 * s_FAT12.byteCluster, FATBuff);		
		}
		else /* If yes, it will read one Sector */
		{
			FATBuff = (unsigned char *)malloc(s_FAT12.byteSector * sizeof(unsigned char));
			hal_read_multil_sector(s_FAT12.sectorFirstFAT + addSector, s_FAT12.byteCluster, FATBuff);		
		}
		/* The statement checks whether the Cluster position is even or odd, and calculate the value in it by reverse bit*/
		if(ValueCluster & 1)
		ValueCluster = (*(FATBuff + addClusterMap) >> 4)|(*(FATBuff + addClusterMap + 1) << 4);
		else
		ValueCluster = *(FATBuff + addClusterMap)|((*(FATBuff + addClusterMap + 1) & 0x0F) << 8);
		free (FATBuff);
	}
	else ValueCluster == END_CHAIN_INDICATOR;/* If yes, return the value End "0xFFF" of chain indicator */
	return ValueCluster;
}
/* The function read and show the information the user points to*/
static void Next_File(unsigned int numberFile, unsigned int *ParentAdd)
{
	unsigned int countFile = 0;           /* The Variable to count File numbers */
	unsigned int countEntry = 0;          /* The Variable to count Entry numbers */
	unsigned int CurrentAdd;              /* The Variable to store the current address of the File */
	if(s_SumFile == VALUE_SPECIFIC_FILE)  /* The command check the command from File back parent Directory */
	{
		if(*ParentAdd == 0)               /* The command to check parent directory is the root directory */
		{
			hal_read_multil_sector(s_FAT12.sectorFirstRootDirec, s_FAT12.byteSector, s_DataBuff);
			s_SumFile = fat_show_folder_infor(s_DataBuff);
		}		
		else                              /*If the parent directory is not the root directory */
		{
			CurrentAdd = *ParentAdd;      /* The command assigns the address of the parent directory to the cluster address to read */
			do                            /* The loop reads File information on Cluster */
			{ 	
				hal_read_multil_sector(CurrentAdd + s_FAT12.sectorFirstData, s_FAT12.byteCluster, s_DataBuff);
				s_SumFile = fat_show_folder_infor(s_DataBuff);                    
				CurrentAdd = Find_Clustermap(CurrentAdd);
			}
			while(CurrentAdd != END_CHAIN_INDICATOR);
		}
	}
	else /* The case from Directory to File or Subfolder */
	{
		while(numberFile != countFile) /* The loop calculates the File and Entry position of the File to want read in the parent directory */
		{
			if(fat_attribute_file(countEntry, s_DataBuff) != SPEC_ENTRY_IGNORE) /* The statement check entry's attribute  is a File*/
				countFile ++;
			countEntry ++;
		}
		countEntry --;
		if (fat_attribute_file(countEntry, s_DataBuff) == SPEC_TYPE_FILE)         /* The statement check case read and show to File */
		{	
			*ParentAdd = fat_add_file(BYTE_CHECK_FOLDER, s_DataBuff);             /* The statement saves the file's parent directory address */
			CurrentAdd = fat_add_file(countEntry, s_DataBuff);                    /* The statement reads the file's address */
			do /* The loop prints out file data on clusters */
			{ 
				hal_read_multil_sector(CurrentAdd + s_FAT12.sectorFirstData, s_FAT12.byteCluster, s_DataBuff);		
				show_file(s_FAT12.byteSector,s_DataBuff);
				CurrentAdd = Find_Clustermap(CurrentAdd);                         /* The statement assigns the value specified in the file format */
			}
			while(CurrentAdd != END_CHAIN_INDICATOR);
			s_SumFile = VALUE_SPECIFIC_FILE;
			printf("1.\t\t GO TO BACK PARENT DIRECTORY \n");
		}
		else /* The statement check case read and show to Directory */
		{
			if(fat_add_file(countEntry, s_DataBuff) != 0) /* The statement check case read and show to directory not root directory */
			{			
				CurrentAdd = fat_add_file(countEntry, s_DataBuff);/* The statement reads the directory's address */
				do /* The loop prints out directory data on clusters */
				{ 
					hal_read_multil_sector(s_FAT12.sectorFirstData + CurrentAdd, s_FAT12.byteCluster, s_DataBuff);
					s_SumFile = fat_show_folder_infor(s_DataBuff);
					CurrentAdd = Find_Clustermap(CurrentAdd);
				}
				while(CurrentAdd != END_CHAIN_INDICATOR);
			}
			else /* The case read and show to root directory */
			{
				hal_read_multil_sector(s_FAT12.sectorFirstRootDirec, s_FAT12.byteSector, s_DataBuff);
				s_SumFile = fat_show_folder_infor(s_DataBuff);
			}
		}
	}			
}
int main() 
{
	char fileName[20];
	unsigned char key;              /* Keyword executable with program */
	unsigned int ParentAdd = 0;     /* The variable stores the address of the parent directory */
	/* The statement calls the function enters the file name to be read */
	File_Init(fileName);
	if( hal_open_file(fileName) == 1) /*The statement calls the function to open the file and check the result of successful file opening */
	{ 
		/* The function call allocates the initial 512 bytes to the buffer variable */
		s_DataBuff = (unsigned char *)malloc(NUMBER_BYTE_BIOS * sizeof(unsigned char));
		/* The statement calls the function read paremeter of FAT12 */
		hal_read_multil_sector(ADD_FIRS_BYTE_BIOS,NUMBER_BYTE_BIOS,s_DataBuff);
		/* The statement saves the parameter's value */
		s_FAT12 = fat_read_boot(s_DataBuff);
		/* The statements read and show root directory information */
		s_DataBuff = (unsigned char *)realloc(s_DataBuff, s_FAT12.byteCluster * sizeof(unsigned char));
		hal_read_multil_sector(s_FAT12.sectorFirstRootDirec,s_FAT12.byteSector,s_DataBuff);
		s_SumFile = fat_show_folder_infor(s_DataBuff);
		/* The loop for executing the file read command is selected by the importer */
		do
		{
			printf("\n>> Enter 'Key' choose the apropariate file === choose '0' to exit the program:");
			scanf("%d",&key);
			if(0 < key && s_SumFile >= key || 1 == key ) Next_File(key,&ParentAdd); /*The statement check if the user entered key is executable */
		}
		while (key != 0); /* Loop exit condition when user enters 0 */
	}
	hal_close_file(); /*The statement calls the function to close the file*/
	return 0;
}
