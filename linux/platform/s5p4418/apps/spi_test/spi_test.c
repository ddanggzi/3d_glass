#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <string.h>
#include "spi_test.h"

static void pabort(const char *s)
{
	perror(s);
	abort();
}

static const char *device_spi0 = "/dev/spidev0.0";
static const char *device_spi2 = "/dev/spidev2.0";

//static const char *device = "/dev/spidev0.1";
//static uint8_t mode;
//static uint8_t bits = 8;
//static uint32_t speed = 500000;
//static uint16_t delay;



int spi_init(int fd)
{
	int ret;
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	return 0;
}

//int spi_read(int fd,int offset, char * addr, int size)
int spi_read(int fd,int offset, uint8_t * addr, int size)

{
	int ret,i;

#if 0   // for EEPROM
	uint8_t tx[size+CMD_BUF];	
	uint8_t rx[size+CMD_BUF];

	for(i=0;i<size+CMD_BUF;i++)
#else   // for TCON
	uint8_t tx[size+ADDR_BYTE];	
	uint8_t rx[size+ADDR_BYTE];

	for(i=0;i<size+ADDR_BYTE;i++)
#endif
	{
		rx[i]=0;
	}
	tx[0] =  READ_CMD;
	if(ADDR_BYTE == 3)
	{	
		tx[1] = (offset >> 16);
		tx[2] = (offset >> 8);
		tx[3] = (offset );
	}
	else if(ADDR_BYTE == 2)
	{
		tx[1] = (offset >> 8);
		tx[2] = (offset);
	}
	else if(ADDR_BYTE == 1)
	{

#if 0   // for EEPROM
		tx[1] = (offset);
#else   // for TCON
        tx[0] = (offset);
#endif 
	}
	
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = ARRAY_SIZE(tx),
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret == 1)
		pabort("can't send spi message");
#ifdef SHOW_READ_DATA
	for (ret = 0; ret < ARRAY_SIZE(tx); ret++) {
		if (!(ret % 6))
			puts("");
		printf("%.2X ", rx[ret]);		
	}
	puts("");	
#endif

#if 0   // for EEPROM
	memcpy(addr, rx+CMD_BUF,size);
#else   // for TCON
	memcpy(addr, rx+ADDR_BYTE,size);
#endif
	return 0;
}
int spi_blk_erase(int fd,int addr)
{
	int ret,i;
	uint8_t tx[4];	
	uint8_t rx[4];

	tx[0] =  0xd8;
	
	tx[1] = ((addr >> 16) & 0xff);
	tx[2] = ((addr >> 8) & 0xff);
	tx[3] = (addr & 0xff);

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = ARRAY_SIZE(tx),
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret == 1)
		pabort("can't send spi message");
#ifdef SHOW_READ_DATA
	for (ret = 0; ret < ARRAY_SIZE(tx); ret++) {
		if (!(ret % 6))
			puts("");
		printf("%.2X ", rx[ret]);		
	}
	puts("");	
#endif
	return 0;
}

int spi_write_enable(int fd)
{
	int ret,i;
	uint8_t tx[1];	
	uint8_t rx[1];

	tx[0] =  WRITE_ENABLE;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = ARRAY_SIZE(tx),
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	
	return ret;

}

