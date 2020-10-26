#include "FAT.h"

#define BYTE_OFFSET_DATE      0x18                                          /* Byte offset of the date modified*/
#define LENGTH_BYTE_DATE      2                                             /* Length(byte) of the date modified*/
#define MASK_DAY              0b00011111                                    /* Bit offset of day (1-31):  0; Length(bit): 5*/
#define BIT_OFFSET_MONTH      5                                             /* Bit offset of month (1-12):  5; Length(bit): 4*/
#define MASK_MONTH            0b00001111
#define BIT_OFFSET_YEAR       9                                             /* Bit offset of year-1980 (0-127): 9; Length(bit): 7*/

#define BYTE_OFFSET_SECTOR                 0x0B                                         /* Byte offset of the bytes per Sector*/
#define LENGTH_BYTE_SECTOR                    2                                         /* Length(byte) of the bytes per Sector*/
#define BYTE_OFFSET_SECTOR_CLUSTER         0x0D                                         /* Byte offset of the sectors per Cluster*/
#define LENGTH_BYTE_SECTOR_CLUSTER            1                                         /* Length(byte) of the sectors per Cluster*/
#define BYTE_OFFSET_SECTOR_RESERVED        0x0E                                         /* Byte offset of the sectors in reserved logical sectors*/
#define LENGTH_BYTE_SECTOR_RESERVED           2                                         /* Length(byte) of the sectors in reserved logical sectors*/
#define BYTE_OFFSET_NUMBER_FAT             0x10                                         /* Byte offset of the number File Allocation Tables*/
#define LENGTH_BYTE_NUMBER_FAT                1                                         /* Length(byte) of the number File Allocation Tables*/
#define BYTE_OFFSET_SECTOR_FAT             0x16                                         /* Byte offset of the sectors per File Allocation Tables*/
#define LENGTH_BYTE_SECTOR_FAT                2                                         /* Length(byte) of the sectors per File Allocation Tables*/
#define BYTE_OFFSET_ENTRY_ROOT_DIR         0x11                                         /* Byte offset of the entries count of Root Directory*/
#define LENGTH_BYTE_ENTRY_ROOT_DIR            1                                         /* Length(byte) of the entries count of Root Directory*/
#define BYTE_PER_ENTRY                      32                                          /* The bytes per Entry*/

#define VALUE_ADD_PARENT_FOLDER   0x2E2E                                                /* Value specifies where the entry contains the parent folder address */
#define VALUE_ADD_CURRENT_FOLDER  0x202E                                                /* Value specifies where the entry contains the folder address */
#define BYTE_OFFSET_ATTRI         0x0B                                                  /* Byte offset of the FAT_Attribute_File */
#define LENGTH_BYTE_ATTRI         2                                                     /* Length(byte) of the FAT_Attribute_File*/
#define VALUE_LONG_FILE_NAME      0x0F                                                  /* Value of attribute "Long filename" */
#define MASK_SUBDIRECTORY         0x10                                                  /* Mask of attribute "Subdirectory" */

#define BYTE_OFFSET_NAME_FILE     0x00                                         /* Byte offset of the name File*/
#define LENGTH_BYTE_NAME_FILE     11                                           /* Length(byte) of the name File*/
#define BYTE_OFFSET_SIZE      0x1C                                          /* Byte offset of the FAT_Size_File */
#define LENGTH_BYTE_SIZE      4                                             /* Length(byte) of the FAT_Size_File */
#define BYTE_OFFSET_ADD      0x1A                                           /* Byte offset of the address cluster file */
#define LENGTH_BYTE_ADD      2                                             /* Length(byte) of the address cluster file */
#define BYTE_OFFSET_TIME      0x16                                          /* Byte offset of the time modified*/
#define LENGTH_BYTE_TIME      2                                             /* Length(byte) of the time_file */
#define BIT_OFFSET_HOURS 	  11                                            /* Bit offset of hours  (0-23): 11; Length(bit): 5 */
#define BIT_OFFSET_MINUTE 	  5                                             /* Bit offset of minute (0-59):  5; Length(bit): 6 */
#define MASK_MINTE  		  0b00111111
#define MASK_SECOND 		  0b00011111                                    /* Bit offset of Seconds/2 (0-29):  0; Length(bit): 5 */
/* The function change from Little endian to Big endian */
unsigned int FAT_Convert_BigEndian(unsigned int ByteOffset,unsigned int LengthByte,unsigned char *DataBuff)
{
	unsigned int addByte  = LengthByte + ByteOffset, BigEndian = 0;
	for(addByte; addByte > ByteOffset; addByte--)
		BigEndian = (BigEndian << 8) | (*(DataBuff + addByte - 1));
	return BigEndian;
}

