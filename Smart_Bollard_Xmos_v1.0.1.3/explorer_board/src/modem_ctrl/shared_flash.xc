// Copyright 2022-2023 EMQOS LIMITED.
// This Software is subject to the terms of the EMQOS
// Author Vamsi

#include <xs1.h>
#include <platform.h>
#include <timer.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


/* Flash Opcode Cycle*/
#define Read_Memory						0x03
#define High_Speed_Read_Memory			0x0B
#define Flash_4KByte_Sector_Erase		0x20
#define Flash_32KByte_Block_Erase		0x52
#define Flash_64KByte_Block_Erase		0xD8
#define Chip_Erase						0x60
#define Byte_Program					0x02
#define AAI_Word_program				0xAD
#define Read_Status_Register_1			0x05
#define Read_Status_Register_2			0x35
#define Read_Status_Register_3			0x15
#define Enable_Write_Status_Register	0x50
#define Write_Status_Register			0x01
#define Write_Status_Register_2			0x31
#define Write_Status_Register_3			0x11
#define Write_Enable					0x06
#define Write_Disable					0x04
#define Read_Device_ID					0x90
#define JEDEC_ID						0x9F
#define EBSY							0x70
#define DBSY							0x80

#define FileNameLength                  45


//#define PRODUCTION
#define ONE
//#define TESTING

extern "C" {
	
	
	//////////Debug///////////////////
	extern void Debug_TextOut( int8_t minVerbosity, const char * pszText );
	extern void Debug_Output1( int8_t minVerbosity, const char * pszFormat, uint32_t someValue );
	
}
//extern void return_string(int x, char *ptr);
//int shared_flash_write(char *buf, int size);
extern uint8_t capture_sequence_number;
extern uint8_t capture_number;
extern uint8_t* FTPImagFileBufferptr_sf;

//int sf_write(uint8_t* FTPImagFileBufferptr, uint8_t* SF_Filename, uint32_t uImageSize, uint8_t capture_sequence_number_sf,uint8_t capture_number_sf);

int sf_write_test();
//int shared_flash_write();
int clear_flash_start = 1;



char tempPrintString[300];
unsigned char  sf_filename1[100] = {"                                                  "};
uint8_t pr[4] = {'Y','N','\\','0'};

#ifdef TESTING

//Data we are going to write/read from flash memory
//unsigned int data_pool[] = {0x00010203, 0x04050607, 0x08090A0B, 0x0C0D0E0F,
                            //0x10111213, 0x14151617, 0x18191A1B, 0x1C1D1E1F,
                            //0x20212223, 0x24252627, 0x28292A2B, 0x2C2D2E2F,
                            //0x30313233, 0x34353637, 0x38393A3B, 0x3C3D3E3F,
                            //0x40414243, 0x44454647, 0x48494A4B, 0x4C4D4E4F,
                            //0x50515253, 0x54555657, 0x58595A5B, 0x5C5D5E5F,
                            //0x60616263, 0x64656667, 0x68696A6B, 0x6C6D6E6F,
                            //0x70717273, 0x74757677, 0x78797A7B, 0x7C7D7E7F,
                            //0x80818283, 0x84858687, 0x88898A8B, 0x8C8D8E8F,
                            //0x90919293, 0x94959697, 0x98999A9B, 0x9C9D9E9F,
                            //0xA0A1A2A3, 0xA4A5A6A7, 0xA8A9AAAB, 0xACADAEAF,
                            //0xB0B1B2B3, 0xB4B5B6B7, 0xB8B9BABB, 0xBCBDBEBF,
                            //0xC0C1C2C3, 0xC4C5C6C7, 0xC8C9CACB, 0xCCCDCECF,
                            //0xD0D1D2D3, 0xD4D5D6D7, 0xD8D9DADB, 0xDCDDDEDF,
                            //0xE0E1E2E3, 0xE4E5E6E7, 0xE8E9EAEB, 0xECEDEEEF,
                            //0xF0F1F2F3, 0xF4F5F6F7, 0xF8F9FAFB, 0xFCFDFEFF};
                            
#ifdef ONE
unsigned char data_pool[] = {"Hello World. Running from flash and Writing to SF from Production code!!! EMQOS. A great place to work at. Peacefule place Girinagar!!!Hello World. Running from flash and Writing to SF from Production code!!! EMQOS. A great place to work at. Testing!!!!!!!!"};
#endif
#ifndef ONE
unsigned char data_pool[] = {"Hello World. EMQOS. A great place to work at. Peacefule place Girinagar!!!"};
#endif
//unsigned char  sf_filename1[50] = {"IMG_FL_1111_32092_20220803_111940_01.jpeg"};


unsigned char test_data[] = {"Hello World. Running from flash and Writing to SF from Production code!!! EMQOS. A great place to work at. Peacefule place Girinagar!!!\
	Hello World. Running from flash and Writing to SF from Production code!!! EMQOS. A great place to work at. Testing!!!!!!!!\
	 Hello World. Running from flash and Writing to SF from Production code!!! EMQOS. A great place to work at. \
	 Peacefule place Girinagar!!!Hello World. Running from flash and Writing\
	  to SF from Production code!!! EMQOS. A great place to work at. Testing!!!!!!!!"};
	  
#endif	  
	  




on tile[0] : out port cs = XS1_PORT_1A;
on tile[0] : out port clk = XS1_PORT_1L;
on tile[0] : out port mosi = XS1_PORT_1I;
on tile[0] : in port miso = XS1_PORT_1J; 



void Flash_SPI_Chip_Enable(){
  cs <: 0;	
}

void Flash_SPI_Chip_Disable(){
  cs <: 1;	
}


unsigned inverse(unsigned byte)
{
unsigned mask = 1, result = 0;
 while(mask)
 {
 if (byte & 0x80)
 result |= mask;
 mask <<= 1;
 byte <<= 1;
 }
 return(result);
}



unsigned SPI_transfer(unsigned byte)
		{
			unsigned counter;
			unsigned tmp;
			 for(counter = 8; counter; counter--)
			 {
				 if (byte & 0x80)
				 mosi <: 1;
				 else
				 mosi <: 0;
				 byte <<= 1;
				 clk <: 0; /* a slave latches input data bit */
				 miso :> tmp;
				 if (tmp)
				 byte |= 0x01;
				 clk <: 1; /* a slave shifts out next output data bit */
			 }
			 return(byte);
		} 


/* a byte transfer in (1,1) mode */
//unsigned SPI_transfer1(unsigned byte)
//{
	//unsigned counter;
	//unsigned tmp;
	////byte = inverse(byte);
	 //for(counter = 8; counter; counter--)
	 //{
		 //if (byte & 0x80)
		 //mosi <: 1;
		 //else
		 //mosi <: 0;
		 ////delay_milliseconds(1);
		 //delay_microseconds(100);
		 //clk <: 0; /* a slave shifts out output data bit */
		 ////delay_milliseconds(1);
		 //delay_microseconds(100);
		 //byte <<= 1;
		 //miso :> tmp;
		//// delay_milliseconds(1);
		 //delay_microseconds(100);
		 //if (tmp)
		 //byte |= 0x01;
		 //clk <: 1; /* a slave latches input data bit */
		//// delay_milliseconds(1);
		 //delay_microseconds(100);
	 //}
 //return(byte);
 
//}
/* a byte transfer in (1,1) mode */
unsigned SPI_transfer1(unsigned byte)
{
	unsigned counter;
	unsigned tmp;
	//byte = inverse(byte);
	 for(counter = 8; counter; counter--)
	 {
		 if (byte & 0x80)
		 mosi <: 1;
		 else
		 mosi <: 0;
		 delay_microseconds(100);
		
		 clk <: 1; /* a slave shifts out output data bit */
		delay_microseconds(100);
		//delay_ticks(100);
		 byte <<= 1;
		 miso :> tmp;
		//delay_microseconds(100);
		 if (tmp)
		 byte |= 0x01;
		 clk <: 0; /* a slave latches input data bit */
		delay_microseconds(100);
		//delay_ticks(100);
	 }
 return(byte);
 
}

/* a byte transfer in (1,1) mode */
unsigned SPI_transfer2(unsigned byte)
{
	unsigned counter;
	unsigned tmp;
	//byte = inverse(byte);
	 for(counter = 8; counter; counter--)
	 {
		 if (byte & 0x80)
		 mosi <: 1;
		 else
		 mosi <: 0;
		 //delay_milliseconds(1);
		 delay_microseconds(100);
		 clk <: 1; /* a slave shifts out output data bit */
		 //delay_milliseconds(1);
		 delay_microseconds(100);
		delay_ticks(10);
		 byte <<= 1;
		 miso :> tmp;
		// delay_milliseconds(1);
		// delay_microseconds(100);
		 if (tmp)
		 byte |= 0x01;
		 clk <: 0; /* a slave latches input data bit */
		// delay_milliseconds(1);
		 delay_microseconds(100);
		//delay_ticks(10);
	 }
 return(byte);
 
}

/* a byte transfer in (1,1) mode */
unsigned SPI_transfer3(unsigned byte)
{
	unsigned counter;
	unsigned tmp;
	//byte = inverse(byte);
	//if (byte == 0x3)
		//printf("Errorrrrrrr.....!!!!");
	 for(counter = 8; counter; counter--)
	 {
		 if (byte & 0x80)
		 mosi <: 1;
		 else
		 mosi <: 0;
       // delay_microseconds(1);
		delay_ticks(80);
		//delay_ticks(10);
		
		 clk <: 1; /* a slave shifts out output data bit */
		//delay_microseconds(1);
		//delay_ticks(10);
		delay_ticks(80);
		 byte <<= 1;
		 miso :> tmp;
		//delay_microseconds(100);
		 if (tmp)
		 byte |= 0x01;
		 clk <: 0; /* a slave latches input data bit */
		//delay_microseconds(1);
		//delay_ticks(10);
		delay_ticks(80);
	 }
 return(byte);
 
}

uint32_t Release_Power_Down()
{
	delay_milliseconds(1);
	uint8_t Status_Register_buf[5];
	//Flash_Read_Unique_ID();
	Status_Register_buf[0] = 0xAB;//
	Status_Register_buf[1] = 0x00;
	Status_Register_buf[2] = 0x00;
	Status_Register_buf[3] = 0x00;
	Flash_SPI_Chip_Enable();
	//Flash_SPI_TX(Status_Register_buf,4);
	SPI_transfer1(Status_Register_buf[0]);
	SPI_transfer1(Status_Register_buf[1]);
	SPI_transfer1(Status_Register_buf[2]);
	SPI_transfer1(Status_Register_buf[3]);
	SPI_transfer1(Status_Register_buf[4]);

	delay_milliseconds(1);
	//Flash_SR = Flash_SPI_RX();
	return 0;
}



static uint8_t Flash_Read_Status_Register_1()
{
	uint8_t Flash_SR=0;
	uint8_t Status_Register_buf[2];

	Status_Register_buf[0] = Read_Status_Register_1;
	Status_Register_buf[1] = 0x00;
	Flash_SPI_Chip_Enable();
	delay_microseconds(1);
	SPI_transfer3(Status_Register_buf[0]);
	SPI_transfer3(Status_Register_buf[1]);

	Flash_SR = SPI_transfer3(0);
	//delay_microseconds(500);

	Flash_SPI_Chip_Disable();
	delay_microseconds(1);
	return Flash_SR;
}

static uint8_t Flash_Read_Status_Register_2()
{
	uint8_t Flash_SR=0;
	uint8_t Status_Register_buf[2];

	Status_Register_buf[0] = Read_Status_Register_2;
	Status_Register_buf[1] = 0x00;
	Flash_SPI_Chip_Enable();
	SPI_transfer2(Status_Register_buf[0]);
	SPI_transfer2(Status_Register_buf[1]);

	Flash_SR = SPI_transfer2(0);
	delay_microseconds(50);

	Flash_SPI_Chip_Disable();
	delay_microseconds(100);
	return Flash_SR;
}


static uint8_t Flash_Read_Status_Register_3()
{
	uint8_t Flash_SR=0;
	uint8_t Status_Register_buf[2];

	Status_Register_buf[0] = Read_Status_Register_3;
	Status_Register_buf[1] = 0x00;
	Flash_SPI_Chip_Enable();
	SPI_transfer2(Status_Register_buf[0]);
	SPI_transfer2(Status_Register_buf[1]);

	Flash_SR = SPI_transfer1(0);
	//delay_microseconds(500);

	Flash_SPI_Chip_Disable();
	delay_microseconds(100);
	return Flash_SR;
}





