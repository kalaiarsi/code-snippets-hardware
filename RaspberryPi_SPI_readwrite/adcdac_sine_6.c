// takes about 9 seconds to write sine value and adc value to file..
//adc and dac - filewrite.. 
// resolution : 1.67 mV , adc : 2048 values (11bit ???) //range of input for DAC : 0 to 4.096V..
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
//#define VREF 2.048	// MCP4822 voltage reference [V]
#define VREF 4.096 
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
static uint8_t mode;// adc
static uint8_t dac_mode = SPI_MODE_0;// dac: SPI_MODE_0 
#define PI 3.14


static void exit_on_error (const char *s)// Exit and print error code
{ 	perror(s);
  	abort();
} 

static int adc_transfer(int fd)
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
//	printf("%.2x-%.2x    %.2x  %.4d",rx[0],rx[1],rcvd, rcvd);
//	printf("value read from adc is : %4d\n",rcvd);
//	printf("%4d\n",rcvd);
	
//	puts("");
	return rcvd;
}



static int dac_transfer(int dac_fd, double voltA,double voltB)
{
	int dataA, dataB;
	uint16_t tx;    	// RX buffer (16 bit unsigned integer)
 
      	struct spi_ioc_transfer tr = 
      	{	.tx_buf = (unsigned long)&tx, 
          	.rx_buf = (unsigned long)NULL,
          	.len = 2,
          	.delay_usecs = 0,
          	.speed_hz = SPI_SPEED,
          	.bits_per_word = 8,
          	.cs_change = 0,//0 - initially
       	};
/*
	if ((voltA > VREF) || (voltA < 0))
        { 
          printf ( "VA out of range (0 -> %f)\n\n", VREF);
          exit(-2);
         }
*/

/*        if ((voltB > VREF) || (voltB < 0))
        {
          printf ( "VB out of range (0 -> %f)\n\n", VREF);
          exit(-3);
        }
*/
//	if(dac_fd<0){exit_on_error("dac_fd invalid.");}

//open SPI device for writing...........
// Open SPI device


//        if ((fd = open(dac_device, O_RDWR)) < 0) exit_on_error ("Can't open SPI device");
        // Set SPI mode
//        if (ioctl(fd, SPI_IOC_WR_MODE, &dac_mode) == -1) exit_on_error ("Can't set SPI mode");
	

//.....................................................      
	dataB = voltB / VREF * 4096; 	// Create data to send (see textand data sheet)
        dataB = dataB | 0x9000;//0xF000	
        
	tx = (dataB << 8) | (dataB >> 8);	// Adjust bits
        
//printf("writing on ch-B\n");
       	// Write data
       	if (ioctl(dac_fd, SPI_IOC_MESSAGE(1), &tr) < 1) exit_on_error ("Can't send SPI message");


/* 	writing on channel A is unnecessary and so, not used.           
	for project, using ch-B of ADC and VDC-b of DAC 
       dataA = voltA / VREF * 4096;
        dataA = (dataA & 0x0FFF) | 0x3000;
//	printf("writing on ch-A\n");
        tx = (dataA << 8) | (dataA >> 8);
      //Write data
        if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 1) exit_on_error ("Can't send SPI message");
*/
//	close(dac_fd);
	return dac_fd;
}

static int dac_transfer_channelB(int dac_fd,double sine)
{
//        int dataA;
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
 	printf("sine in function : %f",sine);
 	if (ioctl(dac_fd, SPI_IOC_MESSAGE(1), &tr) < 1) exit_on_error ("Can't send SPI message");
	return dac_fd;
}

static void adc_print_usage(const char *prog)
{
/*
	printf("Usage: %s [-DsbdlHOLC3]\n", prog);
	puts("  -D --device   device to use (default /dev/spidev1.1)\n"
	     "  -s --speed    max speed (Hz)\n"
	     "  -d --delay    delay (usec)\n"
	     "  -b --bpw      bits per word \n"
	     "  -l --loop     loopback\n"
	     "  -H --cpha     clock phase\n"
	     "  -O --cpol     clock polarity\n"
	     "  -L --lsb      least significant bit first\n"
	     "  -C --cs-high  chip select active high\n"
	     "  -3 --3wire    SI/SO signals shared\n");
	exit(1);
*/
}

