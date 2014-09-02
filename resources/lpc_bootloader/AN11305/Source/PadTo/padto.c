/****************************************************************************
 *   $Id:: padto.c 4528 2010-08-25 23:32:22Z nxp21346                        $
 *   Project: Pad a binary file with 0xFFs
 *
 *   Description:
 *     This program adds 0xFF to the end of a binary file before it
 *     is programmed to flash.
 *
 ****************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
****************************************************************************/


#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		printf("padto <filename> <length in kb>\n\n");
		printf("Example:\npadto usbhid.bin 128\n");
		exit(-1);
	}

	int fd = open(argv[1], O_RDWR | O_EXCL);

	if(fd < 0)
	{
		printf("Cannot open file %s\n", argv[1]);
		exit(-1);
	}

	int newlength = atol(argv[2]) * 1024, curlength;
	unsigned char *fbuf = malloc(1024);
	int i;

	if(!fbuf)
	{
		printf("Error allocating memory\n");
		close(fd);
		exit(-1);
	}
	for(i=0;i<1024;i++)
		fbuf[i] = 0xFF;
       
	curlength = lseek(fd, 0, SEEK_END);
	if(curlength < 0)
	{
		printf("Failed to seek to end of file\n");
		close(fd);
		exit(-1);
	}
	if(newlength < curlength)
	{
		printf("Cannot make file smaller\n");
		close(fd);
		exit(-1);
	}
	i = newlength - curlength;
	while(i)
	{
		if(write(fd, fbuf, i > 1024 ? 1024 : i) <= 0)
		{
			printf("Error writing pad into file\n");
			close(fd);
			exit(-1);
		}
		i -= i > 1024 ? 1024 : i;
	}
	close(fd);
	printf("Successfully padded file to %d bytes\n", newlength);
	exit(0);
}
	