/**************************************************************************/
/*  Name        : Flash_WREN                                              */
/*  Parameters  : void                                                    */
/*  Returns     : void                                                    */
/*  Function    : Enable flash write.                                     */
/*------------------------------------------------------------------------*/
//static void Flash_WREN()
//{
	//Flash_SPI_Chip_Enable();
	//uint8_t WREN_buf[1];
	//WREN_buf[0]=Write_Enable;	/* send Flash_WREN command */
	//SPI_transfer1(WREN_buf[0]);
	////delay_microseconds(10);
	//Flash_SPI_Chip_Disable();
	//delay_microseconds(10);
//}


static void Flash_WREN()
{
	Flash_SPI_Chip_Enable();
	delay_microseconds(1);
	//delay_ticks(10);
	uint8_t WREN_buf[1];
	WREN_buf[0]=Write_Enable;	/* send Flash_WREN command */
	SPI_transfer3(WREN_buf[0]);
	//delay_microseconds(10);
	Flash_SPI_Chip_Disable();
	delay_microseconds(1);
	//delay_ticks(10);
}

static void Flash_WREN1()
{
	Flash_SPI_Chip_Enable();
	//delay_microseconds(10);
	delay_ticks(10);
	uint8_t WREN_buf[1];
	WREN_buf[0]=Write_Enable;	/* send Flash_WREN command */
	SPI_transfer3(WREN_buf[0]);
	delay_microseconds(10);
	Flash_SPI_Chip_Disable();
	//delay_microseconds(10);
	delay_ticks(10);
}


/**************************************************************************/
/*  Name        : Flash_EWSR                                              */
/*  Parameters  : void                                                    */
/*  Returns     : void                                                    */
/*  Function    :Enable write to status register.                       */
/*------------------------------------------------------------------------*/
static void Flash_EWSR()
{
	uint8_t EWSR_buf[1];
	EWSR_buf[0]=Enable_Write_Status_Register;	/* enable writing to the status register */
	Flash_SPI_Chip_Enable();
	delay_microseconds(1);
	SPI_transfer3(EWSR_buf[0]);
	//delay_microseconds(10);
	Flash_SPI_Chip_Disable();
	delay_microseconds(1);
}

/**************************************************************************/
/*  Name        : Flash_WRSR                                              */
/*  Parameters  : uint8_t                                                 */
/*  Returns     : void                                                    */
/*  Function    : Write to status register.                               */
/*------------------------------------------------------------------------*/
static void Flash_WRSR(uint8_t byte)
{
	uint8_t WRSR_buf[2];
	WRSR_buf[0]=Write_Status_Register;		/* enable writing to the status register */
	WRSR_buf[1]=byte;						/* data that will change the status of BPx or BPL (only bits 2,3,7 can be written) */
	Flash_SPI_Chip_Enable();
	delay_microseconds(1);
	SPI_transfer3(WRSR_buf[0]);
	SPI_transfer3(WRSR_buf[1]);
	//delay_milliseconds(10);
	Flash_SPI_Chip_Disable();
	delay_microseconds(1);
}


/**************************************************************************/
/*  Name        : Flash_WRSR                                              */
/*  Parameters  : uint8_t                                                 */
/*  Returns     : void                                                    */
/*  Function    : Write to status register.                               */
/*------------------------------------------------------------------------*/
static void Flash_WRSR2(uint8_t byte)
{
	uint8_t WRSR_buf[2];
	WRSR_buf[0]=Write_Status_Register_2;		/* enable writing to the status register */
	WRSR_buf[1]=byte;						/* data that will change the status of BPx or BPL (only bits 2,3,7 can be written) */
	Flash_SPI_Chip_Enable();
	SPI_transfer2(WRSR_buf[0]);
	SPI_transfer2(WRSR_buf[1]);
	delay_milliseconds(10);
	Flash_SPI_Chip_Disable();
	delay_milliseconds(10);
}


/**************************************************************************/
/*  Name        : Flash_WRSR                                              */
/*  Parameters  : uint8_t                                                 */
/*  Returns     : void                                                    */
/*  Function    : Write to status register.                               */
/*------------------------------------------------------------------------*/
static void Flash_WRSR3(uint8_t byte)
{
	uint8_t WRSR_buf[2];
	WRSR_buf[0]=Write_Status_Register_3;		/* enable writing to the status register */
	WRSR_buf[1]=byte;						/* data that will change the status of BPx or BPL (only bits 2,3,7 can be written) */
	Flash_SPI_Chip_Enable();
	SPI_transfer1(WRSR_buf[0]);
	SPI_transfer1(WRSR_buf[1]);
	delay_milliseconds(10);
	Flash_SPI_Chip_Disable();
}


/**************************************************************************/
/*  Name        : Flash_WREN_Check                                        */
/*  Parameters  : void                                                    */
/*  Returns     : uint8_t                                                 */
/*  Function    : Check WEL bit is set .                                  */
/*------------------------------------------------------------------------*/
static uint8_t  Flash_WREN_Check()
{
	uint16_t count = 0;
	uint8_t  byte = 0;
	uint16_t check_count = 0;

	check_count = 5;

	byte = Flash_Read_Status_Register_1();

	while ( (byte != 0x02) && (++count <= check_count) )       // verify that WEL bit is set //
	{
		byte = Flash_Read_Status_Register_1();
	}
	delay_milliseconds(10);
	return byte;
}



/**************************************************************************/
/*  Name        : Flash_Byte_Program                                      */
/*  Parameters  : uint32_t,uint8_t                                        */
/*  Returns     : void                                                    */
/*  Function    : Write one byte on flash .                               */
/*------------------------------------------------------------------------*/
static void Flash_Byte_Program(uint32_t Dst, uint8_t byte)
{
	uint8_t Byte_Program_buf[5];
	Flash_SPI_Chip_Enable();
	Byte_Program_buf[0] = Byte_Program;/* send Sector Erase command */
	Byte_Program_buf[1] = (uint8_t) (Dst >> 16);/* send 3 address bytes */
	Byte_Program_buf[2] = (uint8_t) (Dst >> 8);
	Byte_Program_buf[3] = (uint8_t) (Dst);
	Byte_Program_buf[4] = byte;

	//Flash_SPI_TX(Byte_Program_buf,5);
	SPI_transfer3(Byte_Program_buf[0]);
	SPI_transfer3(Byte_Program_buf[1]);
	SPI_transfer3(Byte_Program_buf[2]);
	SPI_transfer3(Byte_Program_buf[3]);
	SPI_transfer3(Byte_Program_buf[4]);
		
	
	Flash_SPI_Chip_Disable();
}




unsigned is_busy()
{
  char status;
  status = Flash_Read_Status_Register_1();
 // printf("\n Value of Status Register_1 %d",status);
  return (status & 0x1) == 1;
}

void wait_while_busy()
{
  timer tmr;
  unsigned t;
  int i = 0;

  while (is_busy())
  {
    i++;
    tmr :> t;
    tmr when timerafter(t+100) :> t;
    if(i>3000)break;
  }
}