//int spi_write(int fd, int dst, char * addr, int size)
int spi_write(int fd, int dst, uint8_t * addr, int size)
{
	int ret,i,offset=0;
	uint8_t tx[PAGE_SIZE+CMD_BUF];	
	uint8_t rx[PAGE_SIZE+CMD_BUF];

	int write_size =0;

	tx[0] =  WRITE_CMD;

	do{
		spi_write_enable(fd);
		if(size > PAGE_SIZE)
		{
		write_size = PAGE_SIZE - (dst%PAGE_SIZE);
		}
		else
		{	
			if((PAGE_SIZE - (dst%PAGE_SIZE))  > size)
			write_size = size;
			else 
			write_size = (PAGE_SIZE - (dst%PAGE_SIZE));	
		}	

		if(ADDR_BYTE == 3)
		{	
			tx[1] = (dst >> 16);
			tx[2] = (dst >> 8);
			tx[3] = (dst );
		}
		else if(ADDR_BYTE == 2)
		{
			tx[1] = (dst >> 8);
			tx[2] = (dst );
		}
		// Added by ddanggzi
		else if(ADDR_BYTE == 1)
		{
			tx[0] = (dst);
		}
		
		//memcpy(tx+CMD_BUF,&addr[offset],write_size);
		memcpy(tx+ADDR_BYTE,&addr[offset],write_size);
		
		struct spi_ioc_transfer tr = {
			.tx_buf = (unsigned long)tx,
			.rx_buf = (unsigned long)rx,
			//.len = ARRAY_SIZE(tx),
			//.len = write_size+CMD_BUF,
			.len = write_size+ADDR_BYTE,
			.delay_usecs = delay,
			.speed_hz = speed,
			.bits_per_word = bits,
		};

		ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
		if (ret == 1)
			pabort("can't send spi message");

		size -= write_size;

		offset+=write_size;

		dst+=write_size;
		usleep(10000);
	}while(size > 0);

	return ret;

}

void init_oled(int fd){
	uint8_t write_buf[0x8] ;

	printf("======= SPI EEPROM TEST!!!!============\n");

	
	write_buf[0] = 0x01;
    spi_write(fd,0x00,write_buf,1);	//Write Data
    write_buf[0] = 0x70;
    spi_write(fd,0x01,write_buf,1);	//Write Data
	write_buf[0] = 0x0F;
	spi_write(fd,0x02,write_buf,1); //Write Data
	write_buf[0] = 0x00;
    spi_write(fd,0x03,write_buf,1);	//Write Data
    write_buf[0] = 0x4b;
    spi_write(fd,0x04,write_buf,1);	//Write Data
    write_buf[0] = 0xD0;
    spi_write(fd,0x05,write_buf,1);	//Write Data
    write_buf[0] = 0x40;
    spi_write(fd,0x06,write_buf,1);	//Write Data
	write_buf[0] = 0x40;
	spi_write(fd,0x07,write_buf,1); //Write Data
	write_buf[0] = 0x40;
    spi_write(fd,0x08,write_buf,1);	//Write Data
    write_buf[0] = 0x80;
    spi_write(fd,0x09,write_buf,1);	//Write Data

	write_buf[0] = 0x40;
    spi_write(fd,0x0a,write_buf,1);	//Write Data
    write_buf[0] = 0x40;
    spi_write(fd,0x0b,write_buf,1);	//Write Data
	write_buf[0] = 0x40;
	spi_write(fd,0x0c,write_buf,1); //Write Data
	write_buf[0] = 0x29;
    spi_write(fd,0x0d,write_buf,1);	//Write Data
    write_buf[0] = 0x34;
    spi_write(fd,0x0e,write_buf,1);	//Write Data
    write_buf[0] = 0x29;
    spi_write(fd,0x0f,write_buf,1);	//Write Data
    write_buf[0] = 0x26;
    spi_write(fd,0x10,write_buf,1);	//Write Data
	write_buf[0] = 0x26;
	spi_write(fd,0x11,write_buf,1); //Write Data
	write_buf[0] = 0x00;
    spi_write(fd,0x12,write_buf,1);	//Write Data
    write_buf[0] = 0x04;
    spi_write(fd,0x13,write_buf,1);	//Write Data

	write_buf[0] = 0x44;
    spi_write(fd,0x14,write_buf,1);	//Write Data
    write_buf[0] = 0x40;
    spi_write(fd,0x15,write_buf,1);	//Write Data
	write_buf[0] = 0x26;
	spi_write(fd,0x16,write_buf,1); //Write Data
	write_buf[0] = 0x26;
    spi_write(fd,0x17,write_buf,1);	//Write Data
    write_buf[0] = 0x33;
    spi_write(fd,0x18,write_buf,1);	//Write Data
    write_buf[0] = 0x39;
    spi_write(fd,0x19,write_buf,1);	//Write Data
    write_buf[0] = 0x1c;
    spi_write(fd,0x1a,write_buf,1);	//Write Data
	write_buf[0] = 0x03;
	spi_write(fd,0x1b,write_buf,1); //Write Data
	write_buf[0] = 0x20;
    spi_write(fd,0x1c,write_buf,1);	//Write Data
    write_buf[0] = 0x00;
    spi_write(fd,0x1d,write_buf,1);	//Write Data

	write_buf[0] = 0x33;
    spi_write(fd,0x1e,write_buf,1);	//Write Data
    write_buf[0] = 0x25;
    spi_write(fd,0x1f,write_buf,1);	//Write Data
	write_buf[0] = 0x0c;
	spi_write(fd,0x20,write_buf,1); //Write Data
	write_buf[0] = 0x37;
    spi_write(fd,0x21,write_buf,1);	//Write Data
    write_buf[0] = 0x1a;
    spi_write(fd,0x22,write_buf,1);	//Write Data
    write_buf[0] = 0x3c;
    spi_write(fd,0x23,write_buf,1);	//Write Data
    write_buf[0] = 0x00;
    spi_write(fd,0x24,write_buf,1);	//Write Data
	write_buf[0] = 0x01;
	spi_write(fd,0x25,write_buf,1); //Write Data
	write_buf[0] = 0x92;
    spi_write(fd,0x26,write_buf,1);	//Write Data
    write_buf[0] = 0x20;
    spi_write(fd,0x27,write_buf,1);	//Write Data

	write_buf[0] = 0x66;
    spi_write(fd,0x28,write_buf,1);	//Write Data
    write_buf[0] = 0x00;
    spi_write(fd,0x29,write_buf,1);	//Write Data
	write_buf[0] = 0x68;
	spi_write(fd,0x2a,write_buf,1); //Write Data
	write_buf[0] = 0x20;
    spi_write(fd,0x2b,write_buf,1);	//Write Data
    write_buf[0] = 0x3c;
    spi_write(fd,0x2c,write_buf,1);	//Write Data
    write_buf[0] = 0x01;
    spi_write(fd,0x80,write_buf,1);	//Write Data
}

