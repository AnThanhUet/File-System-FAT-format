#ifndef _FAT_h_
#define _FAT_h_
#include <stdio.h>

#define BYTE_CHECK_FOLDER  		  0x00                                          /* Byte checks directory address */
#define SPEC_TYPE_FILE            0                                                     /* The value Indicates the entry contains the file type*/
#define SPEC_TYPE_SUBDIRECTORY    1                                                     /* The value Indicates the entry contains the subdirectory type*/
#define SPEC_ENTRY_PARENT_FOLDER  2                                                     /* The value specified  of entry contains the address of the parent directory*/
#define SPEC_ENTRY_IGNORE         3                                                     /* The value specified  of Entry is ignored*/

struct Paremeter
{
	unsigned int byteSector;
	unsigned int numSectorPerCluster;
	unsigned int byteCluster;
	unsigned int sectorFirstFAT;
	unsigned int sectorFirstRootDirec;
	unsigned int sectorFirstData;
};

/* convert from Little endian to Big endian
unsigned int FAT_Convert_BigEndian(unsigned int ByteOffset,unsigned int LengthByte,unsigned char *DataBuff);

/* read paremeter of FAT12 */
struct Paremeter fat_read_boot(unsigned char *DataBuff);

/* check attribute of file */
int fat_attribute_file(unsigned int numEntry,unsigned char *DataBuff);

/* read size of file*/
unsigned int fat_size_file(unsigned int numEntry,unsigned char *DataBuff);

/* read address cluster of file*/
unsigned int fat_add_file(unsigned int numEntry, unsigned char *DataBuff);

/* reads and show the folder's information in the buffer*/
int fat_show_folder_infor(unsigned char *DataBuff);

/* read and show data of file*/
void show_file(unsigned int sizeFile, unsigned char *DataBuff);

#endif