int sf_write(uint8_t* FTPImagFileBufferptr_sf, uint8_t* SF_Filename, uint32_t uImageSize, uint8_t capture_sequence_number_sf,uint8_t capture_number_sf)
{
	uint32_t i, j;
	char *image;
	char *image1;
	uint8_t  val;
	uint8_t *SF_File_Name;
	SF_File_Name = SF_Filename;
		
	uint32_t ffsize = uImageSize;
	uint32_t fsize = ffsize;
	
	image  = FTPImagFileBufferptr_sf;
	image1 = FTPImagFileBufferptr_sf;
	//image = test_data;
	unsigned startTimer = 0;
	unsigned startTimer_total = 0;
	unsigned currentTimer = 0;
	
	uint32_t lflash_start_address;
	uint32_t lflash_start_read_address;
	
	uint8_t seq_num;
	seq_num = capture_sequence_number;
	//seq_num = 0;
	
		
	printf("\nIn Shared Flash write function =============>>>>>>>>>>>>>>\n");
	#ifdef PRODUCTION
	sprintf(tempPrintString, "\nIn Shared Flash write function =============>>>>>>>>>>>>>>\n");
	Debug_TextOut(0, tempPrintString);	
	sprintf(tempPrintString, "\n File name %s Capture sequence number is %d capture number is %d\n",SF_Filename,seq_num,capture_number);
	Debug_TextOut(0, tempPrintString);
	#endif
	printf("\nFile name %s Capture sequence number is %d capture number is %d\n",SF_Filename,seq_num,capture_number);
	
	asm volatile("gettime %0" : "=r" (startTimer_total));
	// sprintf(tempPrintString, "\n File name %s Capture sequence number is %d capture number is %d\n",SF_Filename,seq_num,capture_number);
	// Debug_TextOut(0, tempPrintString);

 	
	//lflash_start_address= 0x010000 + (uint32_t)(capture_sequence_number * 0x010000 *4);//flash address with one capture
	//lflash_start_address= 0x0FFFF + (capture_sequence_number * 0x00FFFF *4);
	if(capture_number == 0)
	
	lflash_start_address= 0x010000 + (uint32_t)(capture_sequence_number * 0x010000 *4);
	else
	lflash_start_address= 0x010000 + (uint32_t)((capture_sequence_number +1) * 0x010000 *4);//+ (0x010000 * capture_number);
    // lflash_start_address= 0x0FFFF + (capture_sequence_number * 0x00FFFF *4) + (0x0FFFF * capture_number);//flash address with any num of capture
	//lflash_start_address = 0x1000; //zero location
	
	lflash_start_read_address = lflash_start_address;
	
	//sprintf(tempPrintString, "\nSeq Num %d, capture number %d, flash address %03X\n", seq_num,capture_number,lflash_start_address);
	//Debug_TextOut(0, tempPrintString);
	//Debug_TextOut(0, tempPrintString);
	printf("\n==>Seq Num %d, capture number %d, flash address %03X\n", seq_num,capture_number,lflash_start_address);
	#ifdef PRODUCTION
	sprintf(tempPrintString, "\nFile size is %d\n", ffsize);
	Debug_TextOut(0, tempPrintString);
	#endif
	printf("\nFile size is %x\n", ffsize);
	
	
	
	
	///////////////////  Testing  Signals  /////////////////////////////
	
    //Flash_WREN1(); // Write Enable                         06
	//Flash_EWSR();  // Enable to write Register             50
	//Flash_WRSR(0x02);// write to status register           01, 02
	////delay_milliseconds(1);
	////delay_microseconds(1);
	//Flash_WREN1(); // Write Enable                         06
	////delay_microseconds(1);
	////delay_milliseconds(1);
	//val = Flash_Read_Status_Register_1();  // Read status Register  05
	//printf("\n Value of Status Register_1 is  %d\n",val);
	
	/////////////////////////////END////////////////////////////////////
	
	////// Manufacture ID Start   //////////////////////////////////////
	uint8_t  flash_id_cmd[6]= {0};
	uint8_t  flash_id[8] = {0};
	flash_id_cmd[0] = 0x90;
	flash_id_cmd[1] = 0x00;
	flash_id_cmd[2] = 0x00;
	flash_id_cmd[3] = 0x00;
	flash_id_cmd[4] = 0x00;
		
	cs <: 0;
	for(i=0;i<5;i++)
	flash_id[i] = SPI_transfer1(flash_id_cmd[i]);	
	//cs <: 1;	
	// delay_milliseconds(1);	
	
	for(i=0;i<8;i++)
	flash_id[i] = SPI_transfer1(0);
		
	cs <: 1;	
	/*	
	for(i=0;i<8;i++)
		{//val = inverse(flash_id[i]);
			val = flash_id[i];
			printf("\n Device ID read is %d ",val);
			#ifdef PRODUCTION
			sprintf(tempPrintString, "\nDevice ID == %d\n", val);
			Debug_TextOut(0, tempPrintString);
			#endif			                
	    }
	*/	
		printf("\n Manufacture ID read is = ");
		for(i=0;i<8;i++)
		{
			printf(" %02X ",flash_id[i]);		                
	    }
		printf("\r\n");
	//while(1);	
	//// Manufacture ID  End/////////////////////////////////////////////////	
	
	////// Device ID Start   ///////////////////////////////////////////
	//uint8_t  flash_id_cmd[6]= {0};
	//uint8_t  flash_id[8] = {0};
	
	flash_id_cmd[0] = 0x4B;
	flash_id_cmd[1] = 0x00;
	flash_id_cmd[2] = 0x00;
	flash_id_cmd[3] = 0x00;
	flash_id_cmd[4] = 0x00;
	flash_id_cmd[5] = 0x00;
		
	cs <: 0;
	for(i=0;i<6;i++)
	flash_id[i] = SPI_transfer1(flash_id_cmd[i]);	
	//cs <: 1;	
	// delay_milliseconds(1);	
	
	for(i=0;i<8;i++)
	flash_id[i] = SPI_transfer1(0);
		
	cs <: 1;	
	
	printf("\nDevice ID read is \n");
		
	for(i=0;i<8;i++)
		{//val = inverse(flash_id[i]);
			val = flash_id[i];
			printf(" %d ",val);
			#ifdef PRODUCTION
			sprintf(tempPrintString, "\nDevice ID == %d\n", val);
			Debug_TextOut(0, tempPrintString);
			#endif			                
	    }
		
		printf("\nDevice ID read is = ");
		for(i=0;i<8;i++)
		{
			printf(" %02X ",flash_id[i]);		                
	    }
		printf("\r\n");
	//while(1);	
	//// Device ID  End/////////////////////////////////////////////////
	
	
	//// Erase Start ///////////////////////////////////////////////////
	// Erase 4 KB 
	
	    
	uint8_t  result=0;
	//uint32_t address=2000;
	//uint8_t Batch_Erase_buf[4];
	
	//////////////// Unlock Sector /////
	
	
    Flash_WREN();
   //printf("\n Before waiting busy\n");
	wait_while_busy();
	Flash_EWSR();
	//printf("\n Before waiting busy\n");
	wait_while_busy();
	Flash_WRSR(0x02);//enable write to status register
	Flash_WREN();
	delay_milliseconds(1);
	result = Flash_Read_Status_Register_1();
	if(result!=0x02)
	{
		delay_milliseconds(14);
		Flash_WREN();
		//printf("\n Before waiting busy\n");
		wait_while_busy();
		result = Flash_Read_Status_Register_1();
	}
		
	
	//uint8_t  result=0;
	uint32_t address=0;
	uint8_t Batch_Erase_buf[4];
	
	//address= SF_FileAddress;//flash_start_address;
	//lflash_start_address = 0x1000;
	//address = 0x000000;
	address = lflash_start_address;
	

   ////// Release_Power_Down();


	//////////////////////// Erase 4 Blocks //////////////////////////////////////////////////////////////Erase 4 Blocks/////Block 1
	//// Need to put as function later
	asm volatile("gettime %0" : "=r" (startTimer));	
		
	result = Flash_Read_Status_Register_1();
//	printf("\n Status Register-1  is %d  should be 2\n\n",result);
	
	{
	if(result!=0x02)
	{
		//Release_Power_Down();
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_EWSR();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		delay_milliseconds(1);
		result = Flash_Read_Status_Register_1();
		if(result!=0x02)
		{
			delay_milliseconds(1);
			Flash_WREN();
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			result = Flash_Read_Status_Register_1();
		}		
	}
	
	if(result!=0x02)
	{
		//Release_Power_Down();
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_EWSR();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		delay_milliseconds(1);
		result = Flash_Read_Status_Register_1();
		if(result!=0x02)
		{
			delay_milliseconds(1);
			Flash_WREN();
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			result = Flash_Read_Status_Register_1();
		}		
	}

    printf("\n\n\nStatus Register-1  is %d  should be 2 Before Erase Command-1",result);
    #ifdef PRODUCTION
    Debug_TextOut(0,"\nShared flash Erase function enter \n");
    sprintf(tempPrintString," \n Status Register-1  is %d  should be 2 Before Erase Command",result);
	Debug_TextOut(0, tempPrintString);
    sprintf(tempPrintString, "\nBefore Erase address is %x \n",address);
	Debug_TextOut(0, tempPrintString);
	#endif
	printf("\nBefore Erase address is %x \n",address);
	if(result==0x02)
	{
		//Flash_Sector_Erase(address);
		Batch_Erase_buf[0] = Flash_64KByte_Block_Erase;
		//Batch_Erase_buf[0] = Flash_4KByte_Sector_Erase;/* send Sector Erase command */
		//Batch_Erase_buf[0] = Chip_Erase;
		Batch_Erase_buf[1] = (uint8_t) (address >> 16);		/* send 3 address bytes */
		Batch_Erase_buf[2] = (uint8_t) (address >> 8);
		Batch_Erase_buf[3] = (uint8_t) (address);
		Flash_SPI_Chip_Enable();       /* enable device */
		delay_milliseconds(2);
		SPI_transfer3(Batch_Erase_buf[0]);
		SPI_transfer3(Batch_Erase_buf[1]);
		SPI_transfer3(Batch_Erase_buf[2]);
		SPI_transfer3(Batch_Erase_buf[3]);

		delay_milliseconds(5);//2809
		Flash_SPI_Chip_Disable();       /* disable device */
		delay_milliseconds(1);
		//printf("\n Before waiting busy\n");
		wait_while_busy();
		
		printf("\n  Erase Done-1\n",result);
		
	}	
}
	//////////////////////// Erase 4 Blocks //////////////////////////////////////////////////////////////Erase 4 Blocks/////Block 2
	//address = address + 6535;
	if(ffsize > 0x10000)	//PS:24032023 smart erase 
	{
	address = address + 0x010000;
       // address = address + 0x00ffff;
	
	result = Flash_Read_Status_Register_1();
	//printf("\nStatus Register-1  is %d  should be 2\n",result);
	

	if(result!=0x02)
	{
		//Release_Power_Down();
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_EWSR();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		delay_milliseconds(1);
		result = Flash_Read_Status_Register_1();
		if(result!=0x02)
		{
			delay_milliseconds(1);
			Flash_WREN();
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			result = Flash_Read_Status_Register_1();
		}
		
	}
   
    
	if(result!=0x02)
	{
		//Release_Power_Down();
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_EWSR();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		delay_milliseconds(1);
		result = Flash_Read_Status_Register_1();
		if(result!=0x02)
		{
			delay_milliseconds(1);
			Flash_WREN();
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			result = Flash_Read_Status_Register_1();
		}
		
	}
	
	if(result!=0x02)
	{
		//Release_Power_Down();
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_EWSR();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		delay_milliseconds(1);
		result = Flash_Read_Status_Register_1();
		if(result!=0x02)
		{
			delay_milliseconds(1);
			Flash_WREN();
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			result = Flash_Read_Status_Register_1();
		}		
	}
     printf("\nStatus Register-1  is %d  should be 2 Before Erase Command-2",result);
    #ifdef PRODUCTION
    Debug_TextOut(0,"\n	Shared flash erase function enter \n");
    sprintf(tempPrintString," \n Status Register-1  is %d  should be 2 Before Erase Command",result);
	Debug_TextOut(0, tempPrintString);
    sprintf(tempPrintString, "\nBefore Erase-2 address is %x \n",address);
	Debug_TextOut(0, tempPrintString);
	#endif
	printf("\nBefore Erase-2 address is %x \n\n",address);
		if(result==0x02)
		{
			//Flash_Sector_Erase(address);
			Batch_Erase_buf[0] = Flash_64KByte_Block_Erase;
			//Batch_Erase_buf[0] = Flash_4KByte_Sector_Erase;/* send Sector Erase command */
			//Batch_Erase_buf[0] = Chip_Erase;
			Batch_Erase_buf[1] = (uint8_t) (address >> 16);		/* send 3 address bytes */
			Batch_Erase_buf[2] = (uint8_t) (address >> 8);
			Batch_Erase_buf[3] = (uint8_t) (address);
			Flash_SPI_Chip_Enable();       /* enable device */
			delay_milliseconds(2);
			SPI_transfer3(Batch_Erase_buf[0]);
			SPI_transfer3(Batch_Erase_buf[1]);
			SPI_transfer3(Batch_Erase_buf[2]);
			SPI_transfer3(Batch_Erase_buf[3]);

			delay_milliseconds(5);//2809
			Flash_SPI_Chip_Disable();       /* disable device */
			delay_milliseconds(1);
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			
			printf("\n  Erase Done-2\n",result);
			
		}	
	}
//////////////////////// Erase 4 Blocks //////////////////////////////////////////////////////////////Erase 4 Blocks/////Block 3
    //address = address + 6535;
    if(ffsize > 0x20000)	//PS:24032023 smart erase 
	{
	address = address + 0x010000;
    //address = address + 0x00ffff;
	result = Flash_Read_Status_Register_1();
//	printf("\nStatus Register-1  is %d  should be 2\n",result);
	

	if(result!=0x02)
	{
		//Release_Power_Down();
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_EWSR();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		delay_milliseconds(1);
		result = Flash_Read_Status_Register_1();
		if(result!=0x02)
		{
			delay_milliseconds(1);
			Flash_WREN();
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			result = Flash_Read_Status_Register_1();
		}
		
	}
   
	if(result!=0x02)
	{
		//Release_Power_Down();
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_EWSR();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		delay_milliseconds(1);
		result = Flash_Read_Status_Register_1();
		if(result!=0x02)
		{
			delay_milliseconds(1);
			Flash_WREN();
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			result = Flash_Read_Status_Register_1();
		}
		
	}
	
		if(result!=0x02)
	{
		//Release_Power_Down();
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_EWSR();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		delay_milliseconds(1);
		result = Flash_Read_Status_Register_1();
		if(result!=0x02)
		{
			delay_milliseconds(1);
			Flash_WREN();
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			result = Flash_Read_Status_Register_1();
		}		
	}
	
	printf("\nStatus Register-1  is %d  should be 2 Before Erase Command-3",result);
    #ifdef PRODUCTION
    Debug_TextOut(0,"\nShared flash erase function enter \n");
    sprintf(tempPrintString," \n Status Register-1  is %d  should be 2 Before Erase Command",result);
	Debug_TextOut(0, tempPrintString);
    sprintf(tempPrintString, "\nBefore Erase-3 address is %x \n",address);
	Debug_TextOut(0, tempPrintString);
	#endif

	printf("\nBefore Erase-3 address is %x \n\n",address);
		if(result==0x02)
		{
			//Flash_Sector_Erase(address);
			Batch_Erase_buf[0] = Flash_64KByte_Block_Erase;
			//Batch_Erase_buf[0] = Flash_4KByte_Sector_Erase;/* send Sector Erase command */
			//Batch_Erase_buf[0] = Chip_Erase;
			Batch_Erase_buf[1] = (uint8_t) (address >> 16);		/* send 3 address bytes */
			Batch_Erase_buf[2] = (uint8_t) (address >> 8);
			Batch_Erase_buf[3] = (uint8_t) (address);
			Flash_SPI_Chip_Enable();       /* enable device */
			delay_milliseconds(2);
			SPI_transfer3(Batch_Erase_buf[0]);
			SPI_transfer3(Batch_Erase_buf[1]);
			SPI_transfer3(Batch_Erase_buf[2]);
			SPI_transfer3(Batch_Erase_buf[3]);

			delay_milliseconds(5);//2809
			Flash_SPI_Chip_Disable();       /* disable device */
			delay_milliseconds(1);
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			
			printf("\n  Erase Done-3\n",result);
			
		}	
	}
//////////////////////// Erase 4 Blocks //////////////////////////////////////////////////////////////Erase 4 Blocks/////Block 4
    //address = address + 6535;
    if(ffsize > 0x30000)	//PS:24032023 smart erase 
	{
	address = address + 0x010000;
   // address = address + 0x00ffff;
	result = Flash_Read_Status_Register_1();
	//printf("\n Status Register-1  is %d  should be 2\n",result);
	if(result!=0x02)
	{
		//Release_Power_Down();
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_EWSR();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		//delay_milliseconds(1);
		result = Flash_Read_Status_Register_1();
		if(result!=0x02)
		{
			//delay_milliseconds(14);
			Flash_WREN();
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			result = Flash_Read_Status_Register_1();
		}
		
	}
    
	if(result!=0x02)
	{
		//Release_Power_Down();
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_EWSR();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		delay_milliseconds(1);
		result = Flash_Read_Status_Register_1();
		if(result!=0x02)
		{
			delay_milliseconds(1);
			Flash_WREN();
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			result = Flash_Read_Status_Register_1();
		}		
	}
	
		if(result!=0x02)
	{
		//Release_Power_Down();
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_EWSR();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		delay_milliseconds(1);
		result = Flash_Read_Status_Register_1();
		if(result!=0x02)
		{
			delay_milliseconds(1);
			Flash_WREN();
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			result = Flash_Read_Status_Register_1();
		}		
	}
    printf("\nStatus Register-1  is %d  should be 2 Before Erase Command-4",result);
    #ifdef PRODUCTION
    Debug_TextOut(0,"\nShared flash erase function enter \n");
    sprintf(tempPrintString," \nStatus Register-1  is %d  should be 2 Before Erase Command",result);
	Debug_TextOut(0, tempPrintString);
    sprintf(tempPrintString, "\nBefore Erase-4 address is %x \n",address);
	Debug_TextOut(0, tempPrintString);
	#endif
	printf("\nBefore Erase-4 address is %x \n\n",address);
		if(result==0x02)
		{
			//Flash_Sector_Erase(address);
			Batch_Erase_buf[0] = Flash_64KByte_Block_Erase;
			//Batch_Erase_buf[0] = Flash_4KByte_Sector_Erase;/* send Sector Erase command */
			//Batch_Erase_buf[0] = Chip_Erase;
			Batch_Erase_buf[1] = (uint8_t) (address >> 16);		/* send 3 address bytes */
			Batch_Erase_buf[2] = (uint8_t) (address >> 8);
			Batch_Erase_buf[3] = (uint8_t) (address);
			Flash_SPI_Chip_Enable();       /* enable device */
			delay_milliseconds(2);
			SPI_transfer3(Batch_Erase_buf[0]);
			SPI_transfer3(Batch_Erase_buf[1]);
			SPI_transfer3(Batch_Erase_buf[2]);
			SPI_transfer3(Batch_Erase_buf[3]);

			delay_milliseconds(5);//2809
			Flash_SPI_Chip_Disable();       /* disable device */
			delay_milliseconds(10);
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			
			printf("\n  Erase Done-4\n",result);
			
		}	
		
	   asm volatile("gettime %0" : "=r" (currentTimer));
	   printf("\r\n\n\n Total time after erasing 4 blocks: %dms \n", (((currentTimer - startTimer) / 100) / 1000));
	}
	//////////////////////// Erase 4 Blocks //////////////////////////////////////////////////////////////Erase End  ////////////////////////////////////////////////////
	//// Testing///////////////////////////////////////////////////////
	#if 1
	int jj = 0;
	int ii = 0;
	//printf("\nEvery 1000 data before writing to SF\n");
	printf("\nfirst 900 data before writing to SF\n");
	//for(ii=0;ii<fsize;ii++)
	for(ii=0;ii<900;ii++)
	{
		jj++;
		if(jj== 1000){ jj = 0;printf("%02X ", *image1);}
		// if(((int)fsize - ii)==300)	printf("\n\n Last bytes fsize left %d ctr %d\n\n",(fsize-ii),ii);
		//if(((int)fsize - ii)<300)	printf("%2x",*image1); 	
		image1++;		
	}
	#endif
	///////////////////////////////////////////////////////////////////
	
	//// Write Start ///////////////////////////////////////////////////
	
      result = Flash_Read_Status_Register_1();	
      printf("\n Status Register1 is %d",result);      
      result = Flash_Read_Status_Register_2();	
      printf("\n Status Register2 is %d",result);
      
     // result = (result & ~(0x02));
     Flash_WREN1();
     //printf("\n Before waiting busy\n");
     wait_while_busy();
	 Flash_EWSR();
     //printf("\n Before waiting busy\n");
     wait_while_busy();
	 Flash_WRSR(0x02);//enable write to status register
	 Flash_WREN();
	//printf("\n Before waiting busy\n");
     wait_while_busy();
	 delay_milliseconds(1);
		if(result!=0x02)
		{
			delay_milliseconds(14);
			Flash_WREN();
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			result = Flash_Read_Status_Register_1();
		}
       Flash_WRSR2(0); //Disable Quad Mode
      //printf("\n Before waiting busy\n");
       wait_while_busy();      
       result = Flash_Read_Status_Register_2();	
       printf("\n Status Register2 is %d",result);      
       result = Flash_Read_Status_Register_3();	
       printf("\n Status Register3 is %d",result);
      
      
      
	//uint8_t  result=0;//,k=0;
	//uint16_t      read_offset=0,no_of_read_chunks,l=0;
	const uint8_t* data_ptr ;//= (const uint8_t *) data_pool;
	//uint8_t  data;
	//uint32_t lflash_start_address;
	////uint16_t read_bytes_left=0,read_Chunk_size=0;
	//lflash_start_address= SF_FileAddress;//flash_start_address;
	uint8_t Byte_Program_buf[5];
	uint8_t x;

	
	////DelayMs(1);
	Flash_WREN();
	//printf("\n Before waiting busy\n");
    wait_while_busy();
	Flash_EWSR();
	//printf("\n Before waiting busy\n");
    wait_while_busy();
	Flash_WRSR(0x02);//enable write to status register
	Flash_WREN();
	//printf("\n Before waiting busy\n");
    wait_while_busy();
	result = Flash_WREN_Check();//261
	

	result=Flash_Read_Status_Register_1();
	//Debug_TextOut(0,"T2");
	printf("\nStatus Register-1  is %d  should be 2 Before Write Command Check 1",result);
	#ifdef PRODUCTION
	sprintf(tempPrintString,"\nStatus Register-1  is %d  should be 2 Before Write Command Check 1",result);
	Debug_TextOut(0, tempPrintString);
	#endif
	if(result!=0x02)
	{
		Release_Power_Down();
		Flash_WREN();
		////printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_EWSR();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		delay_milliseconds(1);
		result = Flash_Read_Status_Register_1();
		if(result!=0x02)
		{
			delay_milliseconds(14);
			Flash_WREN();
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			result = Flash_Read_Status_Register_1();
		}
		
	}
	if(result!=0x02)
	{
		Flash_WREN();
		Flash_EWSR();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		delay_milliseconds(1);
		result = Flash_Read_Status_Register_1();
		if(result!=0x02)
		{
			delay_milliseconds(14);
			Flash_WREN1();
			result = Flash_Read_Status_Register_1();
		}
		
	}
	//Debug_TextOut(0,"T3");
	result=Flash_Read_Status_Register_1();
	printf("\nStatus Register-1  is %d  should be 2 Before Write Command Check 2",result);
	#ifdef PRODUCTION
	sprintf(tempPrintString,"\nStatus Register-1  is %d  should be 2 Before Write Command Check 2",result);
	
	Debug_TextOut(0, tempPrintString);
	sprintf(tempPrintString, "\nWriting to shared flash\n");
	Debug_TextOut(0, tempPrintString);
	#endif
	
//	lflash_start_address = 0x1000;	
   // lflash_start_address = lflash_start_address & 0xFFFFFF00;	                
  
	if(result==0x02)
	//if(result&(0x02))
	{
		#ifdef PRODUCTION
	    sprintf(tempPrintString, "\nBefore for loop of write ");
	
		Debug_TextOut(0, tempPrintString);
		#endif
		printf("\nBefore for loop of write address is %x \n",lflash_start_address);
		asm volatile("gettime %0" : "=r" (startTimer));	
		
		int k =0;
		int l = 0;
	    ////   Write part Start //////////
		//lflash_start_address = 0;					
		Flash_SPI_Chip_Enable();
		Byte_Program_buf[0] = Byte_Program;
		Byte_Program_buf[1] = (uint8_t) (lflash_start_address >> 16);/* send 3 address bytes */
		Byte_Program_buf[2] = (uint8_t) (lflash_start_address >> 8);
		Byte_Program_buf[3] = (uint8_t) (lflash_start_address);
		//Byte_Program_buf[4] = byte;
	
		//Flash_SPI_TX(Byte_Program_buf,5);
		SPI_transfer3(Byte_Program_buf[0]);
		SPI_transfer3(Byte_Program_buf[1]);
		SPI_transfer3(Byte_Program_buf[2]);
		SPI_transfer3(Byte_Program_buf[3]);
		
		//// Address write done  ////
		
		//// Writing File present///////////////////////////////////////
		//SPI_transfer3(0x01);
		//printf("%x",0x01);
		
		printf("\nWriting to Flash\n\n");
		//char pr[] = "1";
		//uint8_t pr[4] = {'Y','N','\\','0'};
		//SPI_transfer3('Y');
		//SPI_transfer3(pr[0]);
		printf("File exist character written is ==>%c\n\n",pr[0]);
		////Writing File info i.e. file name, file address, file size
		#ifdef PRODUCTION
		sprintf(tempPrintString, "\nFile name is %s \n",SF_File_Name);
	
		Debug_TextOut(0, tempPrintString);
		#endif
		
		printf("\n File Name is %s\n",SF_File_Name);
		//printf("\n File Name is\n");
		//const uint8_t* data_ptr = (const uint8_t *) sf_filename1;
		//memcpy(sf_filename1,SF_File_Name,45);
		//sprintf(sf_filename1,"Y%s",SF_File_Name,45);//vamsi code
		sprintf(sf_filename1,"Y%s",SF_File_Name);
		//const uint8_t* data_ptr = (const uint8_t *) sf_filename1;
		//uint8_t bytes[2];
		printf("\nFile name is %s\n",sf_filename1);
		//printf("\nLength of file name is %d", strlen(sf_filename1));
		
		//for(i=0; i<45; i++)
		SPI_transfer3(clear_flash_start);
		SPI_transfer3(clear_flash_start);
		SPI_transfer3(clear_flash_start);
		SPI_transfer3(clear_flash_start);
		lflash_start_address++;
		lflash_start_address++;
		lflash_start_address++;
		lflash_start_address++;
		//SPI_transfer3('1');//PS:27022023
		//SPI_transfer3('1');//PS:27022023
		//SPI_transfer3('1');//PS:27022023
		//SPI_transfer3('1');//PS:27022023
		//for(i=0; i<45; i++)
		for(i=0; i<FileNameLength; i++)
		{SPI_transfer3(sf_filename1[i]);
		//printf(" i%d,%c", i, (char)sf_filename1[i]);
		k++;
		lflash_start_address++;
		}
		SPI_transfer3(0);//PS:27022023	//Null termination
		k++;
		lflash_start_address++;
		//Added to fill the filename array
		//SPI_transfer3(0);//PS:27022023
		//SPI_transfer3(0);//PS:27022023
		//SPI_transfer3(0);//PS:27022023
		//SPI_transfer3(0);//PS:27022023
		SPI_transfer3(0);//PS:27022023
		k++;
		lflash_start_address++;
		
		//SPI_transfer3(pr[3]);//PS:27022023
		//{printf("%c",data_ptr[i]);SPI_transfer3(data_ptr[i]);};
		
		////address 32 bit as byte by byte//////////////////////////////
		//x = (lflash_start_address >> (3 << 3)) & 0xFF;
        //SPI_transfer3(x);
        //printf("%c",x);
        //x = (lflash_start_address >> (2 << 3)) & 0xFF;
        //SPI_transfer3(x);
        //printf("%c",x);
        //x = (lflash_start_address >> (1 << 3)) & 0xFF;
        //SPI_transfer3(x);
        //printf("%c",x);
        //x = (lflash_start_address >> (0 << 3)) & 0xFF;
        //SPI_transfer3(x);
        //printf("%c",x);
        
        // File Size 16 bit as byte by byte ////////////////////////////
        //bytes[0] = (fsize >> 8)& 0x00FF;// high byte 
        //bytes[1] = fsize & 0x00FF; // low byte  		
        //SPI_transfer3(bytes[0]);
        //printf("%d",bytes[0]);
        //SPI_transfer3(bytes[1]);
        //printf("%d",bytes[1]);
        
        char length[20];
        sprintf(length,"ffsize: %lu",ffsize);
        sprintf(tempPrintString, "\nFile size is %s ",length);
	
		Debug_TextOut(0, tempPrintString);
        //for(i=0;i<sizeof(length);i++)
        //{ 
         //printf("%c",length[i]); SPI_transfer3(length[i]);
        //};
        uint8_t *ptr;
        ptr = (uint8_t *) (&ffsize);
        for(i=0;i<4;i++)
        {
			printf("\r\nSF: i: %i Data: %02X",i, (uint8_t) *ptr);
			SPI_transfer3(*ptr);
			//ptr--;
			ptr++;//PS:27022023
			k++;
			lflash_start_address++;
		}
        
        ////////// For test data ///////////////////////////////////////
		// So far 53 byte written Next image data for rest of the sector.
		//for(i=0;i< 3935; i++)
		//{SPI_transfer3(*image);image++;};
		//Debug_TextOut(0,"\nImage File\n");
		printf("\n Image File \n");
		//for(i=0;i< 201; i++)
		for(i=0;i< 201; i++)			
		{
			//PS:20032023 commented below code for time optimization
			//printf("%02X ",*image);
			SPI_transfer3(*image);image++;
			k++;
			lflash_start_address++;
		};
	
		//Debug_Output1(0,"%c",*image);
		Flash_SPI_Chip_Disable();
		delay_milliseconds(1);
		//ffsize = 1000;//2560;
		// Writing rest of the image to next sectors
		ffsize = ffsize - 201;
		
		//lflash_start_address = lflash_start_address + 256 + 4;
		printf("\nNumber of bytes written in 1st page %d\n",k);
		
		/////// For Test Data //////////////////////////////////////////
		
		//Flash_SPI_Chip_Enable();
		//Byte_Program_buf[0] = Byte_Program;
		//Byte_Program_buf[1] = (uint8_t) (lflash_start_address >> 16);/* send 3 address bytes */
		//Byte_Program_buf[2] = (uint8_t) (lflash_start_address >> 8);
		//Byte_Program_buf[3] = (uint8_t) (lflash_start_address);
		
		//SPI_transfer3(Byte_Program_buf[0]);
		//SPI_transfer3(Byte_Program_buf[1]);
		//SPI_transfer3(Byte_Program_buf[2]);
		//SPI_transfer3(Byte_Program_buf[3]);
		
		//for(i=0;i<256;i++)
			//{SPI_transfer3(*image);image++;}
			
	
		//Flash_SPI_Chip_Disable();
	 ///////////////////////////////////////////////////////////////////
	 #ifdef PRODUCTION
	 sprintf(tempPrintString, "\nDone writing meta data writing rest of the image\n");
	 Debug_TextOut(0, tempPrintString);
	 #endif
		k=0;
		//// For Actual image ////////////
		int wflag = 1;
		while(wflag){
				    							
					 Flash_WREN();
					 wait_while_busy();
					 delay_ticks(20);					 							
					 Flash_WREN();
					 wait_while_busy();
					 delay_ticks(20);	
					
					Flash_SPI_Chip_Enable();
					Byte_Program_buf[0] = Byte_Program;
					Byte_Program_buf[1] = (uint8_t) (lflash_start_address >> 16);/* send 3 address bytes */
					Byte_Program_buf[2] = (uint8_t) (lflash_start_address >> 8);
					Byte_Program_buf[3] = (uint8_t) (lflash_start_address);
					
					SPI_transfer3(Byte_Program_buf[0]);
					SPI_transfer3(Byte_Program_buf[1]);
					SPI_transfer3(Byte_Program_buf[2]);
					SPI_transfer3(Byte_Program_buf[3]);
					if((ffsize<256)|| (ffsize == 256))
					{
						//printf( "\n\r Last chunck of %d\n\n",ffsize);
					 for(i=0;i<ffsize;i++)
						  {SPI_transfer3(*image);image++;lflash_start_address++;}// printf(" %02X",*image);}
							wflag = 0;
							ffsize=0;
							
							//printf( "\n\r End of last packet");
						
					}
					else					
					for(i=0;i<256;i++)
					 {SPI_transfer3(*image);image++;lflash_start_address++; 
						//PS:20032023 commented below code for time optimization
						// if(k< 500)printf(" %02X",*image);k++;
						}
					////printf( "\n\r ffsize = %ld",ffsize);
					Flash_SPI_Chip_Disable();
					delay_ticks(20);
					
					if(fsize == 0)wflag = 0;
					else
						ffsize = ffsize - 256;				
				}
		

     }
	                   
	   asm volatile("gettime %0" : "=r" (currentTimer));
	   printf("\r\n\n\n Time after writing %d bytes: %dms.",fsize, (((currentTimer - startTimer) / 100) / 1000));
	   sprintf(tempPrintString, "\r\n\n\n Time after writing %d bytes: %dms.",fsize, (((currentTimer - startTimer) / 100) / 1000));
		Debug_TextOut(0, tempPrintString);
	   printf("\r\n\n\n Total tiem after Erase and Writing %d bytes: %dms.",fsize, (((currentTimer - startTimer_total) / 100) / 1000));
	   sprintf(tempPrintString, "\r\n\n\n Total time after Erase and Writing %d bytes: %dms.",fsize, (((currentTimer - startTimer_total) / 100) / 1000));
		Debug_TextOut(0, tempPrintString);
	             
		printf("\n  write Done \n",result);
	//	while(1);
		#ifdef PRODUCTION
		sprintf(tempPrintString, "\nWrite Done \n\n");
		Debug_TextOut(0, tempPrintString);
	    #endif
		

	////// Write End  ////////////////////////////////////////////////////
	
	
	
	//////Read Back Verification disabled
	#if 0	//ps:21032023 for time optimization
		if(capture_number == 0)
		lflash_start_address= 0x010000 + (uint32_t)(capture_sequence_number * 0x010000 *4);
		else
		lflash_start_address= 0x010000 + (uint32_t)((capture_sequence_number +1) * 0x010000 *4);

		//lflash_start_address= SF_FileAddress;
		//printf("\nBefore for loop of read address is %x \n",lflash_start_address);
		////uint32_t lflash_start_address = 0;
		lflash_start_address = lflash_start_read_address;
		printf("\nBefore for loop of read address is %x \n",lflash_start_address);
		uint16_t SPI_RX_count=0;
		uint8_t Read_Cont_buf[5];
		uint8_t Read_data[100];
	   // lflash_start_address = 0x1000;
		Read_Cont_buf[0] = Read_Memory;
		Read_Cont_buf[1] = (uint8_t) (lflash_start_address >> 16);
		Read_Cont_buf[2] = (uint8_t) (lflash_start_address >> 8);
		Read_Cont_buf[3] = (uint8_t) (lflash_start_address);
		Read_Cont_buf[4] = 0x00;
		Flash_SPI_Chip_Enable();
		
		
		SPI_transfer1(Read_Cont_buf[0]);
		SPI_transfer1(Read_Cont_buf[1]);
		SPI_transfer1(Read_Cont_buf[2]);
		SPI_transfer1(Read_Cont_buf[3]);
		//SPI_transfer1(Read_Cont_buf[4]);

	  printf("\n Read Data is");
	#endif
 #if 0   //ps:27022023 commented because xmos was hanging at this place
	  for (SPI_RX_count = 0; SPI_RX_count < 2050; SPI_RX_count++)
		{
			
				Read_data[0]/*[SPI_RX_count]*/ = SPI_transfer1(0);
				printf("%2x ",Read_data[0]);

		}
		
#endif
#if 0
             printf("\nImage size is %d\n",(int)fsize);
	          printf("\nMeta Data\n");
	          for(i=0;i<55;i++)
	          {Read_data[0] = SPI_transfer1(0);printf("%c ",Read_data[0]);}	
	         // printf("\nValue of every 500 data in image is\n");
	          printf("\nMeta Data Crossed\n");
#if 0
	        for(i=0;i<(int)fsize; i++)
	          { 
				  Read_data[0] = SPI_transfer3(0);
				  printf("%02x ",Read_data[0]);
				 // if(i==300)break;			  
				  
				  }	
#endif

#endif

#if 0	//ps:21032023 commented for time optimization
		
		/////// Calculating  HASH and printing every 10th byte//////Longitudinal Redundancy
	         uint8_t lrc = 0x00;
	         uint8_t lrc1 = 0x00;
	         i=0;
	         j=0;
	         int k = 0;
	          printf("\nImage size is %d\n",(int)fsize);
	          printf("\nMeta Data\n");
	          for(i=0;i<55;i++)
	          {Read_data[0] = SPI_transfer1(0);printf("%c ", Read_data[0]);}	
	         // printf("\nValue of every 500 data in image is\n");
	          printf("\nMeta Data Crossed\n");
	          printf("\nFirst 900 bytes\n");
	        for(i=0;i<(int)fsize; i++)
	          { 
				  Read_data[0] = SPI_transfer3(0);
				  lrc = (lrc + Read_data[0]) & 0xFF;
				//  if(i<900)printf("%02X ", Read_data[0]);
				///  if(i==200)printf("\nValue of every 1000 data in image is\n");
				  j++;
				  if(j== 1000){
								   j = 0;
								   //printf(" %02X ", Read_data[0]);
								   lrc1 =  lrc;
								   lrc1 = ((lrc1 ^ 0xFF) + 1) & 0xFF; 
								   //printf(" lrc=>%2x",lrc1);
								   lrc = 0;
								}
					
					//if((fsize - i)==300)	printf("\n\n Last bytes\n\n");
				//	if(((int)fsize - i)==300)	printf("\n\n Last bytes fsize left %d ctr %d\n\n",(fsize-i),i);
				//	if(((int)fsize - i)<300)	printf("%2x",Read_data[0]); 
				  
			  }
			  
			// lrc =   ((lrc ^ 0xFF) + 1) & 0xFF;
			 
			// printf("\n LRC is %2x", lrc);           
            
            ////////////////////////////////////////////////////////////

		printf("\n");
		#endif

	Flash_SPI_Chip_Disable();
	
	
		#ifdef PRODUCTION
	            sprintf(tempPrintString, "%s",Read_data);
				Debug_TextOut(0, tempPrintString);
				#endif
	//printf("\n  read Done\n ",result);
	
	return 1;
}



