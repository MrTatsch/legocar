#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include "fpga_spi.h"

int fd;

static uint8_t spi_mode,spi_bits;
static uint32_t spi_speed;
static uint16_t spi_delay;

int fpga_open()
{
  int ret;
  printf ( "Will use SPI to send commands to fpga\n" );
  printf("spi configuration: \n" );
  printf("    + dev: %s\n", FPGA_SPI_DEV );
  printf("    + mode: %d\n", spi_mode);
  printf("    + bits per word: %d\n", spi_bits);
  printf("    + speed: %d Hz (%d KHz)\n", spi_speed, spi_speed/1000);
  printf("\n");

  spi_mode = SPI_MODE;
  spi_bits = SPI_BITS_PER_WORD;
  spi_speed = SPI_SPEED_HZ;
  spi_delay = SPI_DELAY;

  if ((fd = open(FPGA_SPI_DEV, O_RDWR)) < 0) {
    printf("E: fpga: spi: Failed to open dev\n");
    return -1;
  }
  
  if ((ret = ioctl(fd, SPI_IOC_WR_MODE, &spi_mode)) < 0 )
  {
    printf("E: fpga: spi: can't set spi mode wr\n");
    return -1;
  }
    
  if ((ret = ioctl(fd, SPI_IOC_RD_MODE, &spi_mode)) < 0 )
  {
    printf("E: fpga: spi: can't set spi mode rd\n");
    return -1;
  }
    
  if ((ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bits)) < 0 )
  {
    printf("E: fpga: spi: can't set bits per word wr\n");
    return -1;
  }
    
  if ((ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &spi_bits)) < 0 )
  {
    printf("E: fpga: spi: can't set bits per word rd\n");
    return -1;
  }
    
  if ((ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed)) < 0 )
  {
    printf("E: fpga: spi: can't set speed wr\n");
    return -1;
  }
    
  if ((ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &spi_speed)) < 0 )
  {
    printf("E: fpga: spi: can't set speed rd\n");
    return -1;
  }

  return 0;
}

void fpga_close()
{
  close(fd);
}

int fpga_setservo ( uint8_t servoNr, uint16_t servoPos )
{
  unsigned char wbuf[3];
  unsigned char rbuf[3];


  wbuf[0] = (CMD_SERVO<<4) | (servoNr & 0xf);
  wbuf[1] = HIGHBYTE(servoPos);
  wbuf[2] = LOWBYTE(servoPos);
  
  if ((spisend(rbuf,wbuf, 3)) != 3) {
    printf("E: fpga: Could not write specified amount of bytes to printf chardev\n");
    return -1;
  }

  return 0;
}

int fpga_setleds ( uint8_t onoff, uint8_t leds )
{
  unsigned char rbuf[1];
  unsigned char wbuf[1];
    
  wbuf[0] = (CMD_LED<<4) | (leds & ((1<<2)|(1<<1)|(1<<0)));
  if ( onoff )
    wbuf[0] |= (1<<3);
  
  if ((spisend(rbuf,wbuf, 1)) != 1) {
    printf("E: fpga: Could not write specified amount of bytes to printf chardev\n");
    return -1;
  }

  return 0;
}

int fpga_setspeedv (uint16_t vspeed, uint16_t vsteering )
{
    unsigned char rbuf[5];
    unsigned char wbuf[5];

    
    wbuf[0] = CMD_SPEED;
    wbuf[1] = HIGHBYTE(vspeed);
    wbuf[2] = LOWBYTE(vspeed);
    wbuf[3] = HIGHBYTE(vsteering);
    wbuf[4] = LOWBYTE(vsteering);
    
    if ((spisend(rbuf,wbuf, 5)) != 5) {
        printf("E: fpga: Could not write specified amount of bytes to spi\n");
        return -1;
    }
    
    return 0;
}

int spisend ( char*rbuf, char*wbuf, int len )
{

  int ret;

  struct spi_ioc_transfer tr = {
    .tx_buf = (unsigned long)wbuf,
    .rx_buf = (unsigned long)rbuf,
    .len = len,
    .delay_usecs = SPI_DELAY,
    .speed_hz = SPI_SPEED_HZ,
    .bits_per_word = SPI_BITS_PER_WORD,
  };
    
  ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
  if (ret < 1){
    printf("can't send spi message");
        return -1;
    }
    
  for (ret = 0; ret < len; ret++) {
    if (!(ret % 6))
      puts("");
    printf("%.2X ", wbuf[ret]);
  }
  puts("");
}

void fpga_testservos()
{
    if ( fpga_open() )
    {
        printf ( "E: FPGA: Could not open SPI to FPGA\n" );
        exit(-1);
    }
    printf("Moving servo left\n");
    fpga_setservo(1,0);
    sleep(2);
    printf("Moving servo centre\n");
    fpga_setservo(1,4000);
    sleep(2);
    printf("Moving servo right\n");
    fpga_setservo(1,8000);
    sleep(2);
    printf("Moving servo centre\n");
    fpga_setservo(1,4000);
    fpga_close();
}

/*
int fpgamain(int argc, char *argv[])
{
    testservos();
  return 0;
}
*/
