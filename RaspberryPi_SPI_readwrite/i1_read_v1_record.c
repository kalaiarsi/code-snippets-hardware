//reads from i1.txt, sedn i1 and reads it as v1, writes v1 to v1.txt
//alter ADC_FACTOR for calibration

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <math.h>

static const char *adc_device = "/dev/spidev0.0";//MCP3202
static const char *dac_device = "/dev/spidev0.1";// MCP4822 CD to RPi CS1
static uint8_t bits = 8;
static uint32_t speed = 4000000;//500000 : adc - use SPI_SPEED = speed as same (upto 4mhz
static uint16_t delay;//in microseconds
#define SPI_SPEED 2100000  // dac - SPI frequency clock
#define VREF 4.096 
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
static uint8_t mode;// adc
static uint8_t dac_mode = SPI_MODE_0;// dac: SPI_MODE_0 
#define PI 3.14
#define SAMPLE_COUNT 20000
static double Frequency[SAMPLE_COUNT];
FILE *file;
static double current[SAMPLE_COUNT];
#define ADC_FACTOR 1.6

static void exit_on_error (const char *s)// Exit and print error code
{ 	perror(s);
  	abort();
} 

//static int adc_transfer(int fd)
static float adc_transfer(int fd)
{
	int rcvd;
	int ret;
	uint8_t tx[] = {
			0xFF,0//gives 2048 on pin 3 :)
	};
	uint8_t rx[ARRAY_SIZE(tx)] = { };//{1,} or even {0,1}
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = ARRAY_SIZE(tx),
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		exit_on_error("can't send spi message");
	rcvd=((rx[0]&0x0F)<<8)+rx[1];
	printf("rcvd : %d",rcvd);
//	return rcvd;//returns 0 to 2048 or 4096....
	return (4.096*rcvd*ADC_FACTOR/4096);//2048 (or) 4096 ????????
}

static int dac_transfer_channelB(int dac_fd,double sine)
{
	int  dataB;
        uint16_t tx;            // RX buffer (16 bit unsigned integer) 
        struct spi_ioc_transfer tr = 
        {       .tx_buf = (unsigned long)&tx, 
                .rx_buf = (unsigned long)NULL,
                .len = 2,
                .delay_usecs = 0,
                .speed_hz = SPI_SPEED,
	        .bits_per_word = 8,
                .cs_change = 0,//0 - initially
        };

	dataB = sine / VREF * 4096;    // Create data to send (see textand data sheet)
        dataB = dataB | 0x9000;//0xF000 
        tx = (dataB << 8) | (dataB >> 8);       // Adjust bits
// 	printf("sine in function : %f",sine);
 	if (ioctl(dac_fd, SPI_IOC_MESSAGE(1), &tr) < 1) exit_on_error ("Can't send SPI message");
	return dac_fd;
}

static int setup_adc()
{
	int ret;
	int adc_fd;
	adc_fd = open(adc_device, O_RDWR);
	if (adc_fd < 0)
		exit_on_error("can't open device");

	ret = ioctl(adc_fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		exit_on_error("can't set spi mode");

	ret = ioctl(adc_fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		exit_on_error("can't get spi mode");
	ret = ioctl(adc_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		exit_on_error("can't set bits per word");

	ret = ioctl(adc_fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		exit_on_error("can't get bits per word");
	ret = ioctl(adc_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		exit_on_error("can't set max speed hz");

	ret = ioctl(adc_fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		exit_on_error("can't get max speed hz");
	return adc_fd;
}

static int setup_dac()
{
	int dac_fd;
	dac_fd=open(dac_device,O_RDWR);
       	if (dac_fd<0) exit_on_error ("Can't open SPI device");
	int ret=ioctl(dac_fd,SPI_IOC_WR_MODE,&dac_mode);
	if (ret == -1) exit_on_error ("Can't set SPI mode");
	return dac_fd;
}

static void read_current(char filename[])
{
	int i=0;double data_value;char line[40];
	file=fopen(filename,"r");
	printf("filename : %s\n",filename);
	if(!file){printf("\nerror opening file in read from file\n");exit(-1);}
	while(fscanf(file,"%s",line)==1)
	{
	data_value=atof(line);
	current[i]=data_value;
	i=i+1;
	}
	printf("data file read");
	fclose(file);
	}

int main(int argc, char *argv[])
{
	printf("in main");
	int adc_fd;int dac_fd;
	double frequency=0.01;
	double voltage_value;
	double current_value;	
	FILE *fp_IV;FILE *fp_v1;
	fp_IV=fopen("/home/pi/IV.txt","w");
	fp_v1=fopen("/home/pi/v1.txt","w");
	read_current("/home/pi/i1.txt");
	printf("current[10000]=%f",current[10000]);
	double voltA=1.12345, voltB=2.12345;
	adc_fd=setup_adc();
	dac_fd=setup_dac();
	int count=0;
	float time=0.0;
	while(count<SAMPLE_COUNT)
	{
//	sine=sin(2*PI*frequency*(time))+2.0;// -1 to +1 is changed to +1 to +3 (to avoid saturation)
	current_value=current[count]+2.0;
	printf("voltA : %f , voltB : %f\n",voltA,voltB);
	voltB=current_value;
	dac_fd=dac_transfer_channelB(dac_fd,current_value);
	voltage_value=adc_transfer(adc_fd);
	printf("I : %f , V : %f\n",current_value,voltage_value);

//	time=time+0.0001;//+0.001;// sampling time : 1ms
//	frequency=frequency+1;
	count++;
	fprintf(fp_IV,"%6.12f %6.12f\n",current_value,voltage_value);
	fprintf(fp_v1,"%6.12f\n",voltage_value);
	}
	close(adc_fd);
	close(dac_fd);
	fclose(fp_IV);fclose(fp_v1);
	return(0);
}