#if 0

int sf_write_test()
{
	uint32_t i, j;
	uint8_t  val;
	char *image;
	uint8_t SF_File_Name[100];
	//SF_File_Name = SF_Filename;
		
	uint16_t fsize = 512;//uImageSize;
	
//	image  = FTPImagFileBufferptr;
	//image = test_data;
	unsigned startTimer = 0;
	unsigned currentTimer = 0;
	
	uint32_t lflash_start_address;
	uint32_t lflash_start_read_address;
	int capture_sequence_number = 3;
	
	lflash_start_address= 0x010000 + (uint32_t)(capture_sequence_number * 0x010000 *4);
	
	//lflash_start_address = 0;
	
	lflash_start_read_address = lflash_start_address;
	
	//lflash_start_address= (capture_sequence_number * 0x00FFFF *4);//flash_start_address;
	
	//sprintf(tempPrintString, "\nSeq Num %d flash address %03X\n", val,lflash_start_address//);
			                //Debug_TextOut(0, tempPrintString);
	
	
	
	sprintf(SF_File_Name, "IMG_FL_132S8028_32018_20230215_093255_01.jpeg");
	printf("\n File name is %s\n",SF_File_Name);
	
	///////////////////  Testing  Signals  //////////////////////////////////
	
	
    //Flash_WREN1(); // Write Enable                         06
	//Flash_EWSR();  // Enable to write Register             50
	//Flash_WRSR(0x02);// write to status register           01, 02
	////delay_milliseconds(1);
	////delay_microseconds(1);
	//Flash_WREN1(); // Write Enable                         06
	////delay_microseconds(1);
	////delay_milliseconds(1);
	//val = Flash_Read_Status_Register_1();  // Read status Register  05
	//printf("\n Value of Status Register_1 is  %d\n",val);
	
	
	
	
	
	
	/////////////////////////////END///////////////////////////////////////
	
	
	
	////// Device ID Start   /////////////////////////////////////////////
	uint8_t  flash_id_cmd[6]= {0};
	uint8_t  flash_id[8] = {0};
	flash_id_cmd[0] = 0x4B;
	flash_id_cmd[1] = 0x00;
	flash_id_cmd[2] = 0x00;
	flash_id_cmd[3] = 0x00;
	flash_id_cmd[4] = 0x00;
	flash_id_cmd[5] = 0x00;
	
	
	 cs <: 0;
	for(i=0;i<6;i++)
	flash_id[i] = SPI_transfer1(flash_id_cmd[i]);	
	//cs <: 1;
	
	
	// delay_milliseconds(1);
	
	
	for(i=0;i<8;i++)
	flash_id[i] = SPI_transfer1(0);
	
	cs <: 1;
	
		
	for(i=0;i<8;i++)
	{//val = inverse(flash_id[i]);
		val = flash_id[i];
	printf("\n Device ID read is %d ",val);
	#ifdef PRODUCTION
	sprintf(tempPrintString, "\nDevice ID == %d\n", val);
			                Debug_TextOut(0, tempPrintString);
			                #endif
			                
			                }
	//while(1);
	
	//// Device ID  End/////////////////////////////////////////////////
	
	
	//// Erase Start ///////////////////////////////////////////////////
	// Erase 4 KB 
	
	    
	uint8_t  result=0;
	//uint32_t address=2000;
	//uint8_t Batch_Erase_buf[4];
	
	//////////////// Unlock Sector /////
	
	
	    Flash_WREN();
	   //printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_EWSR();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		delay_milliseconds(1);
		result = Flash_Read_Status_Register_1();
		if(result!=0x02)
		{
			delay_milliseconds(14);
			Flash_WREN();
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			result = Flash_Read_Status_Register_1();
		}
		
	
	
	
	//uint8_t  result=0;
	uint32_t address=0;
	uint8_t Batch_Erase_buf[4];
	
	//address= SF_FileAddress;//flash_start_address;
	//address = 0x000000;
	address = lflash_start_address;
	

   ////// Release_Power_Down();


	//////////////////////// Erase 4 Blocks ///////////////////////////////////////////////////////////////////Block 1
	//// Need to put as function later
	asm volatile("gettime %0" : "=r" (startTimer));	
	result = Flash_Read_Status_Register_1();
//	printf("\n Status Register-1  is %d  should be 2\n\n",result);
	

	if(result!=0x02)
	{
		//Release_Power_Down();
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_EWSR();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		delay_milliseconds(1);
		result = Flash_Read_Status_Register_1();
		if(result!=0x02)
		{
			delay_milliseconds(1);
			Flash_WREN();
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			result = Flash_Read_Status_Register_1();
		}
		
	}
      printf("\n\n\nStatus Register-1  is %d  should be 2 Before Erase Command-1",result);
      #ifdef PRODUCTION
     Debug_TextOut(0,"\nShared flash Erase function enter \n");
      sprintf(tempPrintString," \n Status Register-1  is %d  should be 2 Before Erase Command",result);
	Debug_TextOut(0, tempPrintString);
    sprintf(tempPrintString, "\nBefore Erase address is %x \n",address);
			                Debug_TextOut(0, tempPrintString);
			                #endif
	printf("\nBefore Erase address is %x \n",address);
		if(result==0x02)
		{
			//Flash_Sector_Erase(address);
			Batch_Erase_buf[0] = Flash_64KByte_Block_Erase;
			//Batch_Erase_buf[0] = Flash_4KByte_Sector_Erase;/* send Sector Erase command */
			//Batch_Erase_buf[0] = Chip_Erase;
			Batch_Erase_buf[1] = (uint8_t) (address >> 16);		/* send 3 address bytes */
			Batch_Erase_buf[2] = (uint8_t) (address >> 8);
			Batch_Erase_buf[3] = (uint8_t) (address);
			Flash_SPI_Chip_Enable();       /* enable device */
			delay_milliseconds(2);
			SPI_transfer3(Batch_Erase_buf[0]);
			SPI_transfer3(Batch_Erase_buf[1]);
			SPI_transfer3(Batch_Erase_buf[2]);
			SPI_transfer3(Batch_Erase_buf[3]);

			delay_milliseconds(5);//2809
			Flash_SPI_Chip_Disable();       /* disable device */
			delay_milliseconds(1);
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			
			printf("\n  Erase Done-1\n",result);
			
		}	
	
	//////////////////////// Erase 4 Blocks ///////////////////////////////////////////////////////////////////Block 2
	//address = address + 6535;
	//address = address + 0x00ffff;
	address = address + 0x010000;
	result = Flash_Read_Status_Register_1();
	//printf("\nStatus Register-1  is %d  should be 2\n",result);
	

	if(result!=0x02)
	{
		//Release_Power_Down();
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_EWSR();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		delay_milliseconds(1);
		result = Flash_Read_Status_Register_1();
		if(result!=0x02)
		{
			delay_milliseconds(1);
			Flash_WREN();
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			result = Flash_Read_Status_Register_1();
		}
		
	}
      printf("\nStatus Register-1  is %d  should be 2 Before Erase Command-2",result);
      #ifdef PRODUCTION
     Debug_TextOut(0,"\n	Shared flash erase function enter \n");
      sprintf(tempPrintString," \n Status Register-1  is %d  should be 2 Before Erase Command",result);
	Debug_TextOut(0, tempPrintString);
    sprintf(tempPrintString, "\nBefore Erase-2 address is %x \n",address);
			                Debug_TextOut(0, tempPrintString);
			                #endif
    
	if(result!=0x02)
	{
		//Release_Power_Down();
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_EWSR();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		delay_milliseconds(1);
		result = Flash_Read_Status_Register_1();
		if(result!=0x02)
		{
			delay_milliseconds(1);
			Flash_WREN();
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			result = Flash_Read_Status_Register_1();
		}
		
	}
	printf("\nBefore Erase-2 address is %x \n\n",address);
		if(result==0x02)
		{
			//Flash_Sector_Erase(address);
			Batch_Erase_buf[0] = Flash_64KByte_Block_Erase;
			//Batch_Erase_buf[0] = Flash_4KByte_Sector_Erase;/* send Sector Erase command */
			//Batch_Erase_buf[0] = Chip_Erase;
			Batch_Erase_buf[1] = (uint8_t) (address >> 16);		/* send 3 address bytes */
			Batch_Erase_buf[2] = (uint8_t) (address >> 8);
			Batch_Erase_buf[3] = (uint8_t) (address);
			Flash_SPI_Chip_Enable();       /* enable device */
			delay_milliseconds(2);
			SPI_transfer3(Batch_Erase_buf[0]);
			SPI_transfer3(Batch_Erase_buf[1]);
			SPI_transfer3(Batch_Erase_buf[2]);
			SPI_transfer3(Batch_Erase_buf[3]);

			delay_milliseconds(5);//2809
			Flash_SPI_Chip_Disable();       /* disable device */
			delay_milliseconds(1);
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			
			printf("\n  Erase Done-2\n",result);
			
		}	
	
//////////////////////// Erase 4 Blocks ///////////////////////////////////////////////////////////////////Block 3
    //address = address + 6535;
    address = address + 0x010000;
    //address = address + 0x00ffff;
	result = Flash_Read_Status_Register_1();
//	printf("\nStatus Register-1  is %d  should be 2\n",result);
	

	if(result!=0x02)
	{
		//Release_Power_Down();
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_EWSR();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		delay_milliseconds(1);
		result = Flash_Read_Status_Register_1();
		if(result!=0x02)
		{
			delay_milliseconds(1);
			Flash_WREN();
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			result = Flash_Read_Status_Register_1();
		}
		
	}
      printf("\nStatus Register-1  is %d  should be 2 Before Erase Command-3",result);
      #ifdef PRODUCTION
     Debug_TextOut(0,"\nShared flash erase function enter \n");
      sprintf(tempPrintString," \n Status Register-1  is %d  should be 2 Before Erase Command",result);
	Debug_TextOut(0, tempPrintString);
    sprintf(tempPrintString, "\nBefore Erase-3 address is %x \n",address);
			                Debug_TextOut(0, tempPrintString);
			                #endif
	if(result!=0x02)
	{
		//Release_Power_Down();
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_EWSR();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		delay_milliseconds(1);
		result = Flash_Read_Status_Register_1();
		if(result!=0x02)
		{
			delay_milliseconds(1);
			Flash_WREN();
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			result = Flash_Read_Status_Register_1();
		}
		
	}
	printf("\nBefore Erase-3 address is %x \n\n",address);
		if(result==0x02)
		{
			//Flash_Sector_Erase(address);
			Batch_Erase_buf[0] = Flash_64KByte_Block_Erase;
			//Batch_Erase_buf[0] = Flash_4KByte_Sector_Erase;/* send Sector Erase command */
			//Batch_Erase_buf[0] = Chip_Erase;
			Batch_Erase_buf[1] = (uint8_t) (address >> 16);		/* send 3 address bytes */
			Batch_Erase_buf[2] = (uint8_t) (address >> 8);
			Batch_Erase_buf[3] = (uint8_t) (address);
			Flash_SPI_Chip_Enable();       /* enable device */
			delay_milliseconds(2);
			SPI_transfer3(Batch_Erase_buf[0]);
			SPI_transfer3(Batch_Erase_buf[1]);
			SPI_transfer3(Batch_Erase_buf[2]);
			SPI_transfer3(Batch_Erase_buf[3]);

			delay_milliseconds(5);//2809
			Flash_SPI_Chip_Disable();       /* disable device */
			delay_milliseconds(1);
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			
			printf("\n  Erase Done-3\n",result);
			
		}	
	
//////////////////////// Erase 4 Blocks ///////////////////////////////////////////////////////////////////Block 4
    //address = address + 6535;
    address = address + 0x010000;
    //address = address + 0x00ffff;
	result = Flash_Read_Status_Register_1();
	//printf("\n Status Register-1  is %d  should be 2\n",result);
	

	if(result!=0x02)
	{
		//Release_Power_Down();
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_EWSR();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		//delay_milliseconds(1);
		result = Flash_Read_Status_Register_1();
		if(result!=0x02)
		{
			//delay_milliseconds(14);
			Flash_WREN();
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			result = Flash_Read_Status_Register_1();
		}
		
	}
      printf("\nStatus Register-1  is %d  should be 2 Before Erase Command-4",result);
      #ifdef PRODUCTION
     Debug_TextOut(0,"\nShared flash erase function enter \n");
      sprintf(tempPrintString," \nStatus Register-1  is %d  should be 2 Before Erase Command",result);
	Debug_TextOut(0, tempPrintString);
    sprintf(tempPrintString, "\nBefore Erase-4 address is %x \n",address);
			                Debug_TextOut(0, tempPrintString);
			                #endif
	if(result!=0x02)
	{
		//Release_Power_Down();
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_EWSR();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		delay_milliseconds(1);
		result = Flash_Read_Status_Register_1();
		if(result!=0x02)
		{
			delay_milliseconds(1);
			Flash_WREN();
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			result = Flash_Read_Status_Register_1();
		}
		
	}
	printf("\nBefore Erase-4 address is %x \n\n",address);
		if(result==0x02)
		{
			//Flash_Sector_Erase(address);
			Batch_Erase_buf[0] = Flash_64KByte_Block_Erase;
			//Batch_Erase_buf[0] = Flash_4KByte_Sector_Erase;/* send Sector Erase command */
			//Batch_Erase_buf[0] = Chip_Erase;
			Batch_Erase_buf[1] = (uint8_t) (address >> 16);		/* send 3 address bytes */
			Batch_Erase_buf[2] = (uint8_t) (address >> 8);
			Batch_Erase_buf[3] = (uint8_t) (address);
			Flash_SPI_Chip_Enable();       /* enable device */
			delay_milliseconds(2);
			SPI_transfer3(Batch_Erase_buf[0]);
			SPI_transfer3(Batch_Erase_buf[1]);
			SPI_transfer3(Batch_Erase_buf[2]);
			SPI_transfer3(Batch_Erase_buf[3]);

			delay_milliseconds(5);//2809
			Flash_SPI_Chip_Disable();       /* disable device */
			delay_milliseconds(10);
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			
			printf("\n  Erase Done-4\n",result);
			
		}	
		
		 asm volatile("gettime %0" : "=r" (currentTimer));
	   printf("\r\n\n\n Total time after erasing 4 blocks: %dms \n", (((currentTimer - startTimer) / 100) / 1000));
	
	//////////////////////// Erase 4 Blocks //////////////////////////////////////////////////////////////Erase End  ////////////////////////////////////////////////////
	
	
	//// Write Start ///////////////////////////////////////////////////
	
      result = Flash_Read_Status_Register_1();
		
	
      printf("\n Status Register1 is %d",result);
      
       result = Flash_Read_Status_Register_2();
		
	
      printf("\n Status Register2 is %d",result);
      
     // result = (result & ~(0x02));
      Flash_WREN1();
     //printf("\n Before waiting busy\n");
       wait_while_busy();
		Flash_EWSR();
       //printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		//printf("\n Before waiting busy\n");
         wait_while_busy();
		delay_milliseconds(1);
		if(result!=0x02)
		{
			delay_milliseconds(14);
			Flash_WREN();
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			result = Flash_Read_Status_Register_1();
		}
       Flash_WRSR2(0); //Disable Quad Mode
      //printf("\n Before waiting busy\n");
       wait_while_busy();
      
       result = Flash_Read_Status_Register_2();
		
	
       printf("\n Status Register2 is %d",result);
      
       result = Flash_Read_Status_Register_3();
		
	
       printf("\n Status Register3 is %d",result);
      
      
      
	//uint8_t  result=0;//,k=0;
	//uint16_t      read_offset=0,no_of_read_chunks,l=0;
	const uint8_t* data_ptr = (const uint8_t *) data_pool;
	//uint8_t  data;
	//uint32_t lflash_start_address;
	////uint16_t read_bytes_left=0,read_Chunk_size=0;
	//lflash_start_address= SF_FileAddress;//flash_start_address;
	uint8_t Byte_Program_buf[5];
	uint8_t x;
	char *ptr;

	
	////DelayMs(1);
	Flash_WREN();
	//printf("\n Before waiting busy\n");
    wait_while_busy();
	Flash_EWSR();
	//printf("\n Before waiting busy\n");
    wait_while_busy();
	Flash_WRSR(0x02);//enable write to status register
	Flash_WREN();
	//printf("\n Before waiting busy\n");
    wait_while_busy();
	result = Flash_WREN_Check();//261
	

	result=Flash_Read_Status_Register_1();
	//Debug_TextOut(0,"T2");
	printf("\nStatus Register-1  is %d  should be 2 Before Write Command Check 1",result);
	#ifdef PRODUCTION
	sprintf(tempPrintString,"\nStatus Register-1  is %d  should be 2 Before Write Command Check 1",result);
	Debug_TextOut(0, tempPrintString);
	#endif
	if(result!=0x02)
	{
		Release_Power_Down();
		Flash_WREN();
		////printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_EWSR();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		delay_milliseconds(1);
		result = Flash_Read_Status_Register_1();
		if(result!=0x02)
		{
			delay_milliseconds(14);
			Flash_WREN();
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			result = Flash_Read_Status_Register_1();
		}
		
	}
	if(result!=0x02)
	{
		Flash_WREN();
		Flash_EWSR();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		delay_milliseconds(1);
		result = Flash_Read_Status_Register_1();
		if(result!=0x02)
		{
			delay_milliseconds(14);
			Flash_WREN1();
			result = Flash_Read_Status_Register_1();
		}
		
	}
	//Debug_TextOut(0,"T3");
	result=Flash_Read_Status_Register_1();
	printf("\nStatus Register-1  is %d  should be 2 Before Write Command Check 2",result);
	#ifdef PRODUCTION
	sprintf(tempPrintString,"\nStatus Register-1  is %d  should be 2 Before Write Command Check 2",result);
	
	Debug_TextOut(0, tempPrintString);
	sprintf(tempPrintString, "\nWriting to shared flash\n");
			                Debug_TextOut(0, tempPrintString);
			                #endif
			                
  
	if(result==0x02)
	//if(result&(0x02))
	{
		#ifdef PRODUCTION
	    sprintf(tempPrintString, "\nBefore for loop of write ");
	
		Debug_TextOut(0, tempPrintString);
		#endif
		//lflash_start_address = 0;
		printf("\nBefore for loop of write address is %x \n",lflash_start_address);
		asm volatile("gettime %0" : "=r" (startTimer));	
		
		int k =0;
		int l = 0;
	    ////   Write part Start //////////
		//lflash_start_address = 0;					
		Flash_SPI_Chip_Enable();
		Byte_Program_buf[0] = Byte_Program;/* send Sector Erase command */
		Byte_Program_buf[1] = (uint8_t) (lflash_start_address >> 16);/* send 3 address bytes */
		Byte_Program_buf[2] = (uint8_t) (lflash_start_address >> 8);
		Byte_Program_buf[3] = (uint8_t) (lflash_start_address);
		//Byte_Program_buf[4] = byte;
	
		//Flash_SPI_TX(Byte_Program_buf,5);
		SPI_transfer3(Byte_Program_buf[0]);
		SPI_transfer3(Byte_Program_buf[1]);
		SPI_transfer3(Byte_Program_buf[2]);
		SPI_transfer3(Byte_Program_buf[3]);
		
		//// Address write done  ////
		
		//// Writing File present///////////////////////////////////////
		
		printf("\n File Name is %s\n",SF_File_Name);
		//printf("\n File Name is\n");
		//const uint8_t* data_ptr = (const uint8_t *) sf_filename1;
		memcpy(sf_filename1,SF_File_Name,55);
		printf("\n File Name is %s\n",sf_filename1);
		//const uint8_t* data_ptr = (const uint8_t *) sf_filename1;
		uint8_t bytes[4];
	//	printf("\n File Name while writing to flash\n",sf_filename1);
		
		printf("\nWriting to Flash\n\n");
		//char pr[] = "1";
		uint8_t pr[4] = {'Y','N','\\','0'};
		SPI_transfer3(pr[0]);
		printf("%c",pr[0]);
		////Writing File info i.e. file name, file address, file size
		//sprintf(tempPrintString, "\nFile name is %s ",SF_File_Name);
	
		//Debug_TextOut(0, tempPrintString);
		
		
		
		//for(i=0; i<sizeof(sf_filename1); i++)
		for(i=0; i<45; i++)
		{SPI_transfer3(sf_filename1[i]);printf("%c",sf_filename1[i]);};
		
		///// NULL///// After file name ////////////////////////////////
		SPI_transfer3(0);
		//SPI_transfer3(pr[3]);
		       
        // File Size 16 bit as byte by byte ////////////////////////////
        int ffsize = 86964575;
        ptr = (char *)(&ffsize);
        for(i=0;i< 4; i++)
        {
			printf("\r\nSF: i: %lu Data: %lu",i, *ptr);
			SPI_transfer3(*ptr); ptr++;
		}
        //int end =1;
        //ptr = (char *)(&end);
        //if(*ptr == 1)
        
        
       // char length[8];
        //sprintf(length, itoa(ffsize));
       // itoa(ffsize, length, 10);
       //unsafe{
        //return_string(ffsize, length);}
      //  printf("\n Length is %s\n", length
      
       // sprintf(length,"%d",ffsize);
      //  printf("\n Length in str %s\n",length);
        //for(i=0;i<sizeof(length);i++)
       //{ 
        //printf("%c",length[i]); SPI_transfer3(length[i]);};
       // printf("\n");
   	   // So far 53 byte written Next image data for rest of the sector.
		//for(i=0;i< 3935; i++)
		//{SPI_transfer3(*image);image++;};
		const uint8_t* data_ptr1 = (const uint8_t *) data_pool;
		//Debug_TextOut(0,"\nImage File\n");
		for(i=0;i< 200; i++)
		{printf("%c",data_ptr1[i]);SPI_transfer3(data_ptr1[i]);};
		//{printf("%c",*image);SPI_transfer3(*image);image++;};
		//Debug_Output1(0,"%c",*image);
		Flash_SPI_Chip_Disable();
		delay_milliseconds(1);
		// Writing rest of the image to next sectors
		fsize = fsize - 256;
		
		lflash_start_address = lflash_start_address + 256;
		
		/////// For Test Data //////////////////////////////////////////
		
		//Flash_SPI_Chip_Enable();
		//Byte_Program_buf[0] = Byte_Program;
		//Byte_Program_buf[1] = (uint8_t) (lflash_start_address >> 16);/* send 3 address bytes */
		//Byte_Program_buf[2] = (uint8_t) (lflash_start_address >> 8);
		//Byte_Program_buf[3] = (uint8_t) (lflash_start_address);
		
		//SPI_transfer3(Byte_Program_buf[0]);
		//SPI_transfer3(Byte_Program_buf[1]);
		//SPI_transfer3(Byte_Program_buf[2]);
		//SPI_transfer3(Byte_Program_buf[3]);
		
		//for(i=0;i<256;i++)
			//{SPI_transfer3(*image);image++;}
			
	
		//Flash_SPI_Chip_Disable();
	 ///////////////////////////////////////////////////////////////////
		#ifdef PRODUCTION
	sprintf(tempPrintString, "\nDone writing meta data writing rest of the image\n");
			                Debug_TextOut(0, tempPrintString);
			                #endif
		
		//// For Actual image ////////////
		int wflag = 1;
		//while(wflag){
				
					//Flash_SPI_Chip_Enable();
					//Byte_Program_buf[0] = Byte_Program;
					//Byte_Program_buf[1] = (uint8_t) (lflash_start_address >> 16);/* send 3 address bytes */
					//Byte_Program_buf[2] = (uint8_t) (lflash_start_address >> 8);
					//Byte_Program_buf[3] = (uint8_t) (lflash_start_address);
					
					//SPI_transfer3(Byte_Program_buf[0]);
					//SPI_transfer3(Byte_Program_buf[1]);
					//SPI_transfer3(Byte_Program_buf[2]);
					//SPI_transfer3(Byte_Program_buf[3]);
					//if(fsize<256)
					//for(i=0;i<fsize;i++)
						//{ // {SPI_transfer3(*image);image++;lflash_start_address++;}//Debug_Output1(0,"%c",*image);}
							//{printf("%c",data_ptr1[i]);SPI_transfer3(data_ptr1[i]);};
							//wflag = 0;
						//}
					//else
					//for(i=0;i<256;i++)
					//{printf("%c",data_ptr1[i]);SPI_transfer3(data_ptr1[i]);};
						////SPI_transfer3(*image);image++;lflash_start_address++;}//Debug_Output1(0,"%c",*image);}
					//Flash_SPI_Chip_Disable();
					//delay_ticks(20);
					//fsize = fsize - 256;
					//if(fsize == 0)wflag = 0;					
					
					 //wait_while_busy();
					 //delay_ticks(20);								
					 //Flash_WREN();
				//};
		

     }
	                   
	   asm volatile("gettime %0" : "=r" (currentTimer));
	 //  printf("\r\n\n\nTotal time after writing 1000 bytes: %dms.", (((currentTimer - startTimer) / 100) / 1000));
	             
		printf("\nwrite Done \n",result);
	//	while(1);
		#ifdef PRODUCTION
		sprintf(tempPrintString, "\nWrite Done \n\n");
			                Debug_TextOut(0, tempPrintString);
	    #endif
		

	////// Write End  ////////////////////////////////////////////////////
	
	
	
	//////Read Back Verification disabled
			
		//lflash_start_address= SF_FileAddress;
		//lflash_start_address= 0;
		lflash_start_address = lflash_start_read_address;
		printf("\nBefore for loop of read address is %x \n",lflash_start_address);
		//uint32_t lflash_start_address = 0;
		uint16_t SPI_RX_count=0;
		uint8_t Read_Cont_buf[5];
		uint8_t Read_data[100];
	
		Read_Cont_buf[0] = Read_Memory;
		Read_Cont_buf[1] = (uint8_t) (lflash_start_address >> 16);
		Read_Cont_buf[2] = (uint8_t) (lflash_start_address >> 8);
		Read_Cont_buf[3] = (uint8_t) (lflash_start_address);
		Read_Cont_buf[4] = 0x00;
		Flash_SPI_Chip_Enable();
		
		
		SPI_transfer1(Read_Cont_buf[0]);
		SPI_transfer1(Read_Cont_buf[1]);
		SPI_transfer1(Read_Cont_buf[2]);
		SPI_transfer1(Read_Cont_buf[3]);
		//SPI_transfer1(Read_Cont_buf[4]);

	printf("\nRead Data is \n\n");
     
	  for (SPI_RX_count = 0; SPI_RX_count < 100; SPI_RX_count++)
		{
			
				Read_data[SPI_RX_count] = SPI_transfer1(0);
				printf("%c",Read_data[SPI_RX_count]);
				
		}
		
		
		Flash_SPI_Chip_Disable();
		
		#ifdef PRODUCTION
	            sprintf(tempPrintString, "%s",Read_data);
				Debug_TextOut(0, tempPrintString);
				#endif
		
		  //for (SPI_RX_count = 0; SPI_RX_count < 256; SPI_RX_count++)
		//{
			
				////Read_data[SPI_RX_count] = SPI_transfer1(0);
				//printf("%c",Read_data[SPI_RX_count]);

		//}
		

	

	
	
	printf("\n  read Done\n ",result);
	
	
	
	return 1;
}