static void adc_parse_opts(int argc, char *argv[])
{
/*
	while (1) {
		static const struct option lopts[] = {
			{ "device",  1, 0, 'D' },
			{ "speed",   1, 0, 's' },
			{ "delay",   1, 0, 'd' },
			{ "bpw",     1, 0, 'b' },
			{ "loop",    0, 0, 'l' },
			{ "cpha",    0, 0, 'H' },
			{ "cpol",    0, 0, 'O' },
			{ "lsb",     0, 0, 'L' },
			{ "cs-high", 0, 0, 'C' },
			{ "3wire",   0, 0, '3' },
			{ "no-cs",   0, 0, 'N' },
			{ "ready",   0, 0, 'R' },
			{ NULL, 0, 0, 0 },
		};
		int c;

		c = getopt_long(argc, argv, "D:s:d:b:lHOLC3NR", lopts, NULL);

		if (c == -1)
			break;

		switch (c) {
		case 'D':
			adc_device = optarg;
			break;
		case 's':
			speed = atoi(optarg);
			break;
		case 'd':
			delay = atoi(optarg);
			break;
		case 'b':
			bits = atoi(optarg);
			break;
		case 'l':
			mode |= SPI_LOOP;
			break;
		case 'H':
			mode |= SPI_CPHA;
			break;
		case 'O':
			mode |= SPI_CPOL;
			break;
		case 'L':
			mode |= SPI_LSB_FIRST;
			break;
		case 'C':
			mode |= SPI_CS_HIGH;
			break;
		case '3':
			mode |= SPI_3WIRE;
			break;
		case 'N':
			mode |= SPI_NO_CS;
			break;
		case 'R':
			mode |= SPI_READY;
			break;
		default:
			adc_print_usage(argv[0]);
			break;
		}
	}
*/
}

static int setup_adc()
{
int ret;
int adc_fd;
adc_fd = open(adc_device, O_RDWR);
	if (adc_fd < 0)
exit_on_error("can't open device");

	/*
	 * spi mode
	 */
	ret = ioctl(adc_fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		exit_on_error("can't set spi mode");

	ret = ioctl(adc_fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		exit_on_error("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(adc_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		exit_on_error("can't set bits per word");

	ret = ioctl(adc_fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		exit_on_error("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(adc_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		exit_on_error("can't set max speed hz");

	ret = ioctl(adc_fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		exit_on_error("can't get max speed hz");
	return adc_fd;
}


//static void dac_open_SPI(

static int setup_dac()
{
	int dac_fd;
	dac_fd=open(dac_device,O_RDWR);
 	// Open SPI device
       	if (dac_fd<0) exit_on_error ("Can't open SPI device");
	
	// Set SPI mode
	int ret=ioctl(dac_fd,SPI_IOC_WR_MODE,&dac_mode);
	if (ret == -1) exit_on_error ("Can't set SPI mode");
	return dac_fd;
}

int main(int argc, char *argv[])
{
	int adc_fd;
	int dac_fd;
	int adc_read_value;
	double sine;	
	FILE *fp;
//	FILE *fopen(const char *filename,const char *mode);
	fp=fopen("/home/pi/test.txt","w");
	
	double voltA=1.56345, voltB=1.011;
	//parse_opts(argc,argv); // was earlier used for adc-spi setup..
//	printf("setting up adc and dac\n");
	adc_fd=setup_adc();
	dac_fd=setup_dac();
//	sine=sin(2*PI*100*1);
//	printf("sine is : %f\n",sine);

int count=0;
while(count<20000)
{
	sine=sin(2*PI*100*(0.001*count))+2.0;
//	printf("sine is : %f       ",sine);
//	printf("..\n");


	printf("voltA : %f , voltB : %f\n",voltA,voltB);


//	printf("input current : %f,   measured voltage : %f",sine,adc_read_value);
	voltB=sine;
	dac_fd=dac_transfer(dac_fd,voltA,voltB);
//	dac_fd=dac_transfer_channelB(dac_fd,sine);
	//printf("\n");
	//voltA = atof(argv[1]);
        //voltB = atof(argv[2]);
//	printf("reading from adc\n");
	adc_read_value=adc_transfer(adc_fd);
//	printf("sending value to dac\n");
//	dac_transfer(dac_fd,voltA,voltB);
	count++;

//	printf("input current : %f,   measured voltage : %f\n",sine,adc_read_value);

//	printf("dac : %4f, adc : %4f\n",voltB,adc_read_value);
	fprintf(fp,"s : %4f , adc : %4f\n",sine,adc_read_value);
//	voltA=voltA-0.1;
//	voltB=voltB+0.3;
//	voltB=sine;
}
//	printf("looping over, closing SPI connections\n");

	close(adc_fd);
	close(dac_fd);
	fclose(fp);
	return(0);
}