/* The function read paremeter of FAT12 */
struct Paremeter fat_read_boot(unsigned char *DataBuff)
{
	struct Paremeter FAT12;
	FAT12.byteSector = FAT_Convert_BigEndian(BYTE_OFFSET_SECTOR,LENGTH_BYTE_SECTOR,DataBuff);
	unsigned int sectorReserved = FAT_Convert_BigEndian(BYTE_OFFSET_SECTOR_RESERVED,LENGTH_BYTE_SECTOR_RESERVED,DataBuff);
	unsigned int sectorFAT = FAT_Convert_BigEndian(BYTE_OFFSET_NUMBER_FAT,LENGTH_BYTE_NUMBER_FAT,DataBuff) * FAT_Convert_BigEndian(BYTE_OFFSET_SECTOR_FAT,LENGTH_BYTE_SECTOR_FAT,DataBuff);
	unsigned int sectorRootDirec = FAT_Convert_BigEndian(BYTE_OFFSET_ENTRY_ROOT_DIR,LENGTH_BYTE_ENTRY_ROOT_DIR,DataBuff) * BYTE_PER_ENTRY / FAT12.byteSector; 
	FAT12.numSectorPerCluster = FAT_Convert_BigEndian(BYTE_OFFSET_SECTOR_CLUSTER,LENGTH_BYTE_SECTOR_CLUSTER,DataBuff);
	FAT12.byteCluster = FAT12.byteSector * FAT12.numSectorPerCluster;
	FAT12.sectorFirstFAT = sectorReserved;
	FAT12.sectorFirstRootDirec = sectorReserved + sectorFAT;
	FAT12.sectorFirstData = sectorReserved + sectorFAT + sectorRootDirec - 2;	
	return FAT12;
}


int fat_attribute_file(unsigned int numEntry,unsigned char *DataBuff)
{
	unsigned int Attribute = SPEC_TYPE_FILE;
	unsigned int addAttriFile = numEntry * BYTE_PER_ENTRY + BYTE_OFFSET_ATTRI;       /*The address of the BYTE contains the file attribute : Byte offset: 0x0B; Length(byte): 1 */
	if(FAT_Convert_BigEndian(BYTE_CHECK_FOLDER + numEntry* BYTE_PER_ENTRY,LENGTH_BYTE_ATTRI,DataBuff) == VALUE_ADD_PARENT_FOLDER) Attribute = SPEC_ENTRY_PARENT_FOLDER;
	else if((*(DataBuff + addAttriFile) == VALUE_LONG_FILE_NAME) || 
	(FAT_Convert_BigEndian(BYTE_CHECK_FOLDER + numEntry* BYTE_PER_ENTRY,LENGTH_BYTE_ATTRI,DataBuff) == VALUE_ADD_CURRENT_FOLDER)) Attribute = SPEC_ENTRY_IGNORE;
	else if(*(DataBuff + addAttriFile) & MASK_SUBDIRECTORY) Attribute = SPEC_TYPE_SUBDIRECTORY; 
	return Attribute;
}