#endif

#if 0


int shared_flash_write()
//int shared_flash_write(char *buf, int size)
{
	uint32_t i, j;
	uint8_t  val;
	char *image;
	//image  = buf;
	unsigned startTimer = 0;
	unsigned startTimer1 = 0;
	unsigned currentTimer = 0;
	
	///////////////////  Testing  Signals  //////////////////////////////////
	
	
    //Flash_WREN1(); // Write Enable                         06
	//Flash_EWSR();  // Enable to write Register             50
	//Flash_WRSR(0x02);// write to status register           01, 02
	////delay_milliseconds(1);
	////delay_microseconds(1);
	//Flash_WREN1(); // Write Enable                         06
	////delay_microseconds(1);
	////delay_milliseconds(1);
	//val = Flash_Read_Status_Register_1();  // Read status Register  05
	//printf("\n Value of Status Register_1 is  %d\n",val);
	/////////////////////////////END///////////////////////////////////////
	
	
	
	////// Device ID Start   /////////////////////////////////////////////
	uint8_t  flash_id_cmd[6]= {0};
	uint8_t  flash_id[8] = {0};
	flash_id_cmd[0] = 0x4B;
	flash_id_cmd[1] = 0x00;
	flash_id_cmd[2] = 0x00;
	flash_id_cmd[3] = 0x00;
	flash_id_cmd[4] = 0x00;
	flash_id_cmd[5] = 0x00;
	
	
	 cs <: 0;
	for(i=0;i<6;i++)
	flash_id[i] = SPI_transfer1(flash_id_cmd[i]);	
	//cs <: 1;
	
	
	// delay_milliseconds(1);
	
	
	for(i=0;i<8;i++)
	flash_id[i] = SPI_transfer1(0);
	
	cs <: 1;
	
		
	for(i=0;i<8;i++)
	{//val = inverse(flash_id[i]);
		val = flash_id[i];
	printf("\n Device ID read is %d ",val);
	#ifdef PRODUCTION
	sprintf(tempPrintString, "\nDevice ID == %d\n", val);
			                Debug_TextOut(0, tempPrintString);
			                #endif
			                
			                }
	//while(1);
	
	//// Device ID  End/////////////////////////////////////////////////
	
	
	//// Erase Start ///////////////////////////////////////////////////
	// Erase 4 KB 
	
	    
	uint8_t  result=0;
	//uint32_t address=2000;
	//uint8_t Batch_Erase_buf[4];
	
	//////////////// Unlock Sector /////
	
	
	    Flash_WREN();
	   //printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_EWSR();
		printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		delay_milliseconds(1);
		result = Flash_Read_Status_Register_1();
		if(result!=0x02)
		{
			delay_milliseconds(14);
			Flash_WREN();
			printf("\n Before waiting busy\n");
            wait_while_busy();
			result = Flash_Read_Status_Register_1();
		}
		
	
	//uint8_t  result=0;
	uint32_t address=0;
	uint8_t Batch_Erase_buf[4];

   ////// Release_Power_Down();
	
	result = Flash_Read_Status_Register_1();
	printf("\n Status Register-1  is %d  should be 2",result);
	if(result!=0x02)
	{
		//Release_Power_Down();
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_EWSR();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		delay_milliseconds(1);
		result = Flash_Read_Status_Register_1();
		if(result!=0x02)
		{
			delay_milliseconds(14);
			Flash_WREN();
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			result = Flash_Read_Status_Register_1();
		}
		
	}
      printf("\n Status Register-1  is %d  should be 2 Before Erase Command",result);
      #ifdef PRODUCTION
     Debug_TextOut(0,"\n	Shared flash erase function enter \n");
      sprintf(tempPrintString," \n Status Register-1  is %d  should be 2 Before Erase Command",result);
	Debug_TextOut(0, tempPrintString);
    sprintf(tempPrintString, "\nErasing chip =========>>>EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n");
			                Debug_TextOut(0, tempPrintString);
			                #endif
	
		if(result==0x02)
		{
			//Flash_Sector_Erase(address);
			Batch_Erase_buf[0] = Flash_64KByte_Block_Erase;
			//Batch_Erase_buf[0] = Flash_4KByte_Sector_Erase;/* send Sector Erase command */
			//Batch_Erase_buf[0] = Chip_Erase;
			Batch_Erase_buf[1] = (uint8_t) (address >> 16);		/* send 3 address bytes */
			Batch_Erase_buf[2] = (uint8_t) (address >> 8);
			Batch_Erase_buf[3] = (uint8_t) (address);
			Flash_SPI_Chip_Enable();       /* enable device */
			delay_milliseconds(2);
			SPI_transfer1(Batch_Erase_buf[0]);
			SPI_transfer1(Batch_Erase_buf[1]);
			SPI_transfer1(Batch_Erase_buf[2]);
			SPI_transfer1(Batch_Erase_buf[3]);

			delay_milliseconds(50);//2809
			Flash_SPI_Chip_Disable();       /* disable device */
			delay_milliseconds(10);
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			
			printf("\n  Erase Done\n",result);
			
		}	
	
	
	//// Erase End  ////////////////////////////////////////////////////
	
	
	//// Write Start ///////////////////////////////////////////////////
	
      result = Flash_Read_Status_Register_1();
		
	
      printf("\n Status Register1 is %d",result);
      
       result = Flash_Read_Status_Register_2();
		
	
      printf("\n Status Register2 is %d",result);
      
     // result = (result & ~(0x02));
      Flash_WREN1();
     //printf("\n Before waiting busy\n");
       wait_while_busy();
		Flash_EWSR();
       //printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		//printf("\n Before waiting busy\n");
         wait_while_busy();
		delay_milliseconds(1);
		if(result!=0x02)
		{
			delay_milliseconds(14);
			Flash_WREN();
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			result = Flash_Read_Status_Register_1();
		}
       Flash_WRSR2(0); //Disable Quad Mode
      //printf("\n Before waiting busy\n");
       wait_while_busy();
      
       result = Flash_Read_Status_Register_2();
		
	
       printf("\n Status Register2 is %d",result);
      
       result = Flash_Read_Status_Register_3();
		
	
       printf("\n Status Register3 is %d",result);
      
      
      
	//uint8_t  result=0;//,k=0;
	//uint16_t      read_offset=0,no_of_read_chunks,l=0;
	const uint8_t* data_ptr = (const uint8_t *) data_pool;
	//uint8_t  data;
	uint32_t lflash_start_address;
	//uint16_t read_bytes_left=0,read_Chunk_size=0;
	lflash_start_address=  0;//flash_start_address;
	uint8_t Byte_Program_buf[5];

	
	////DelayMs(1);
	Flash_WREN();
	//printf("\n Before waiting busy\n");
    wait_while_busy();
	Flash_EWSR();
	//printf("\n Before waiting busy\n");
    wait_while_busy();
	Flash_WRSR(0x02);//enable write to status register
	Flash_WREN();
	//printf("\n Before waiting busy\n");
    wait_while_busy();
	result = Flash_WREN_Check();//261
	

	result=Flash_Read_Status_Register_1();
	//Debug_TextOut(0,"T2");
	printf("\n Status Register-1  is %d  should be 2 Before Write Command Check 1",result);
	#ifdef PRODUCTION
	sprintf(tempPrintString," \n Status Register-1  is %d  should be 2 Before Write Command Check 1",result);
	Debug_TextOut(0, tempPrintString);
	#endif
	if(result!=0x02)
	{
		Release_Power_Down();
		Flash_WREN();
		////printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_EWSR();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		//printf("\n Before waiting busy\n");
        wait_while_busy();
		delay_milliseconds(1);
		result = Flash_Read_Status_Register_1();
		if(result!=0x02)
		{
			delay_milliseconds(14);
			Flash_WREN();
			//printf("\n Before waiting busy\n");
            wait_while_busy();
			result = Flash_Read_Status_Register_1();
		}
		
	}
	if(result!=0x02)
	{
		Flash_WREN();
		Flash_EWSR();
		Flash_WRSR(0x02);//enable write to status register
		Flash_WREN();
		delay_milliseconds(1);
		result = Flash_Read_Status_Register_1();
		if(result!=0x02)
		{
			delay_milliseconds(14);
			Flash_WREN1();
			result = Flash_Read_Status_Register_1();
		}
		
	}
	//Debug_TextOut(0,"T3");
	result=Flash_Read_Status_Register_1();
	printf("\n Status Register-1  is %d  should be 2 Before Write Command Check 2",result);
	#ifdef PRODUCTION
	sprintf(tempPrintString," \n Status Register-1  is %d  should be 2 Before Write Command Check 2",result);
	
	Debug_TextOut(0, tempPrintString);
	sprintf(tempPrintString, "\nWriting to shared flash =======WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwww>\n");
			                Debug_TextOut(0, tempPrintString);
			                #endif
			                
  
	if(result==0x02)
	//if(result&(0x02))
	{
		#ifdef PRODUCTION
	    sprintf(tempPrintString, "\nBefore for loop of write ");
	
		Debug_TextOut(0, tempPrintString);
		#endif
		printf("\nBefore for loop of write ");
		asm volatile("gettime %0" : "=r" (startTimer));	
		asm volatile("gettime %0" : "=r" (startTimer1));	
		int k =0;
		int l = 0;
	    for(j=0;j<215;j++){
		
							//k++;
							//if(k== 14){
							            //l++;
										//asm volatile("gettime %0" : "=r" (currentTimer));
										//printf("\r\n After writing %d bytes: %dms.", (l*1000),(((currentTimer - startTimer) / 100) / 1000));
										//k = 0;
									 	////asm volatile("gettime %0" : "=r" (startTimer));	
				                     //}
								
							//for ( i=0;  i < 250;  ++i)		
							//{
								 
							/////////Page Program /////////////////////////////////////////
								//Flash_Byte_Program(lflash_start_address++, data_ptr[i]);
								
								
								Flash_SPI_Chip_Enable();
								Byte_Program_buf[0] = Byte_Program;/* send Sector Erase command */
								Byte_Program_buf[1] = (uint8_t) (lflash_start_address >> 16);/* send 3 address bytes */
								Byte_Program_buf[2] = (uint8_t) (lflash_start_address >> 8);
								Byte_Program_buf[3] = (uint8_t) (lflash_start_address);
								//Byte_Program_buf[4] = byte;
							
								//Flash_SPI_TX(Byte_Program_buf,5);
								SPI_transfer3(Byte_Program_buf[0]);
								SPI_transfer3(Byte_Program_buf[1]);
								SPI_transfer3(Byte_Program_buf[2]);
								SPI_transfer3(Byte_Program_buf[3]);
								
								for ( i=0;  i < 256;  ++i)		
							{
								
								SPI_transfer3(data_ptr[i]);
								//lflash_start_address++;
								
							}
								
								Flash_SPI_Chip_Disable();
								for ( i=0;  i < 256;  ++i)	lflash_start_address++;
							
								//printf("\n Before waiting busy\n");
                                wait_while_busy();
								
								
								//Flash_Byte_Program(lflash_start_address++, *image);
								//image++;
								
								
								Flash_WREN();
                                //Flash_WREN();
								
							//}
		                    //asm volatile("gettime %0" : "=r" (currentTimer));
							//printf("\r\n After writing 250 bytes: %dms.", (((currentTimer - startTimer) / 100) / 1000));
							//Flash_WREN();
	                   }
	                   
	                   asm volatile("gettime %0" : "=r" (currentTimer));
					   printf("\r\n\n\n Total time after writing 1000 bytes: %dms.", (((currentTimer - startTimer1) / 100) / 1000));
										
									//	while(1);
										
	             
		printf("\n  write Done \n",result);
	//	while(1);
		#ifdef PRODUCTION
		sprintf(tempPrintString, "\nWrite Done \n\n");
			                Debug_TextOut(0, tempPrintString);
	    #endif
		

			//////Read Back Verification disabled
			uint32_t lflash_start_address = 0;
			uint16_t SPI_RX_count=0;
			uint8_t Read_Cont_buf[5];
			uint8_t Read_data[256];
		
			Read_Cont_buf[0] = Read_Memory;
			Read_Cont_buf[1] = (uint8_t) (lflash_start_address >> 16);
			Read_Cont_buf[2] = (uint8_t) (lflash_start_address >> 8);
			Read_Cont_buf[3] = (uint8_t) (lflash_start_address);
			Read_Cont_buf[4] = 0x00;
			Flash_SPI_Chip_Enable();
			
			
			SPI_transfer1(Read_Cont_buf[0]);
			SPI_transfer1(Read_Cont_buf[1]);
			SPI_transfer1(Read_Cont_buf[2]);
			SPI_transfer1(Read_Cont_buf[3]);
			//SPI_transfer1(Read_Cont_buf[4]);

      printf("\n Data Read Back\n\n");
	
     for(j=0;j<215;j++){
		 
		  printf("\n");
		 
	  for (SPI_RX_count = 0; SPI_RX_count < 256; SPI_RX_count++)
		{
			
				Read_data[SPI_RX_count] = SPI_transfer1(0);
				printf("%c",Read_data[SPI_RX_count]);
				#ifdef PRODUCTION
	            sprintf(tempPrintString, "\nData Read back DDDDDDDDD=========>>>>> %c\n",Read_data[SPI_RX_count]);
				Debug_TextOut(0, tempPrintString);
				#endif
		}
		
	}
	Flash_SPI_Chip_Disable();

	}
	
	printf("\n  read Done\n ",result);
	////// Write End  ////////////////////////////////////////////////////
	
	
	
	return 1;
}

#endif