int main()
{
	int ret,i;
	int size = 64;
	uint8_t temp;

	uint8_t read_buf_org[0x80] = {0, };
	uint8_t read_buf[0x80] = {0, };
	uint8_t write_buf[0x80] ;

	int fd;
	int fd1;

	
	bits = 8;
	speed = 750000;

	fd = open(device_spi2, O_RDWR);
	
	if (fd < 0)
		pabort("can't open device");
	
	if( 0 !=spi_init(fd))
		pabort("SPI Initilize fail");

	fd1 = open(device_spi0, O_RDWR);
	
	if (fd1 < 0)
		pabort("can't open device");
	
	if( 0 !=spi_init(fd1))
		pabort("SPI Initilize fail");


	printf("======= SPI EEPROM TEST!!!!============\n");
/*
	spi_read(fd,0x00,read_buf_org,5);	//Read Data
	for (i =0 ; i <5 ; i++)
	{
		printf("0x%.2X ",read_buf_org[i]);
	}
	printf("\n");

	spi_read(fd,0x20,read_buf_org,5);	//Read Data
	for (i =0 ; i <5 ; i++)
	{
		printf("0x%.2X ",read_buf_org[i]);
	}
	printf("\n");

	spi_read(fd,0x30,read_buf_org,5);	//Read Data
	for (i =0 ; i <5 ; i++)
	{
		printf("0x%.2X ",read_buf_org[i]);
	}
	printf("\n");

	spi_read(fd,0x40,read_buf_org,5);	//Read Data
	for (i =0 ; i <5 ; i++)
	{
		printf("0x%.2X ",read_buf_org[i]);
	}
	printf("\n");
	
		
	
	// READ : org data 
	printf("== READ DATA(ORG)");
	spi_read(fd,0x000,read_buf_org,size);	//Read Data
	for (i =0 ; i <size ; i++)
	{
		if ((i%16) == 0)	
			printf("\n");
		printf("0x%.2X ",read_buf_org[i]);
	}
	printf("\n");	

*/	


    write_buf[0] = 0xE0;
	spi_write(fd,0x01,write_buf,1);	//Write Data

	write_buf[0] = 0x04;
	spi_write(fd,0x02,write_buf,1);	//Write Data
	write_buf[0] = 0x01;
	spi_write(fd,0x80,write_buf,1);	//Write Data


	while(1){
		write_buf[0] = 0x00;
		spi_write(fd,0x81,write_buf,1);	//Write Data


		spi_read(fd,0x81,read_buf,1);	//Read Data
		temp = read_buf[0];

		printf("== READ DATA(new) = %x", temp);

	    if( ( temp & 0x0C ) != 0x00 ){
			
			write_buf[0] = 0x01;
			spi_write(fd,0x00,write_buf,1); //Write Data
	    }
		else{
			usleep(100);
			break;
		}

    }


#if 1	
	// WRITE : new data
	printf("\r\nOLED Module Ready !!!\r\n");
	sleep(1);
 /*   
    write_buf[0] = 0x4F;
    spi_write(fd,0x04,write_buf,1);	//Write Data

	
    write_buf[0] = 0xE0;
    spi_write(fd,0x05,write_buf,1);	//Write Data
    write_buf[0] = 0x41;
    spi_write(fd,0x06,write_buf,1);	//Write Data
    write_buf[0] = 0x40;
    spi_write(fd,0x07,write_buf,1);	//Write Data
    write_buf[0] = 0x39;
    spi_write(fd,0x08,write_buf,1);	//Write Data
*/
    
 /* 
	//while(1){
	//spi_write_enable(fd);
    printf("== READ DATA(new)");

	
	write_buf[0] = 0xE0;
	spi_write(fd,0x01,write_buf,1);	//Write Data

	write_buf[0] = 0x04;
	spi_write(fd,0x02,write_buf,1);	//Write Data
	write_buf[0] = 0x01;
	spi_write(fd,0x80,write_buf,1);	//Write Data
	write_buf[0] = 0x01;
	spi_write(fd,0x81,write_buf,1);	//Write Data

	spi_read(fd,0x81,read_buf,1);	//Read Data
	temp = read_buf[0];

	printf("\r\n ---------- 0x%.2X ",read_buf[0]);

	//spi_write_enable(fd);
	
	temp &= 0xF0;
	write_buf[0] = temp;
	printf("\r\n kks = %x\r\n", temp);
	
	spi_write(fd,0x01,write_buf,1);	//Write Data
*/
//	usleep(100000);


 
	init_oled(fd);
    init_oled(fd1);
/*
    write_buf[0] = 0xB0;
    spi_write(fd,0x01,write_buf,1);	//Write Data
    
    write_buf[0] = 0x0F;
	write_buf[0] |= 0x40;
    spi_write(fd,0x04,write_buf,1);	//Write Data
    write_buf[0] = 0xE0;
    spi_write(fd,0x05,write_buf,1);	//Write Data
    write_buf[0] = 0x41;
    spi_write(fd,0x06,write_buf,1);	//Write Data
    write_buf[0] = 0x40;
    spi_write(fd,0x07,write_buf,1);	//Write Data
    write_buf[0] = 0x39;
    spi_write(fd,0x08,write_buf,1);	//Write Data

*/
	write_buf[0] = 0x04;
	spi_write(fd,0x02,write_buf,1);	//Write Data
	write_buf[0] = 0x01;
	spi_write(fd,0x80,write_buf,1);	//Write Data

#endif
    printf("\r\n================<Left OLED DATA>====================\r\n");	
	for(i = 0; i < 0x81; i++){
		write_buf[0] = i;
		spi_write(fd,0x81,write_buf,1);	//Write Data

		spi_read(fd,0x81,read_buf,1);	//Read Data
		temp = read_buf[0];

		if ((i%16) == 0)	
			printf("\n");
		printf("0x%.2X ",temp);
	}
	
	printf("\r\n=========================================\r\n");	

	
	close(fd);

	return 0;
}