/* The function read name of file */
static void Name_File(unsigned int numEntry,unsigned char *DataBuff)
{
	unsigned int numByte;
	for (numByte = BYTE_OFFSET_NAME_FILE; numByte < LENGTH_BYTE_NAME_FILE; numByte++)		
		printf("%c",*(DataBuff + numByte + numEntry * BYTE_PER_ENTRY));		/* The statement reads characters of the file name and extension */
}

/* The function read size of file*/
unsigned int fat_size_file(unsigned int numEntry,unsigned char *DataBuff)
{
	unsigned int size = FAT_Convert_BigEndian(numEntry * BYTE_PER_ENTRY + BYTE_OFFSET_SIZE, LENGTH_BYTE_SIZE,DataBuff);
	return size;
}

/* The function read address cluster of file*/
unsigned int fat_add_file(unsigned int numEntry, unsigned char *DataBuff)
{
	unsigned int add = FAT_Convert_BigEndian(numEntry * BYTE_PER_ENTRY + BYTE_OFFSET_ADD, LENGTH_BYTE_ADD, DataBuff);
	return add;
}

/* The function calculation time*/
static void Time_File(unsigned int numEntry, unsigned char *DataBuff) 
{
	unsigned int time = FAT_Convert_BigEndian(numEntry * BYTE_PER_ENTRY + BYTE_OFFSET_TIME, LENGTH_BYTE_TIME, DataBuff);
	printf("%02d:%02d:%02d\t",time >> BIT_OFFSET_HOURS,(time >> BIT_OFFSET_MINUTE)&MASK_SECOND,(time & MASK_SECOND)*2);
}
/* The function calculation date */
static void Date_File(unsigned int numEntry, unsigned char *DataBuff)
{
	unsigned int date = FAT_Convert_BigEndian(numEntry * BYTE_PER_ENTRY + BYTE_OFFSET_DATE, LENGTH_BYTE_DATE, DataBuff);
	printf("\t%02d/%02d/%04d ",date & MASK_DAY,(date >> BIT_OFFSET_MONTH) & MASK_MONTH, (date >> BIT_OFFSET_YEAR) + 1980);
}

int fat_show_folder_infor(unsigned char *DataBuff)
{
	int sumFile = 0;
	int numEntry = 0;                                                     /*The variable counts the number of ENTRY that have been read */
	printf("\nKey. \tName    \tDate modified       \tSize\n");
	while(*(DataBuff + numEntry * BYTE_PER_ENTRY) != 0)
	{
		if(fat_attribute_file(numEntry,DataBuff) == SPEC_TYPE_FILE)
		{
			sumFile ++;                                                 /*The count command calculates the total number of files in the fordel */
			printf("%d.\t",sumFile);
			Name_File(numEntry, DataBuff);
			Date_File(numEntry, DataBuff);
			Time_File(numEntry, DataBuff);
			printf("%d\t", fat_size_file(numEntry, DataBuff));
			printf("\n");
		}
		if(fat_attribute_file(numEntry,DataBuff) == SPEC_TYPE_SUBDIRECTORY)
		{
			sumFile ++;                                                 /*The count command calculates the total number of files in the fordel */
			printf("%d.\t",sumFile);
			Name_File(numEntry, DataBuff);
			Date_File(numEntry, DataBuff);
			Time_File(numEntry, DataBuff);
			printf("\n");
		}
		if(fat_attribute_file(numEntry,DataBuff) == SPEC_ENTRY_PARENT_FOLDER)
		{
			sumFile ++;                                                 /*The count command calculates the total number of files in the fordel */
			printf("%d.\t",sumFile);
			printf("\t\t GO TO BACK PARENT DIRECTORY ");
			printf("\n");
		}
		numEntry ++;
	}
	return sumFile;
}

void show_file(unsigned int sizeFile, unsigned char *DataBuff)
{
	unsigned int addByte;
	printf("\n");	
	for (addByte = 0; addByte < sizeFile; addByte++)
		printf("%c",*(DataBuff + addByte));		/* The statement reads characters of the file */				
	printf("\n");
}
