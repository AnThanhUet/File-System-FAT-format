#ifndef _HAL_h_
#define _HAL_h_

#include <stdio.h>
#include <stdlib.h>

/*The function to open the file and return the result of successful file opening */
int hal_open_file();

/* The function reads the file data and writes to the buffer */
void hal_read_multil_sector(int numSector,unsigned int byteRead,unsigned char *DataBuff);

/* The function to close the file*/
void hal_close_file();

#endif

