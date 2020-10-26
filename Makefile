CC = gcc
CFLAGS = -I.

INC_FILES = HAL.h FAT.h

.PHONY: main HAL.o FAT.o main.o 

%.o: %.c $(INC_FILES) 
	$(CC) -c -o $@ $< $(CFLAGS) 

main: main.o HAL.o FAT.o
	$(CC) -o $@ $^ $(CFLAGS) 

HAL.o: HAL.c
	$(CC) -c $< $(CFLAGS)

FAT.o: FAT.c
	$(CC) -c $< $(CFLAGS)  

main.o: main.c
	$(CC) -c $< $(CFLAGS)

clean:
	@rm main main.o HAL.o FAT.o
