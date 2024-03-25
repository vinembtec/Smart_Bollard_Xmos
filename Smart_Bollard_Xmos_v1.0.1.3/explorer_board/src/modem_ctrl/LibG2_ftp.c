
/*** Include Files ***/

#include "LibG2_ftp.h"
#include "rtos_osal.h"
//#include "../Camera_Main/host_mcu/host_mcu_types.h"


/*** Macro Definitions ***/

#define COMMAND_MODE 1


/*** Function Prototypes ***/

extern void cleanup_flash();

/*** Variable Declarations ***/
uint8_t bootUpFlag = 1;
uint8_t bootUpCount = 0;

int iEnableBrighterDataImage;
int iSendFtpStatus = 1;
//extern uint8_t mdm_comm_status;
uint8_t                			mdm_comm_status = 0;

extern uint8_t Debug_Verbose_Level;
extern uint8_t glMdmUart_recv_buf[GPRS_GEN_BUF_SZ_VLARGE];
extern uint8_t rx_ans[GPRS_GEN_BUF_SZ_MED];
extern uint8_t cmd_send[GPRS_GEN_BUF_SZ_VSMALL];

extern uint16_t glMdmUart_bytes_recvd;
//$//extern Modem sys_mdm_p;
extern uint8_t MDM_AWAKE;
//$//extern GPRSSystem glSystem_cfg;
extern uint8_t Last_Comms_Success_Flg;
extern uint8_t glClose_socket;
extern uint8_t Modem_Off_initiated;
extern uint8_t WAITING_FOR_SMS_RESPONSE;
extern bool LCD_Top_Message_String;
extern uint8_t transparentModeErrorCode;
extern uint8_t Meter_Reset;
uint32_t Ftp_checkModemStatus_timestamp = 0;
extern volatile bool LE910C4_CN_Flag;
extern uint8_t Modem_No_Response_Count;
bool Modem_Power_ON_Sequence_Flag = 0;
extern uint8_t  glComm_failure_count;
extern bool Modem_Recovery_Flag;
extern uint8_t DO_IP_SYNC;
extern uint8_t currentFileHeaderInfoTile0[205];

extern uart_tx_t* uart;
TaskHandle_t ftp_handler_task = NULL;

///// SF ///////////////////////////////////////////////////////////////
//#define SF
//extern int shared_flash_write(char * buf, int size);
//extern int shared_flash_write();
//extern int sf_write_test();
uint8_t *FTPImagFileBufferptr_sf;
extern int sf_write(uint8_t* FTPImagFileBufferptr, uint8_t* SF_Filename, uint32_t uImageSize, uint8_t capture_sequence_number,uint8_t capture_number);
extern uint8_t update_capture_seq_num();
//extern uint8_t get_seq_num();
extern uint8_t capture_sequence_number;
uint8_t capture_number;
char SF_Filename[200];
///// For Measuring Second Time XMOS ON //////////////////////////////
unsigned startTimer_S = 0;
unsigned currentTimer_S = 0;
int num_images = 0;
////////////////////////////////////////////////////////////////////////

#if ON_TILE(0)
extern uint8_t Modem_TX_Busy;
/*typedef enum {
    MODEM_TXD = 0
} txdUARTSelection;*/
//extern void UART_transmitData_TILE_0(txdUARTSelection n,unsigned char c);
#endif
extern FRESULT scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
);

//extern void run_test(void);	
//extern soc_peripheral_t modem_dev;
/*
#if( configAPPLICATION_ALLOCATED_HEAP == 1 )
#if XCOREAI_EXPLORER
//__attribute__((section(".ExtMem_data")))
#endif

#endif
*/
/*** Function Definitions ***/
///////////////////////////////////////////////////////////////////////////////////////////
#if ON_TILE(0)
#define EQUAL               0
#define ASCENDING           0
#define DESCENDING          1

#define MAX_SIZE            200
#define MAX_BATCHED_FILES   20

#define NP_SIZE_FTP_IMAGE 2
#define BOTH_FULL_SIZE_NP_FTP_IMAGE 3

uint32_t iBatchedImagesToSend = 0xFFFFFFFF;		//added on 06_08_2021
//extern char FTPBaseFilename[200];
//extern char previousFileNameBase[200];

uint16_t Number_of_NP_BatchedImages=0;
uint16_t Number_of_FL_BatchedImages=0;


char FTP_FL_Filename[MAX_BATCHED_FILES][MAX_SIZE];
char FTP_NP_Filename[MAX_BATCHED_FILES][MAX_SIZE];


bool fileTransferInProgress = false;
bool sendSomLogFile = false;
int iFTPFileSendOption = BOTH_FULL_SIZE_NP_FTP_IMAGE;

//uint16_t uTicksDelay_while_Heap_memory_allocation = 100; //PS:23032023 added uTicksDelay_while_Heap_memory_allocation variable for changing delay at 4 locations

//void return_string(int x, char *ptr);

//extern ImageInfo_t imageCaptureTile0;
/*
void return_string(int x, char *ptr)
{
	itoa(x,ptr,10);
}*/

/* Send dummy 1500 bytes */
int	UsbModem_sendDummyData(void)
{
    uint8_t data[0] = {0xAA};

    //UsbModem_sendData(data, sizeof(data), 20);

	for(int kt =0;kt<=(sizeof(data));kt++)
	{
		////int state = rtos_osal_critical_enter();
		{
			rtos_uart_tx_write(uart_tx_ctx, data, 1);
			//UART_transmitData_TILE_0(0, data[kt]);
		}
		////rtos_osal_critical_exit(state);		
	}	
	
	Debug_TextOut(0,"Dummy Data 0xAA Sent..!");
    //qDebug("Dummy Data 0xAA Sent..!");

    /*if (1 == iSomLogEnable)
		log0(1, 0x87, "Dummy Data 0xAA Sent..! ");
	*/
	
    return 0;
}

#if 1
void RearrangeFTP_FL_NP_Filename(uint8_t instance)
{
    char tempFileName[200];
    int i;
    uint16_t txStringLength=0;
    uint16_t imageIndex = 0;

	if (0 == instance)
	{
		//qDebug("\r\n");
		/*if (1 == iSomLogEnable)
		{
			log0(1, 0x85, " ");
		}*/
		
		//Debug_Output1(0,"jpegNumberPlateSizeImageNumbers: 0x%X", imageCaptureTile0.vars.jpegNumberPlateSizeImageNumbers);
		//Debug_Output1(0,"jpegFullSizeImageNumbers: 0x%X", imageCaptureTile0.vars.jpegFullSizeImageNumbers);

		//Debug_Output1(0,"previousJpegNumberPlateSizeImageNumbers : 0x%X", imageCaptureTile0.vars.previousJpegNumberPlateSizeImageNumbers);
		//Debug_Output1(0,"previousJpegFullSizeImageNumbers : 0x%X", imageCaptureTile0.vars.previousJpegFullSizeImageNumbers);

		//Debug_Output1(0,"brighterDataImages: 0x%X", imageCaptureTile0.vars.brighterDataImages);		//TBD	//added on 07_09_2021
		/*if (1 == iSomLogEnable)
		{
			printValueInHex = true;
			log1(1, 0x85, "jpegNumberPlateSizeImageNumbers: ",jpegNumberPlateSizeImageNumbers);
			log1(1, 0x85, "jpegFullSizeImageNumbers: ",jpegFullSizeImageNumbers);
			log1(1, 0x85, "previousJpegNumberPlateSizeImageNumbers: ",previousJpegNumberPlateSizeImageNumbers);
			log1(1, 0x85, "previousJpegFullSizeImageNumbers: ",previousJpegFullSizeImageNumbers);
			log1(1, 0x85, "brighterDataImages: ",brighterDataImages );
			printValueInHex = false;
		}*/
	}
	
    /* Number Plate Images */

    uint16_t index = 0;
    uint32_t numPlateImgNumbers = 0;

    for (i = 0, imageIndex = 0; i <= (imageCaptureTile0.vars.totalCapturedNumImages + imageCaptureTile0.vars.previousTotalJpegImages); i++)
    {
        if (i < imageCaptureTile0.vars.totalCapturedNumImages)
        {
            index = i;
            numPlateImgNumbers = imageCaptureTile0.vars.jpegNumberPlateSizeImageNumbers;
			if ((1 == iEnableBrighterDataImage) && (imageCaptureTile0.vars.executeBrighterDataImageTask))	//TBD	//added on 07_09_2021
			{
				imageCaptureTile0.vars.brighterDataImages = imageCaptureTile0.vars.currentBrighterDataImages;
			}
		}
        else
        {
            index = i - imageCaptureTile0.vars.totalCapturedNumImages;
            numPlateImgNumbers = imageCaptureTile0.vars.previousJpegNumberPlateSizeImageNumbers;
			if ((1 == iEnableBrighterDataImage) && (imageCaptureTile0.vars.executeBrighterDataImageTask))	//TBD	//added on 07_09_2021
			{
				imageCaptureTile0.vars.brighterDataImages = imageCaptureTile0.vars.previousBrighterDataImages;
			}
        }
		//Debug_Output1(0,"brighterDataImages: 0x%X", imageCaptureTile0.vars.brighterDataImages);
		//qDebug("brighterDataImages: 0x%X", imageCaptureTile0.vars.brighterDataImages);		//TBD	//added on 07_09_2021
        
		//if (((iImageFilesTobeTransferred >> index) & 0x1) && (numPlateImgNumbers & (1 << index)))
		if (((imageCaptureTile0.vars.brighterDataImages >> index) & 0x1) && (numPlateImgNumbers & (1 << index)))	//TBD	//added on 07_09_2021
        {
            txStringLength = 0;
            //delayInMSec(50);

            memcpy(&tempFileName[txStringLength], "IMG_NP_", strlen("IMG_NP_"));
            txStringLength += strlen("IMG_NP_");

            if (i < imageCaptureTile0.vars.totalCapturedNumImages)
            {
				memcpy(imageCaptureTile0.vars.currentFileNameBase, imageCaptureTile0.vars.currentFileNameBase, sizeof(imageCaptureTile0.vars.currentFileNameBase));				
				//Debug_Output1(0,"Send Current File Info : %s",imageCaptureTile0.vars.currentFileNameBase);
                //qDebug()<<"Send Current File Info "<<imageCaptureTile0.vars.currentFileNameBase;
                strncpy(&tempFileName[txStringLength], &imageCaptureTile0.vars.currentFileNameBase[4], 26);
            }
            else
            {
				//Debug_Output1(0,"Send Previous File Info : %s", imageCaptureTile0.vars.previousFileNameBase);
                //qDebug()<<"Send Previous File Info "<<imageCaptureTile0.vars.previousFileNameBase;
                strncpy(&tempFileName[txStringLength], &imageCaptureTile0.vars.previousFileNameBase[4], 26);
            }

            txStringLength += 26;

            tempFileName[txStringLength] = '_';
            txStringLength += strlen("_");

            if ((index >= 0) && (index <= 9))
            {
                tempFileName[txStringLength++] = '0'; /* File Packet Number */
                tempFileName[txStringLength++] = index + 0x30; /* File Packet Number */
            }
            else if ((index >= 10) && (index <= 99))
            {
                tempFileName[txStringLength++] = (index / 10) + 0x30; /* File Packet Number */
                tempFileName[txStringLength++] = (index % 10) + 0x30; /* File Packet Number */
            }

            strncpy(&tempFileName[txStringLength], ".jpeg", strlen(".jpeg"));
            txStringLength += strlen(".jpeg");

            tempFileName[txStringLength] = '\0';
            //txStringLength += 1;

			//Debug_Output1(0,"FTPFilename_with_NP : %s",tempFileName);
            //qDebug()<<"FTPFilename_with_NP: "<<tempFileName;

            for (int j = imageIndex; j < Number_of_NP_BatchedImages; j++)	//TBD	//Added on 28_04_2021
            {
                if (0 == strcmp(tempFileName, FTP_NP_Filename[j]))
                {
                    if (j > 0)
                    {
                        int k;

                        /* Push Batched Files to next locations */
                        for (k = 0; k < (j - imageIndex); k++)
                        {
                            strcpy(FTP_NP_Filename[j - k], FTP_NP_Filename[j - (k + 1)]);
							
							/*if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
							{
								imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
							}*/
							if (imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
							{
								return;
							}
                        }

                        /* Copy Current Image File Name */
                        strcpy(FTP_NP_Filename[imageIndex], tempFileName);
                    }
                    break;
                }
				/* Monitor Seconds sensor trigger */
				//if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (1 == cameraCount))	//TBD //commented on 26_09_2021
				/*if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
				{
					imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
				}*/
				if (imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
				{
					return;
				}
            }

            imageIndex++;
        }
		/* Monitor Seconds sensor trigger */
		//if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (1 == cameraCount))	//TBD //commented on 26_09_2021
		/*if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
		{
			imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
		}*/
		if (imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
		{
			return;
		}
    }


    /* Full Size Images */

    //qDebug("\r\n");

    index = 0;
    uint32_t fullSizeImgNumbers = 0;

    for (i = 0, imageIndex = 0; i <= (imageCaptureTile0.vars.totalCapturedNumImages + imageCaptureTile0.vars.previousTotalJpegImages); i++)
    {
        if (i < imageCaptureTile0.vars.totalCapturedNumImages)
        {
            index = i;
            fullSizeImgNumbers = imageCaptureTile0.vars.jpegFullSizeImageNumbers;
			if ((1 == iEnableBrighterDataImage) && (imageCaptureTile0.vars.executeBrighterDataImageTask))	//TBD	//added on 07_09_2021
			{
				imageCaptureTile0.vars.brighterDataImages = imageCaptureTile0.vars.currentBrighterDataImages;
			}
        }
        else
        {
            index = i - imageCaptureTile0.vars.totalCapturedNumImages;
            fullSizeImgNumbers = imageCaptureTile0.vars.previousJpegFullSizeImageNumbers;
			if ((1 == iEnableBrighterDataImage) && (imageCaptureTile0.vars.executeBrighterDataImageTask))	//TBD	//added on 07_09_2021
			{
				imageCaptureTile0.vars.brighterDataImages = imageCaptureTile0.vars.previousBrighterDataImages;
			}
        }

		//qDebug("brighterDataImages: 0x%X", brighterDataImages);		//TBD	//added on 07_09_2021
		
        //if (((iImageFilesTobeTransferred >> index) & 0x1) && (fullSizeImgNumbers & (1 << index)))	//commented on 21_06_2021
		if (((imageCaptureTile0.vars.brighterDataImages >> index) & 0x1) && (fullSizeImgNumbers & (1 << index)))		//TBD	//added on 07_09_2021
        {
            txStringLength = 0;
            //delayInMSec(50);

            memcpy(&tempFileName[txStringLength], "IMG_FL_", strlen("IMG_FL_"));
            txStringLength += strlen("IMG_FL_");

            if (i < imageCaptureTile0.vars.totalCapturedNumImages)
            {
				memcpy(imageCaptureTile0.vars.currentFileNameBase, imageCaptureTile0.vars.currentFileNameBase, sizeof(imageCaptureTile0.vars.currentFileNameBase));				
				//Debug_Output1(0,"Send Current File Info : %s",imageCaptureTile0.vars.currentFileNameBase);
                //qDebug()<<"Send Current File Info "<<imageCaptureTile0.vars.currentFileNameBase;
                strncpy(&tempFileName[txStringLength], &imageCaptureTile0.vars.currentFileNameBase[4], 26);
            }
            else
            {
				//Debug_Output1(0,"Send Previous File Info : %s",imageCaptureTile0.vars.previousFileNameBase);
                //qDebug()<<"Send Previous File Info "<<imageCaptureTile0.vars.previousFileNameBase;
                strncpy(&tempFileName[txStringLength], &imageCaptureTile0.vars.previousFileNameBase[4], 26);
            }
            txStringLength += 26;

            tempFileName[txStringLength] = '_';
            txStringLength += strlen("_");

            if ((index >= 0) && (index <= 9))
            {
                tempFileName[txStringLength++] = '0'; /* File Packet Number */
                tempFileName[txStringLength++] = index + 0x30; /* File Packet Number */
            }
            else if ((index >= 10) && (index <= 99))
            {
                tempFileName[txStringLength++] = (index / 10) + 0x30; /* File Packet Number */
                tempFileName[txStringLength++] = (index % 10) + 0x30; /* File Packet Number */
            }

            strncpy(&tempFileName[txStringLength], ".jpeg", strlen(".jpeg"));
            txStringLength += strlen(".jpeg");

            tempFileName[txStringLength] = '\0';
            //txStringLength += 1;

			//Debug_Output1(0,"FTPFilename_with_FL : %s",tempFileName);
            //qDebug()<<"FTPFilename_with_FL: "<<tempFileName;

            for (int j = imageIndex; j < Number_of_FL_BatchedImages; j++)	//TBD	//Added on 28_04_2021
            {
                if (0 == strcmp(tempFileName, FTP_FL_Filename[j]))
                {
                    if (j > 0)
                    {
                        int k;

                        /* Push Batched Files to next locations */
                        for (k = 0; k < (j - imageIndex); k++)
                        {
                            strcpy(FTP_FL_Filename[j - k], FTP_FL_Filename[j - (k + 1)]);
							
							/*if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
							{
								imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
							}*/
							if (imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
							{
								return;
							}
                        }

                        /* Copy Current Image File Name */
                        strcpy(FTP_FL_Filename[imageIndex], tempFileName);
                    }
                    break;
                }
				/* Monitor Seconds sensor trigger */
				//if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (1 == cameraCount))	//TBD //commented on 26_09_2021
				/*if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
				{
					imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
				}*/
				if (imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
				{
					return;
				}
            }

            imageIndex++;
        }
		
		/* Monitor Seconds sensor trigger */
		//if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (1 == cameraCount))	//TBD //commented on 26_09_2021
		/*if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
		{
			imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
		}*/
		if (imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
		{
			return;
		}
    }
}




// Will print out all the strings 2d array
void print_2d_array(char str[MAX_SIZE][MAX_SIZE], int len) {

    int i = 0;
    while (i < len) {
		Debug_Output1(0,"Sorted Filename: %s",str[i]);
        //qDebug()<<"Sorted Filename:"<<str[i];
        i++;
 		if (imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
		{
			return;
		}	
   }
}

// Swaps two strings (assuming memory allocation is already correct)
void swap_str(char str1[], char str2[]) {

    char temp[MAX_SIZE];

    strcpy(temp, str1);
    strcpy(str1, str2);
    strcpy(str2, temp);

}
// Will sort a 2D array in either descending or ascending order,
// depending on the flag you give it
void sort_strings(char str[MAX_SIZE][MAX_SIZE], int len, int order_type) {

    int i = 0;
	/*This section newly added to check start positon of year data from image file name*/
	int k = 0;
	int count = 0; //char  _ for getting correct position for time stamp
    int position = 0;
	
	while (str[i][k] != '\0')
	{
		if(str[i][k] == '_')
		{
			count++;
		}
		position++;
		k++;
		if(count >= 4) break;
	}
	
	while (i < len) {

        int j = 0;
        while (j < len) {

            if ((order_type == ASCENDING) &&
                (strcmp(str[i]+position, str[j]+position) < EQUAL)) {

                swap_str(str[i], str[j]);
            } else if ((order_type == DESCENDING) &&
                (strcmp(str[i]+position, str[j]+position) > EQUAL)) {

                swap_str(str[i], str[j]);
            }

            j++;

			/* Monitor Seconds sensor trigger */
			//if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (1 == cameraCount))	//TBD //commented on 26_09_2021
			/*if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
			{
				imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
			}*/
			if (imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
			{				
				return;
			}
        }

        i++;

		/* Monitor Seconds sensor trigger */
		//if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (1 == cameraCount))	//TBD //commented on 26_09_2021
		/*if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
		{
			imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
		}*/
		if (imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
		{				
			return;
		}
    }
}

int Sort_File(uint8_t instance) {

    int order_type;
    //char str[MAX_SIZE][MAX_SIZE];

    order_type = ASCENDING;
    if (order_type != ASCENDING && order_type != DESCENDING) {
        Debug_TextOut(0,"Please enter 0 or 1");
		//qDebug("Please enter 0 or 1\n");
        exit(1);
    }

    int i = 0;
   // while ((i < MAX_SIZE) && (scanf("%s", str[i]) == 1)) i++;
   // if (i == MAX_SIZE) printf("You reached the maximum strings allowed\n");

    // Program output of the sorted strings
	if (0 == instance)
	{
		//Debug_TextOut(0,"---------[Sorted Strings]---------");
		//qDebug("---------[Sorted Strings]---------");
		
		/*if (1 == iSomLogEnable)
		{
			log0(1, 0x85, "---------[Sorted Strings]---------");
		}*/
    }
	
	sort_strings(FTP_NP_Filename, Number_of_NP_BatchedImages, DESCENDING);
	/* Monitor Seconds sensor trigger */
	//if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (1 == cameraCount))	//TBD //commented on 26_09_2021
	/*if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
	{
		imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
	}*/
	if (imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
	{				
		return 0;
	}
    print_2d_array(FTP_NP_Filename, Number_of_NP_BatchedImages);

    sort_strings(FTP_FL_Filename, Number_of_FL_BatchedImages, DESCENDING);
	/* Monitor Seconds sensor trigger */
	//if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (1 == cameraCount))	//TBD //commented on 26_09_2021
	/*if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
	{
		imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
	}*/
	if (imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
	{				
		return 0;
	}
    print_2d_array(FTP_FL_Filename, Number_of_FL_BatchedImages);

    RearrangeFTP_FL_NP_Filename(instance);

    return 0;
}
#endif

void GetImageFilesInDirToArray_For_FTP_Transfer(uint8_t instance)
{
    uint16_t iRet;
    int i;
#if 1    
	Number_of_FL_BatchedImages = 0;
    Number_of_NP_BatchedImages = 0;
	
	for (i = 0; i < MAX_BATCHED_FILES; i++)
	{
		memset(&FTP_NP_Filename[i][0], 0, MAX_SIZE);
		memset(&FTP_FL_Filename[i][0], 0, MAX_SIZE);		
	}
#if 1		
    FRESULT fr;     /* Return value */
    DIR dj;         /* Directory object */
    FILINFO fno;    /* File information */
			//taskENTER_CRITICAL();
			//xSemaphoreTakeRecursive();
			rtos_printf("TILE = %d\n", 0);			
			fr = f_findfirst(&dj, &fno, "", "*.*"); /* Start to search for photo files */

			while (fr == FR_OK && fno.fname[0]) {         /* Repeat while an item is found */
				printf("TILE0_fno.fname = %s\n", fno.fname);                /* Print the object name */

				if ((strncmp(fno.fname,"IMG_NP", 6) == 0) && (Number_of_NP_BatchedImages < MAX_BATCHED_FILES))	// || (strncmp(dir->d_name,"IMG_NP", 6) == 0) )
				{
					strcpy(&FTP_NP_Filename[Number_of_NP_BatchedImages][0],fno.fname);
					Number_of_NP_BatchedImages++;
					//qDebug()<<"File List"<<dir->d_name;//TBD//--to be commented
				}
				else if ((strncmp(fno.fname,"IMG_FL", 6) == 0) && (Number_of_FL_BatchedImages < MAX_BATCHED_FILES))
				{
					strcpy(&FTP_FL_Filename[Number_of_FL_BatchedImages][0],fno.fname);
					Number_of_FL_BatchedImages++;
				}
				
				if (Number_of_NP_BatchedImages >= MAX_BATCHED_FILES)
				{
					Debug_Output1(0,"NP Image File List Limit Exceeds.. %s",fno.fname);
					//qDebug()<<"NP Image File List Limit Exceeds.."<<dir->d_name;
					/*if (1 == iSomLogEnable)
					{
						log0(1, 0x87, "NP Image File List Limit Exceeds.." );
						log0(0, 0x87, dir->d_name );
					}*/
					break;
				}

				if (Number_of_FL_BatchedImages >= MAX_BATCHED_FILES)
				{
					Debug_Output1(0,"FL Image File List Limit Exceeds.. %s",fno.fname);
					//qDebug()<<"FL Image File List Limit Exceeds.."<<dir->d_name;
					/*if (1 == iSomLogEnable)
					{
						log0(1, 0x87, "FL Image File List Limit Exceeds.." );
						log0(0, 0x87, dir->d_name );
					}*/
					break;
				}
				
				fr = f_findnext(&dj, &fno);               /* Search for next item */
				
			}
			
			imageCaptureTile0.vars.totalCapturedNumImages = Number_of_NP_BatchedImages + Number_of_FL_BatchedImages;
			
			printf("TILE0_last fr = %d\n", fr); 
			printf("TILE0_last fno.fname = %s\n", fno.fname); 
			//xSemaphoreGiveRecursive() ;
			//taskEXIT_CRITICAL();
			f_closedir(&dj);	
			
			////printf("TILE0_last f_unmount ("") = %d\n", f_unmount (""));
			//f_unmount ("");
#endif
	////return ;
	//qDebug()<<"File List"<<d;
	//printValueInHex = true;    
	//log1(1, 0x85, "File List", d);	//TBD //decimal to hex 
	//printValueInHex = false;	
	
	/* Sort Files */
    Sort_File(instance);

	/* Monitor Seconds sensor trigger */
	//if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (1 == cameraCount))	//TBD //commented on 26_09_2021
	/*if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
	{
		imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
	}*/
	if (imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
	{
		return;
	}

    //qDebug("\r\n");
	
	if (0 == instance)
	{
		//Debug_Output1(0,"Current NP image Count: %d", imageCaptureTile0.vars.jpegNumberPlateSizeImageCount);
		//qDebug()<<"Current NP image Count:"<<imageCaptureTile0.vars.jpegNumberPlateSizeImageCount;
		/*if (1 == iSomLogEnable)
		{
			log0(1, 0x87, " ");
			log1(1, 0x87, "Current NP image Count:",imageCaptureTile0.vars.jpegNumberPlateSizeImageCount );
		}*/
		for (i = 0; i < imageCaptureTile0.vars.jpegNumberPlateSizeImageCount; i++)
		{
			Debug_Output2(0,"	NP Image(%d): %s", i, FTP_NP_Filename[i]);
			//qDebug("	NP Image(%d): %s", i, FTP_NP_Filename[i]);
			/*if (1 == iSomLogEnable)
			{
				//log1(1, 0x85, "	NP Image: ",i );
				//log0(0, 0x85, FTP_NP_Filename[i]);
				log1(1, 0x87, "	NP Image(",i);
				log0(0, 0x87, "): ");
				log0(0, 0x87, FTP_NP_Filename[i]);
			}*/

			/* Monitor Seconds sensor trigger */
			//if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (1 == cameraCount))	//TBD //commented on 26_09_2021
			/*if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
			{
				imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
			}*/
			if (imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
			{
				return;
			}
		}

		//Debug_Output1(0,"Current FL image Count: %d", imageCaptureTile0.vars.jpegFullSizeImageCount);
		//qDebug()<<"Current FL image Count:"<<jpegFullSizeImageCount;
		/*if (1 == iSomLogEnable)
			log1(1, 0x87, "Current FL image Count:", jpegFullSizeImageCount);
		*/
		for (i = 0; i < imageCaptureTile0.vars.jpegFullSizeImageCount; i++)
		{
			Debug_Output2(0,"	FL Image(%d): %s", i, FTP_FL_Filename[i]);
			//qDebug("	FL Image(%d): %s", i, FTP_FL_Filename[i]);
			/*if (1 == iSomLogEnable)
			{
				//log1(1, 0x85, "	FL Image: ",i );
				log1(1, 0x87, "	FL Image(",i);
				log0(0, 0x87, "): ");
				log0(0, 0x87, FTP_FL_Filename[i]);
			}*/

			/* Monitor Seconds sensor trigger */
			//if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (1 == cameraCount))	//TBD //commented on 26_09_2021
			/*if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
			{
				imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
			}*/
			if (imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
			{
				return;
			}
		}

		//qDebug("\r\n");
		//Debug_Output1(0,"Previous NP image Count: %d", imageCaptureTile0.vars.previousJpegNumberPlateSizeImageCount);		
		//qDebug()<<"Previous NP image Count:"<<imageCaptureTile0.vars.previousJpegNumberPlateSizeImageCount;
		/*if (1 == iSomLogEnable)
		{
			log0(1, 0x87, " ");
			log1(1, 0x87, "Previous NP image Count:", imageCaptureTile0.vars.previousJpegNumberPlateSizeImageCount);
		}*/
		for (i = imageCaptureTile0.vars.jpegNumberPlateSizeImageCount; i < (imageCaptureTile0.vars.jpegNumberPlateSizeImageCount + imageCaptureTile0.vars.previousJpegNumberPlateSizeImageCount); i++)
		{
			Debug_Output2(0,"	NP Image(%d): %s", i, FTP_NP_Filename[i]);
			//qDebug("	NP Image(%d): %s", i, FTP_NP_Filename[i]);
			/*if (1 == iSomLogEnable)
			{
				//log1(1, 0x85, "	NP Image: ",i );
				log1(1, 0x87, "	NP Image(",i);
				log0(0, 0x87, "): ");
				log0(0, 0x87, FTP_NP_Filename[i]);
			}*/

			/* Monitor Seconds sensor trigger */
			//if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (1 == cameraCount))	//TBD //commented on 26_09_2021
			/*if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
			{
				imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
			}*/
			if (imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
			{
				return;
			}
		}

		//Debug_Output1(0,"Previous FL image Count: %d", imageCaptureTile0.vars.previousJpegFullSizeImageCount);	
		//qDebug()<<"Previous FL image Count:"<<previousJpegFullSizeImageCount;
		/*if (1 == iSomLogEnable)
			log1(1, 0x87, "Previous FL image Count:", previousJpegFullSizeImageCount);
		*/
		for (i = imageCaptureTile0.vars.jpegFullSizeImageCount; i < (imageCaptureTile0.vars.jpegFullSizeImageCount + imageCaptureTile0.vars.previousJpegFullSizeImageCount); i++)
		{
			Debug_Output2(0,"	FL Image(%d): %s", i, FTP_FL_Filename[i]);
			//qDebug("	FL Image(%d): %s", i, FTP_FL_Filename[i]);
			/*if (1 == iSomLogEnable)
			{
				//log1(1, 0x85, "	FL Image: ",i );
				log1(1, 0x87, "	FL Image(",i);
				log0(0, 0x87, "): ");
				log0(0, 0x87, FTP_FL_Filename[i]);
			}*/

			/* Monitor Seconds sensor trigger */
			//if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (1 == cameraCount))	//TBD //commented on 26_09_2021
			/*if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
			{
				imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
			}*/
			if (imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
			{
				return;
			}
		}

		//qDebug("\r\n");	
		//Debug_Output1(0,"Batched NP image Count: %d",((Number_of_NP_BatchedImages > (imageCaptureTile0.vars.jpegNumberPlateSizeImageCount + imageCaptureTile0.vars.previousJpegNumberPlateSizeImageCount)) ? (Number_of_NP_BatchedImages - (imageCaptureTile0.vars.jpegNumberPlateSizeImageCount + imageCaptureTile0.vars.previousJpegNumberPlateSizeImageCount)) : 0));	
		//qDebug()<<"Batched NP image Count:"<<((Number_of_NP_BatchedImages > (imageCaptureTile0.vars.jpegNumberPlateSizeImageCount + imageCaptureTile0.vars.previousJpegNumberPlateSizeImageCount)) ? (Number_of_NP_BatchedImages - (imageCaptureTile0.vars.jpegNumberPlateSizeImageCount + imageCaptureTile0.vars.previousJpegNumberPlateSizeImageCount)) : 0);

		/*if (1 == iSomLogEnable)
		{
			log0(1, 0x87, " ");
			log1(1, 0x87, "Batched NP image Count:",
			 ((Number_of_NP_BatchedImages >
			   (imageCaptureTile0.vars.jpegNumberPlateSizeImageCount + imageCaptureTile0.vars.previousJpegNumberPlateSizeImageCount)) ?
				  (Number_of_NP_BatchedImages - (imageCaptureTile0.vars.jpegNumberPlateSizeImageCount +
												 imageCaptureTile0.vars.previousJpegNumberPlateSizeImageCount)) : 0));
		}*/
		
		for (i = (imageCaptureTile0.vars.jpegNumberPlateSizeImageCount + imageCaptureTile0.vars.previousJpegNumberPlateSizeImageCount); i < Number_of_NP_BatchedImages; i++)
		{
			//Debug_Output2(0,"	NP Image(%d): %s", i, FTP_NP_Filename[i]);
			//qDebug("	NP Image(%d): %s", i, FTP_NP_Filename[i]);
			/*if (1 == iSomLogEnable)
			{
				//log1(1, 0x85, "	NP Image: ",i );
				log1(1, 0x87, "	NP Image(",i);
				log0(0, 0x87, "): ");
				log0(0, 0x87, FTP_NP_Filename[i]);
			}*/

			/* Monitor Seconds sensor trigger */
			//if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (1 == cameraCount))	//TBD //commented on 26_09_2021
			/*if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
			{
				imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
			}*/
			if (imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
			{
				return;
			}
		}

		//Debug_Output1(0,"Batched FL image Count:: %d",((Number_of_FL_BatchedImages > (imageCaptureTile0.vars.jpegFullSizeImageCount + imageCaptureTile0.vars.previousJpegFullSizeImageCount)) ? (Number_of_FL_BatchedImages - (imageCaptureTile0.vars.jpegFullSizeImageCount + imageCaptureTile0.vars.previousJpegFullSizeImageCount)) : 0));	
		//qDebug()<<"Batched FL image Count:"<<((Number_of_FL_BatchedImages > (jpegFullSizeImageCount + previousJpegFullSizeImageCount)) ? (Number_of_FL_BatchedImages - (jpegFullSizeImageCount + previousJpegFullSizeImageCount)) : 0);
		/*if (1 == iSomLogEnable)
			log1(1, 0x87, "Batched FL image Count:",((Number_of_FL_BatchedImages > (jpegFullSizeImageCount + previousJpegFullSizeImageCount)) ? (Number_of_FL_BatchedImages - (jpegFullSizeImageCount + previousJpegFullSizeImageCount)) : 0));
		*/
		for (i = (imageCaptureTile0.vars.jpegFullSizeImageCount + imageCaptureTile0.vars.previousJpegFullSizeImageCount); i < Number_of_FL_BatchedImages; i++)
		{
			Debug_Output2(0,"	FL Image(%d): %s", i, FTP_FL_Filename[i]);
			//qDebug("	FL Image(%d): %s", i, FTP_FL_Filename[i]);
			/*if (1 == iSomLogEnable)
			{
				//log1(1, 0x85, "	FL Image: ",i );
				log1(1, 0x87, "	FL Image(",i);
				log0(0, 0x87, "): ");
				log0(0, 0x87, FTP_FL_Filename[i]);
			}*/
			
			/* Monitor Seconds sensor trigger */
			//if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (1 == cameraCount))	//TBD //commented on 26_09_2021
			/*if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
			{
				imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
			}*/
			if (imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
			{
				return;
			}
		}
	}
	//TBD //added on 22_12_2021
	else if (1 == instance)
	{
		//--------------------------------------------
		imageCaptureTile0.vars.jpegFullSizeImageCount = 0;
		imageCaptureTile0.vars.jpegNumberPlateSizeImageCount = 0;
		imageCaptureTile0.vars.previousJpegFullSizeImageCount = 0;
		imageCaptureTile0.vars.previousJpegNumberPlateSizeImageCount = 0;

		imageCaptureTile0.vars.jpegFullSizeImageNumbers = 0;
		imageCaptureTile0.vars.jpegNumberPlateSizeImageNumbers = 0;
		imageCaptureTile0.vars.previousJpegFullSizeImageNumbers = 0;
		imageCaptureTile0.vars.previousJpegNumberPlateSizeImageNumbers = 0;
		//--------------------------------------------
		//Debug_Output1(0,"	NP image Count: %d",Number_of_NP_BatchedImages);	
		//qDebug()<<"	NP image Count:"<<Number_of_NP_BatchedImages;
		/*if (1 == iSomLogEnable)
		{
			log0(1, 0x87, " ");
			log1(1, 0x87, "	NP image Count:", Number_of_NP_BatchedImages);
		}*/
		for (i = 0; i < Number_of_NP_BatchedImages; i++)
		{
			Debug_Output2(0,"		Image(%d): %s", i, FTP_NP_Filename[i]);
			//qDebug("		Image(%d): %s", i, FTP_NP_Filename[i]);
			/*if (1 == iSomLogEnable)
			{
				log1(1, 0x87, "		Image(",i);
				log0(0, 0x87, "): ");
				log0(0, 0x87, FTP_NP_Filename[i]);
			}*/

			/* Monitor Seconds sensor trigger */
			//if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (1 == cameraCount))	//TBD //commented on 26_09_2021
			/*if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
			{
				imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
			}*/
			if (imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
			{
				return;
			}
		}

		//Debug_Output1(0,"	FL image Count: %d",Number_of_FL_BatchedImages);	
		//qDebug()<<"	FL image Count:"<<Number_of_FL_BatchedImages;
		/*if (1 == iSomLogEnable)
			log1(1, 0x87, "	FL image Count:", Number_of_FL_BatchedImages);*/
		
		for (i = 0; i < Number_of_FL_BatchedImages; i++)
		{
			Debug_Output2(0,"		Image(%d): %s", i, FTP_FL_Filename[i]);
			//qDebug("		Image(%d): %s", i, FTP_FL_Filename[i]);
			/*if (1 == iSomLogEnable)
			{
				log1(1, 0x87, "		Image(", i);
				log0(0, 0x87, "): ");
				log0(0, 0x87, FTP_FL_Filename[i]);
			}*/

			/* Monitor Seconds sensor trigger */
			//if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (1 == cameraCount))	//TBD //commented on 26_09_2021
			/*if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
			{
				imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
			}*/
			if (imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
			{
				return;
			}
		}
	}

    //qDebug("\r\n");
	/*if (1 == iSomLogEnable)
	{
		log0(1, 0x87, " ");
	}*/
#endif
}

int SendImageFile_viaFTP(uint8_t instance, chanend_t txCmd)
{
	char tempPrintString[300];
	
	asm volatile("gettime %0" : "=r" (startTimer_S));
	
	//printf("\n\nXMOS Second time Start **************************************************************************>>>>>>>>>>>>\n\n");
#if 1	
    int nr, nc, p;
    uint8_t PktNumber=1;
    unsigned int row;
    unsigned int col;
    unsigned int uImageSize;
    //Matrix<short> grey;
    int iNum_of_Packets;
    //delayInMSec(1000);
    //char tempString[500];
    //char FTPFilename_with_NP[200];
    //char FTPFilename_with_FL[200];
    //char tempptr1[100];
    //char tempptr2[100];
    signed int iRet;
    uint16_t txStringLength=0;
    //char FTPBaseFilename2[200]="";
    bool ImageTransferFailed = 0, bSendfile;
	uint8_t result = MDM_ERR_NONE;
	
	//fileTransferInProgress = true;
	sendSomLogFile = false;

    GetImageFilesInDirToArray_For_FTP_Transfer(instance);	//TODO:: to be moved after TM Entry Mode Command
	
	/* Monitor Seconds sensor trigger */
	//if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (1 == cameraCount))	//TBD //commented on 26_09_2021
	/*if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
	{
		imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
	}*/
	/*if (imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
	{
		//bTwoCameraTriggered = 1;
		strcpy(imageCaptureTile0.vars.previousFileNameBase, FTPBaseFilename);
		if ((1 == iEnableBrighterDataImage) && (executeBrighterDataImageTask))	//TBD	//added on 07_09_2021
		{
			imageCaptureTile0.vars.previousBrighterDataImages = imageCaptureTile0.vars.currentBrighterDataImages;	//TBD	//added on 07_09_2021
		}
		return SECOND_SENSOR_TRIGGER_DETECTED;
	}*/

    ////imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = false;

	//--------------------------------------------------------
	//TBD //added 17_09_2021
	//TBD //added 17_09_2021
	/*if (1 == iDeleteImagesNoStableState)	//TBD //added 23_12_2021
	{
		//MspInterface_deleteImagesBasedOnVehicleOccupancyStatusInfo();
		ImageDeletion_deleteImagesBasedOnVehicleOccupancyStatusInfo(txCmd);
		if (imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
		{
			bTwoCameraTriggered = 1;
			strcpy(previousFileNameBase, FTPBaseFilename);
			if ((1 == iEnableBrighterDataImage) && (executeBrighterDataImageTask))
			{
				previousBrighterDataImages = currentBrighterDataImages;
			}
			return SECOND_SENSOR_TRIGGER_DETECTED;
		}
	}*/
	//--------------------------------------------------------

    int TotalImageCount = Number_of_NP_BatchedImages + Number_of_FL_BatchedImages;	//TODO:: to be moved after TM Entry Mode Command

/*    if(TotalImageCount > MAX_NUMBER_IMAGE_FILES_TO_SEND)	//TODO:: to be moved after TM Entry Mode Command
    {
        qDebug()<<"Image count exceeds the limit. NOT performing FTP File transfer"<<TotalImageCount;
        return -1;
    }*/

    //if (0 == TotalImageCount)	//TBD //added on 30_06_2021 //--to be Removed
	//
	if ((0 == TotalImageCount) /*&& (0 ==  iFileDownloadEnable) && (0 == iSomLogEnable)*/)	//TBD //added on 12_10_2021
    {
		Debug_Output1(0,"NO Images Available. TotalImageCount = %d ",TotalImageCount);
        //qDebug()<<"NO Images Available. "<<TotalImageCount;
        /*if (1 == iSomLogEnable)
			log1(1, 0x85, "NO Images Available",TotalImageCount);*/
        return -1;
    }

    /*if (FTPcameraCount < 2)		//TBD	//commented on 03_08_2021
    {
        //qDebug()<<"SECND Camera";
        FTPcameraCount++;
    }*/

	/* Dummy Commands, Data send to Modem to immediate Recovery, in case of multiple sensor triggers during previous FTP */
	/*if ((cameraCount > 1) && (usbModemStatus))
	{
		UsbModem_sendDummyData();	//TBD //added on 07_10_2021
		delayInMSec(1000);		//TBD //added on 07_10_2021
		
		if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
		{
			imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
		}
		if(imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
		{
			bTwoCameraTriggered = 1;
			strcpy(previousFileNameBase, FTPBaseFilename);
			return SECOND_SENSOR_TRIGGER_DETECTED;
		}

		system("lsusb");	//TBD //added on 22_10_2021
		delayInMSec(50);	
		if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
		{
			imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
		}
		if(imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
		{
			bTwoCameraTriggered = 1;
			strcpy(previousFileNameBase, FTPBaseFilename);
			return SECOND_SENSOR_TRIGGER_DETECTED;
		}
			
		system("modprobe option");	//TBD //added on 22_10_2021
		delayInMSec(50);
		if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
		{
			imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
		}
		if(imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
		{
			bTwoCameraTriggered = 1;
			strcpy(previousFileNameBase, FTPBaseFilename);
			return SECOND_SENSOR_TRIGGER_DETECTED;
		}

		//TBD //added on 22_10_2021
        if (0 == strncmp(modemPartNumber, "ME910C1-WW", strlen("ME910C1-WW")))
        {
            system("echo 1bc7 1101 > /sys/bus/usb-serial/drivers/option1/new_id");	//ME910C1-WW
        }
        else if (0 == strncmp(modemPartNumber, "LE910-NA1", strlen("LE910-NA1")))
        {
            system("echo 1bc7 0036 > /sys/bus/usb-serial/drivers/option1/new_id");	//LE910-NA1
        }
        else if (0 == strncmp(modemPartNumber, "LE910C4-CN", strlen("LE910C4-CN")))
        {
            system("echo 1bc7 1201 > /sys/bus/usb-serial/drivers/option1/new_id");	//LE910C4-CN
        }
        else
        {
            qDebug()<<"Invalid Modem Part Number: "<<modemPartNumber;
        }
		delayInMSec(50);
		if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
		{
			imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
		}
		if(imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
		{
			bTwoCameraTriggered = 1;
			strcpy(previousFileNameBase, FTPBaseFilename);
			return SECOND_SENSOR_TRIGGER_DETECTED;
		}
		
		UsbModem_sendInitCommands("AT\r\n");	//TBD //added on 22_10_2021
		if(imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
		{
			bTwoCameraTriggered = 1;
			strcpy(previousFileNameBase, FTPBaseFilename);
			return SECOND_SENSOR_TRIGGER_DETECTED;
		}
		delayInMSec(50);
		if(imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
		{
			bTwoCameraTriggered = 1;
			strcpy(previousFileNameBase, FTPBaseFilename);
			return SECOND_SENSOR_TRIGGER_DETECTED;
		}
		
	}*/
	
#if 1	//TBD //commented on 12_08_2021 //--to be UNcommented
	//static bool isSomInTransparentMode = false;
	
	//if (!isSomInTransparentMode)	//TBD //commented on 02_08_2021 //--to be UNcommented
	{
		Debug_Output1(0,"Transparent mode Entry",0);
		//qDebug()<<"Transparent mode Entry";
        /*if (1 == iSomLogEnable)
			log0(1, 0x45, "Transparent mode Entry");*/
		
		/*iRet = fnvSend_TransparentModeEntry_ViaUART();
		if(iRet < 0)
		{
			qDebug()<<"No response from MSP for TM Entry command.";            
			qDebug()<<"SOM-MSP FTP related Communication will NOT process further.";
            if (1 == iSomLogEnable)
			{
				log0(1, 0x47, "No response from MSP for TM Entry command.");
				log0(1, 0x47, "SOM-MSP FTP related Communication will NOT process further.");
			}
			ImageTransferFailed = 1;
			//return -1;
			//if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (1 == cameraCount))	//TBD //commented on 26_09_2021
			if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
			{
				imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
			}
			if(imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
			{
				bTwoCameraTriggered = 1;
				strcpy(previousFileNameBase, FTPBaseFilename);
				return SECOND_SENSOR_TRIGGER_DETECTED;
			}
			return iRet;
		}*/
		
		/*else if (0 == iRet)
		{
			isSomInTransparentMode = true;
		}*/
	}
	/*else
	{
		isSomInTransparentMode = false;
		qDebug()<<"SOM Already In Transparent Mode!";
		qDebug("\r\n");
	}*/
#endif

#if 0
//Send FTP FileInformation
    ////iRet = fnvSend_FTPFileInfo_ViaUART(TotalImageCount,FTPFilename_with_FL , 0);

    /*if(iRet < 0)
    {
        qDebug()<<"NP: No response from MSP for FTP file info command. SOM-MSP FTP related Communication will halt ";
        if (1 == iSomLogEnable)
			log0(1, 0x47, "NP: No response from MSP for FTP file info command. SOM-MSP FTP related Communication will halt");
        //FTPFailedBaseFileCount++;
        //fnStoreFailedBaseFilename(FTPFailedBaseFileCount,FTPBaseFilename);
        ImageTransferFailed = 1;
        //break;
        return iRet;
    }*/

    //delayInMSec(50);
#endif
/*	
	usbModemStatus = false;		//TBD //added on 17_12_2021
	
    // USB Modem Check Status 
    if (0 != UsbModem_checkStatus())
    {
        return -2;
    }
	
	usbModemStatus = true;	//TBD //added on 17_12_2021
*/
	/*if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
	{
		imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
	}
	if(imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
	{
		bTwoCameraTriggered = 1;
		strcpy(previousFileNameBase, FTPBaseFilename);
		return SECOND_SENSOR_TRIGGER_DETECTED;
	}*/

    /* Send Dummy Bytes if previously error occurred */
    //if (sendAppendCommand)
    //{
        //sendAppendCommand = false;
        /* Send dummy 1500 bytes */
        //UsbModem_sendDummyData();
    //}
#if 0 //PS:20022023	
/*	UsbModem_sendDummyData();

	mdm_send_AT_command("ATS12=?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES,(DEFAULT_TIME_OUT*10));

	for(int checkModemUart = 0; checkModemUart <= 5; checkModemUart++)
	{
		if(checkModemUart >= 2)
		{
			result = mdm_send_data( (uint8_t*) "+++", 3, 40 );    //2209
		}
		
		result = mdm_send_AT_command("AT\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);
		
		if(result == MDM_ERR_NONE) break;
		if(checkModemUart>=4)
		{
			rtos_printf("HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE from Tile 0\n");
			InterTileCommTile0_sendPacketToHostMcuViaTile1(txCmd, HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE);
			imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = 1;
			bootUpFlag = 1;
			return SECOND_SENSOR_TRIGGER_DETECTED;
		}
	}
*/	
//ps:20022023	mdm_send_AT_command("ATS12?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES,(DEFAULT_TIME_OUT*10));

//ps:20022023	mdm_send_AT_command("ATS12=20\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES,(DEFAULT_TIME_OUT*10));

    //delayInMSec(2000);	//TBD //added on 05_07_2021 //--to be Removed
    //delayInMSec(1000);	//TBD //added on 30_06_2021 //--to be Removed
    //delayInMSec(2000);	//TBD //added on 25_06_2021 //--to be Removed
    //delayInMSec(2000);	//TBD //added on 25_06_2021 //--to be Removed
//ps:20022023	mdm_send_AT_command("AT\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES,(DEFAULT_TIME_OUT*10));
	//UsbModem_sendInitCommands("AT\r\n");	//TBD //added on 27_09_2021 //--to be Removed
	/*if(imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
	{
		bTwoCameraTriggered = 1;
		strcpy(previousFileNameBase, FTPBaseFilename);
		return SECOND_SENSOR_TRIGGER_DETECTED;
	}*/

	/* Close lastly opened FTP PUT in case if Simulteneous trigger happens */
	//if (previousAppendExit)	//TBD //commented on 12_08_2021 //--to be Removed
	{
		////previousAppendExit = false;
		//uint8_t dummyByte = 0xAA;
		//Ftp_sendPacketData(1, 1, &dummyByte, 1);    
		//delayInMSec(200); //this delay is required to get respo
        /* Send dummy 1500 bytes */
        ////UsbModem_sendDummyData();
	}
		
	//if (ftpAlreadyConnected)	//TBD //added on 12_08_2021 //--to be Removed
	{
		////ftpAlreadyConnected = false;
		////Ftp_closeSession();	
	}
	
//ps:20022023	mdm_send_AT_command("AT\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES,(DEFAULT_TIME_OUT*10));
    ////UsbModem_sendInitCommands("AT\r\n");	//TBD //added on 25_06_2021 //--to be Removed
	/*if(imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
	{
		bTwoCameraTriggered = 1;
		strcpy(previousFileNameBase, FTPBaseFilename);
		return SECOND_SENSOR_TRIGGER_DETECTED;
	}*/
//ps:20022023	mdm_send_AT_command("ATE0\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES,(DEFAULT_TIME_OUT*10));
    ////UsbModem_sendInitCommands("ATE0\r\n");	//TBD //added on 25_06_2021 //--to be Removed
	/*if(imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
	{
		bTwoCameraTriggered = 1;
		strcpy(previousFileNameBase, FTPBaseFilename);
		return SECOND_SENSOR_TRIGGER_DETECTED;
	}*/
//ps:20022023	mdm_send_AT_command("AT+CMEE=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES,(DEFAULT_TIME_OUT*10));
	////UsbModem_sendInitCommands("AT+CMEE=1\r\n");	//TBD //added on 22_07_2021
	/*if(imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
	{
		bTwoCameraTriggered = 1;
		strcpy(previousFileNameBase, FTPBaseFilename);
		return SECOND_SENSOR_TRIGGER_DETECTED;
	}*/
	//UsbModem_sendInitCommands("AT+CSQ\r\n");	//TBD //added on 22_07_2021
//ps:20022023	mdm_send_AT_command("AT+CSQ\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES,(DEFAULT_TIME_OUT*10));
	////UsbModem_sendATcommand("AT+CSQ\r\n", 1, 20);
	/*if(imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
	{
		bTwoCameraTriggered = 1;
		strcpy(previousFileNameBase, FTPBaseFilename);
		return SECOND_SENSOR_TRIGGER_DETECTED;
	}*/

	/*UsbModem_sendInitCommands("AT+CFUN=1\r\n");	//TBD //added on 22_07_2021
	if(imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
	{
		bTwoCameraTriggered = 1;
		strcpy(previousFileNameBase, FTPBaseFilename);
		return SECOND_SENSOR_TRIGGER_DETECTED;
	}*/

    /*UsbModem_sendInitCommands("AT#FTPCFG=1200,1,0,1\r\n");
    UsbModem_sendInitCommands("AT#FTPTO=1200\r\n");
    UsbModem_sendInitCommands("AT#FTPOPEN=\"ftp.civicsmart.com\",SmartBollard,ImageTransferTest,1\r\n");
    UsbModem_sendInitCommands("AT#FTPPUT=\"TestFile.txt\",1\r\n");
    UsbModem_sendInitCommands("AT#FTPAPPEXT=10,1\r\n");
    UsbModem_sendInitCommands("0123456789");
    UsbModem_sendInitCommands("AT#FTPCLOSE\r\n");
    delayInMSec(2000);	//TBD //added on 25_06_2021 //--to be Removed
    delayInMSec(2000);	//TBD //added on 25_06_2021 //--to be Removed
    delayInMSec(2000);	//TBD //added on 25_06_2021 //--to be Removed*/

    //TODO:: close FTP If Already Open

	//--------------------------------------------------------
	//TBD //added 17_09_2021
	////if (1 == iDeleteImagesNoStableState)	//TBD //added 23_12_2021
	{
		////MspInterface_deleteImagesBasedOnVehicleOccupancyStatusInfo();
		//ImageDeletion_deleteImagesBasedOnVehicleOccupancyStatusInfo(txCmd);
		/*if (imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
		{
			bTwoCameraTriggered = 1;
			strcpy(previousFileNameBase, FTPBaseFilename);
			if ((1 == iEnableBrighterDataImage) && (executeBrighterDataImageTask))
			{
				previousBrighterDataImages = currentBrighterDataImages;
			}
			return SECOND_SENSOR_TRIGGER_DETECTED;
		}*/
	}
	//--------------------------------------------------------

    /* FTP Open Sesion */

    //qDebug("\r\n");
	
	Debug_Output1(0,"FTP Session Starts",0);
	sprintf(tempPrintString, "\nCalling the function Ftp_openSession");
			                Debug_TextOut(0, tempPrintString);
    printf("\n Calling the function Ftp_openSession ");
    
    ////qDebug()<<"FTP Session Starts";
    /*if (1 == iSomLogEnable)
		log0(1, 0x87, "FTP Session Starts");
    //qDebug("\r\n");*/
#endif //PS:20022023
    uint8_t status = MDM_ERR_NONE;
	
   //ps:20022023 status = Ftp_openSession();
	//Debug_Output1(0,"1111111111111111111111111111",0);
	if (MDM_SECOND_SENSOR_TRIGGER_DETECTED == status)
	{
		return SECOND_SENSOR_TRIGGER_DETECTED;
	}
    else if (MDM_ERR_NONE == status)
    {
		//--------------------------------------------------------
        //TBD //Added on 29_10_2021
		//Ftp_executeFileDownload();	//TBD //--to be Removed
		//--------------------------------------------------------
		//Checklogfilesize();//TBD //--to be Removed
        uint16_t count;
        uint16_t txFileNumber = 0;
        uint16_t startIndex = 0;
        uint16_t totalTxFiles = 0;

        /* First All current Images, Previous Images will be transfered followed by batched Images */
        for (count = 0; count < 3; count++)
        {
            //For transferring Number plate image
            if ((0 != Number_of_NP_BatchedImages) && ((iFTPFileSendOption & NP_SIZE_FTP_IMAGE) == NP_SIZE_FTP_IMAGE))
            {
                /* Current Image files */
                if (0 == count)
                {
                    startIndex = 0;
                    totalTxFiles = imageCaptureTile0.vars.jpegNumberPlateSizeImageCount;
                    if (totalTxFiles > startIndex)
                    {
						Debug_Output1(0,"Initiating Current NP Image Transfer... %d",totalTxFiles);
                        //qDebug()<<"Initiating Current NP Image Transfer..."<<totalTxFiles;
                        /*if (1 == iSomLogEnable)
							log1(1, 0x87, "Initiating Current NP Image Transfer...",totalTxFiles);*/
                    }
                }
                /* Previous Image files */
                else if (1 == count)
                {
                    startIndex = imageCaptureTile0.vars.jpegNumberPlateSizeImageCount;
                    totalTxFiles = imageCaptureTile0.vars.jpegNumberPlateSizeImageCount + imageCaptureTile0.vars.previousJpegNumberPlateSizeImageCount;
                    if (totalTxFiles > startIndex)
                    {
						Debug_Output1(0,"Initiating Previous NP Image Transfer... %d",((totalTxFiles > startIndex) ? (totalTxFiles - startIndex) : imageCaptureTile0.vars.previousJpegNumberPlateSizeImageCount));
                        //qDebug()<<"Initiating Previous NP Image Transfer..."<< ((totalTxFiles > startIndex) ? (totalTxFiles - startIndex) : imageCaptureTile0.vars.previousJpegNumberPlateSizeImageCount);
                        /*if (1 == iSomLogEnable)
							log1(1, 0x87, "Initiating Previous NP Image Transfer...",((totalTxFiles > startIndex) ? (totalTxFiles - startIndex) : imageCaptureTile0.vars.previousJpegNumberPlateSizeImageCount));*/
                    }
                }
                /* Batched Image files */
                else if (2 == count)
                {
                    startIndex = imageCaptureTile0.vars.jpegNumberPlateSizeImageCount + imageCaptureTile0.vars.previousJpegNumberPlateSizeImageCount;
                    totalTxFiles = (Number_of_NP_BatchedImages > (imageCaptureTile0.vars.jpegNumberPlateSizeImageCount + imageCaptureTile0.vars.previousJpegNumberPlateSizeImageCount)) ? Number_of_NP_BatchedImages : (imageCaptureTile0.vars.jpegNumberPlateSizeImageCount + imageCaptureTile0.vars.previousJpegNumberPlateSizeImageCount);
                    if (totalTxFiles > startIndex)
                    {
						Debug_Output1(0,"Initiating Batched NP Image Transfer... %d",(totalTxFiles - startIndex));
                        //qDebug()<<"Initiating Batched NP Image Transfer..."<< (totalTxFiles - startIndex);
                        /*if (1 == iSomLogEnable)
							log1(1, 0x87, "Initiating Batched NP Image Transfer...",(totalTxFiles - startIndex));*/
                    }
                }

                //for(int i=1;i<=imageCaptureTile0.vars.jpegNumberPlateSizeImageCount;i++)
                //for(int i=0;i<Number_of_NP_BatchedImages;i++)
            //totalTxFiles = 1;//ps:21022023    
			for (int i = startIndex; i < totalTxFiles; i++)
                {
                    //if(((iImageFilesTobeTransferred>>(i-1))&0x1))
                    {
                        bSendfile = 0;

                        /* Send the Current Image files */
                        if (0 == count)
                        {
                            //if((strncmp(&FTP_NP_Filename[i][7], &imageCaptureTile0.vars.currentFileNameBase[4], 20)) == 0)
                            //ps:22022023
							if((strncmp(&FTP_NP_Filename[i][10], &imageCaptureTile0.vars.currentFileNameBase[4], 20)) == 0)
							{
								Debug_Output1(0,"Current NP: %s",FTP_NP_Filename[i]);
                                //qDebug()<<"Current NP: "<<FTP_NP_Filename[i];
                                /*if (1 == iSomLogEnable)
								{
									log0(1, 0x87, "Current NP:");
									log0(0, 0x87, FTP_NP_Filename[i]);
								}*/
                                bSendfile = 1;
                            }
                        }
                        /* Send the Previous Image files */
                        else if (1 == count)
                        {
                            //if((strncmp(&FTP_NP_Filename[i][7], &imageCaptureTile0.vars.previousFileNameBase[4], 20)) == 0)
                            //ps:22022023
						    if((strncmp(&FTP_NP_Filename[i][10], &imageCaptureTile0.vars.previousFileNameBase[4], 20)) == 0)
							{
								Debug_Output1(0,"Previous NP: %s",FTP_NP_Filename[i]);
                                //qDebug()<<"Previous NP: "<<FTP_NP_Filename[i];
                                /*if (1 == iSomLogEnable)
								{
									log0(1, 0x87, "Previous NP: ");
									log0(0, 0x87, FTP_NP_Filename[i]);
								}*/
                                bSendfile = 1;
                            }
                        }
                        /* Send the Batched Image files */
                        else if (2 == count)
                        {
                            /*if ((strncmp(&FTP_NP_Filename[i][34],"00",strlen("00"))) == 0)	//TBD	//commented on 06_08_2021
                            {
                                qDebug()<<"Batched NP-00: "<<FTP_NP_Filename[i];
                                bSendfile = 1;
                            }*/					

							char fileNameIndex[3];
							
							/* Send All Batched Files */
							if (iBatchedImagesToSend >= 0xFFFFFFFF)
							{
								strncpy(fileNameIndex, &FTP_NP_Filename[i][34], 2);
								bSendfile = 1;
							}
							/* Send Selected Batched Files */
							else
							{
								for (uint32_t checkIndex = 0; checkIndex < 32; checkIndex++)								
								{										
									if ((iBatchedImagesToSend >> checkIndex) & 0x1)	//TBD	//added on 06_08_2021
									{
										sprintf(fileNameIndex, "%02d", checkIndex);										
										//sprintf(fileNameIndex, "%02d", (i - startIndex));										
										
										//if (0 == strncmp(&FTP_NP_Filename[i][34], fileNameIndex, strlen(fileNameIndex)))
										//ps:22022023
										if (0 == strncmp(&FTP_NP_Filename[i][37], fileNameIndex, strlen(fileNameIndex)))
										{
											Debug_Output2(0,"Batched NP-%s: %s", fileNameIndex, FTP_NP_Filename[i]);
											//qDebug("Batched NP-%s: %s", fileNameIndex, FTP_NP_Filename[i]);
											bSendfile = 1;
											break;
										}					
									}									

									/*if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
									{
										imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
									}
									if(imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
									{
										//return SECOND_SENSOR_TRIGGER_DETECTED;
										iRet = SECOND_SENSOR_TRIGGER_DETECTED;
										return iRet;		//TBD	//UNcommented on 05_07_2021
										//break;	//TBD	//commented on 05_07_2021
									}*/
								}
							}			
							
							if (1 == bSendfile)
							{
								Debug_Output1(0,"Batched NP: %s",FTP_NP_Filename[i]);
								//qDebug("Batched NP-%s: %s", fileNameIndex, FTP_NP_Filename[i]);
								//qDebug("Batched NP: %s", FTP_NP_Filename[i]);
                                /*if (1 == iSomLogEnable)
								{
									log0(1, 0x87, "Batched NP: ");
									log0(0, 0x87,FTP_NP_Filename[i]);
								}*/
							}
                        }

                        if (1 == bSendfile)
                        {
                            //iRet = FileOpen(FTP_NP_Filename[i],i);
                            //iRet = FileOpen(FTP_NP_Filename[i], txFileNumber);
                            printf("\nCalling the function FileOpen ===========> ");
                            // Saving the file name for writing to SF
                            memcpy(SF_Filename, FTP_NP_Filename[i], sizeof(FTP_NP_Filename[i]));	
							iRet = FileOpen(FTP_NP_Filename[i], txFileNumber, txCmd);							
                            txFileNumber++;

                            if (iRet < 0)
                            {
								Debug_Output1(0,"NP File Open. iRet: %d",iRet);
                                //qDebug()<<"File Open fails.";	// SOM-MSP FTP related
                                //qDebug()<<"NP File Open. iRet:" << iRet;	// SOM-MSP FTP related
                                /*if (1 == iSomLogEnable)
									log1(1, 0x87, "NP File Open. iRet:",iRet);*/
                                //if((iRet == -7) || (iRet == RESTART_FILE_TRANSFER_SESSION))
                                //if((iRet == -7) || (iRet == RESTART_FILE_TRANSFER_SESSION) || (iRet == SECOND_SENSOR_TRIGGER_DETECTED))	//commented on 24_06_2021
                                {
                                    //TODO:: need to exit loop to end the communication
                                    if ((SECOND_SENSOR_TRIGGER_DETECTED == iRet) ||  (RESTART_FILE_TRANSFER_SESSION == iRet))	//TBD //added on 26_11_2021
                                    {
                                        return iRet;		//TBD	//UNcommented on 05_07_2021
                                    }
                                    else
                                    {
                                        break;	//TBD	//commented on 05_07_2021
                                    }
                                }
                            }
                        }
                    }

					//if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (1 == cameraCount))	//TBD //commented on 26_09_2021
					/*if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
                    {
                        imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
                    }
                    if(imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
                    {
                        //return SECOND_SENSOR_TRIGGER_DETECTED;
                        iRet = SECOND_SENSOR_TRIGGER_DETECTED;
                        return iRet;		//TBD	//UNcommented on 05_07_2021
                        //break;	//TBD	//commented on 05_07_2021
                    }*/
                }
            }

            if (iRet < 0)
            {
                break;
            }

            //For transferring Full image
            if ((0 != Number_of_FL_BatchedImages) && ((iFTPFileSendOption & NP_SIZE_FTP_IMAGE) == NP_SIZE_FTP_IMAGE))
            {
                /* Current Image files */
                if (0 == count)
                {
                    startIndex = 0;
                    totalTxFiles = imageCaptureTile0.vars.jpegFullSizeImageCount;
                    if (totalTxFiles > startIndex)
                    {
						Debug_Output1(0,"Initiating Current FL Image Transfer... %d",totalTxFiles);
                        //qDebug()<<"Initiating Current FL Image Transfer..."<<totalTxFiles;
                        /*if (1 == iSomLogEnable)
							log1(1, 0x87, "Initiating Current FL Image Transfer...",totalTxFiles);*/
                    }
                }
                /* Previous Image files */
                else if (1 == count)
                {
                    startIndex = imageCaptureTile0.vars.jpegFullSizeImageCount;
                    totalTxFiles = imageCaptureTile0.vars.previousJpegFullSizeImageCount + imageCaptureTile0.vars.jpegFullSizeImageCount;
                    if (totalTxFiles > startIndex)
                    {
						Debug_Output1(0,"Initiating Previous FL Image Transfer... %d",((totalTxFiles > startIndex) ? (totalTxFiles - startIndex) : imageCaptureTile0.vars.previousJpegFullSizeImageCount));
                        //qDebug()<<"Initiating Previous FL Image Transfer..."<< ((totalTxFiles > startIndex) ? (totalTxFiles - startIndex) : previousJpegFullSizeImageCount);
                        /*if (1 == iSomLogEnable)
							log1(1, 0x87, "Initiating Previous FL Image Transfer....",((totalTxFiles > startIndex) ? (totalTxFiles - startIndex) : previousJpegFullSizeImageCount));
						*/
                    }
                }
                /* Batched Image files */
                else if (2 == count)
                {
                    startIndex = imageCaptureTile0.vars.jpegFullSizeImageCount + imageCaptureTile0.vars.previousJpegFullSizeImageCount;
                    totalTxFiles = (Number_of_FL_BatchedImages > (imageCaptureTile0.vars.jpegFullSizeImageCount + imageCaptureTile0.vars.previousJpegFullSizeImageCount)) ? Number_of_FL_BatchedImages : (imageCaptureTile0.vars.jpegFullSizeImageCount + imageCaptureTile0.vars.previousJpegFullSizeImageCount);
                    if (totalTxFiles > startIndex)
                    {
						Debug_Output1(0,"Initiating Batched FL Image Transfer... %d",(totalTxFiles - startIndex));
                        //qDebug()<<"Initiating Batched FL Image Transfer..."<< (totalTxFiles - startIndex);
                        /*if (1 == iSomLogEnable)
							log1(1, 0x87, "Initiating Batched FL Image Transfer...",(totalTxFiles - startIndex));*/
                    }
                }

                //for(int i=1;i<=jpegFullSizeImageCount;i++)
                //for(int i=0;i<Number_of_FL_BatchedImages;i++)
					//totalTxFiles = 1;//ps:21022023
				//Debug_Output1(0,"**********startIndex: %d**********",startIndex);
                for (int i = startIndex; i < totalTxFiles; i++)
                {
					/* Re-Open FTP Session in Case of ME910C1-WW */				
					/*if (me910Modem)	//TBD //added on 26_11_2021
					{
						Ftp_reOpenSession();
					}*/
					
                    //if(((iImageFilesTobeTransferred>>(i-1))&0x1))
                    {
                        bSendfile = 0;
						//Debug_Output1(0,"**********count: %d**********",count);
                        /* Send the Current Image files */
                        if (0 == count)
                        {
                            //if(strncmp(&FTP_FL_Filename[i][7], &imageCaptureTile0.vars.currentFileNameBase[4],20))
                        Debug_Output1(0,"FTP_FL_Filename: %s",FTP_FL_Filename[i]);    
						Debug_Output1(0,"imageCaptureTile0.vars.currentFileNameBase[4]: %s",
						imageCaptureTile0.vars.currentFileNameBase); 
						//if((strncmp(&FTP_FL_Filename[i][7], &imageCaptureTile0.vars.currentFileNameBase[4],20)) == 0)
						//ps:22022023
							if((strncmp(&FTP_FL_Filename[i][10], &imageCaptureTile0.vars.currentFileNameBase[4],20)) == 0)
                            {
                                bSendfile = 1;
								Debug_Output1(0,"Current FL: %s",FTP_FL_Filename[i]);
                                //qDebug()<<"Current FL: "<<FTP_FL_Filename[i];
                                /*if (1 == iSomLogEnable)
								{
									log0(1, 0x87, "Current FL: ");
									log0(0, 0x87, FTP_FL_Filename[i]);
								}*/
                            }
                        }
                        /* Send the Previous Image files */
                        else if (1 == count)
                        {
                            //if((strncmp(&FTP_FL_Filename[i][7], &imageCaptureTile0.vars.previousFileNameBase[4], 20)) == 0)
                            //ps:22022023 
							if((strncmp(&FTP_FL_Filename[i][10], &imageCaptureTile0.vars.previousFileNameBase[4], 20)) == 0)
							{
								Debug_Output1(0,"Previous FL: %s",FTP_FL_Filename[i]);
                                //qDebug()<<"Previous FL: "<<FTP_FL_Filename[i];
                                /*if (1 == iSomLogEnable)
								{
									//log1(1, 0x87, "Previous FL: ",FTP_FL_Filename[i]);
									log0(1, 0x87, "Previous FL: ");
									log0(0, 0x87, FTP_FL_Filename[i]);
								}*/

                                bSendfile = 1;
                            }
                        }						/* Send the Batched Image files */
                        else if (2 == count)
                        {
                            /*if ((strncmp(&FTP_FL_Filename[i][34],"00",strlen("00"))) == 0)	//TBD	//commented on 06_08_2021
                            {
                                qDebug()<<"Batched FL-00: "<<FTP_FL_Filename[i];
                                bSendfile = 1;
                            }*/

							char fileNameIndex[3];
							
							/* Send All Batched Files */
							if (iBatchedImagesToSend >= 0xFFFFFFFF)
							{
								strncpy(fileNameIndex, &FTP_FL_Filename[i][34], 2);
								bSendfile = 1;
							}
							/* Send Selected Batched Files */
							else
							{
								for (uint32_t checkIndex = 0; checkIndex < 32; checkIndex++)								
								{										
									if ((iBatchedImagesToSend >> checkIndex) & 0x1)	//TBD	//added on 06_08_2021
									{
										sprintf(fileNameIndex, "%02d", checkIndex);										
										//sprintf(fileNameIndex, "%02d", (i - startIndex));										
										
										//if (0 == strncmp(&FTP_FL_Filename[i][34], fileNameIndex, strlen(fileNameIndex)))
										//ps:22022023
										if (0 == strncmp(&FTP_FL_Filename[i][37], fileNameIndex, strlen(fileNameIndex)))
										{
											Debug_Output2(0,"Batched NP-%s: %s", fileNameIndex, FTP_FL_Filename[i]);
											//qDebug("Batched NP-%s: %s", fileNameIndex, FTP_FL_Filename[i]);
											bSendfile = 1;
											break;
										}					
									}									
									
									/*if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
									{
										imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
									}
									if(imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
									{
										//return SECOND_SENSOR_TRIGGER_DETECTED;
										iRet = SECOND_SENSOR_TRIGGER_DETECTED;
										return iRet;		//TBD	//UNcommented on 05_07_2021
										//break;	//TBD	//commented on 05_07_2021
									}*/
								}
							}			
							
							if (1 == bSendfile)
							{
								Debug_Output1(0,"Batched FL: %s", FTP_FL_Filename[i]);
								//qDebug("Batched FL-%s: %s", fileNameIndex, FTP_FL_Filename[i]);
								//qDebug("Batched FL: %s", FTP_FL_Filename[i]);
                                /*if (1 == iSomLogEnable)
								{
									log0(1, 0x87, "Batched FL: ");
									log0(0, 0x87, FTP_FL_Filename[i]);
								}*/
							}
                        }

                        if (1 == bSendfile)
                        {
                            //iRet = FileOpen(FTP_FL_Filename[i],i);
                            //iRet = FileOpen(FTP_FL_Filename[i], txFileNumber);
							printf("Before FileOpen1794: %s, txFileNumber:%d, txcmd:%d ", FTP_FL_Filename[i], txFileNumber, txCmd);
							iRet = FileOpen(FTP_FL_Filename[i], txFileNumber, txCmd);
                            txFileNumber++;

                            if (iRet < 0)
                            {
								Debug_Output1(0,"Full Size File Open. iRet: %d",iRet);
                                //qDebug()<<"File Open fails.";	// SOM-MSP FTP related
                                //qDebug()<<"Full Size File Open. iRet:" << iRet;	// SOM-MSP FTP related
                                /*if (1 == iSomLogEnable)
									log1(1, 0x87, "Full Size File Open. iRet:",iRet);*/
                                //if((iRet == -7) || (iRet == RESTART_FILE_TRANSFER_SESSION))
                                //if((iRet == -7) || (iRet == RESTART_FILE_TRANSFER_SESSION) || (iRet == SECOND_SENSOR_TRIGGER_DETECTED))	//commented on 24_06_2021
                                {
                                    //TODO:: need to exit loop to end the communication
                                    if ((SECOND_SENSOR_TRIGGER_DETECTED == iRet) ||  (RESTART_FILE_TRANSFER_SESSION == iRet))	//TBD //added on 26_11_2021
                                    {
                                        return iRet;		//TBD	//UNcommented on 05_07_2021
                                    }
                                    else
                                    {
                                        break;	//TBD	//commented on 05_07_2021
                                    }
                                }
                            }
                        }
                        /*//iRet = FileOpen(FTP_FL_Filename[i],i);
						iRet = FileOpen(FTP_FL_Filename[i], i, txCmd);
                        if(iRet < 0)
                        {
                            qDebug()<<"File Open fails.";	// SOM-MSP FTP related
                            //if((iRet == -7) || (iRet == RESTART_FILE_TRANSFER_SESSION))
                            if((iRet == -7) || (iRet == RESTART_FILE_TRANSFER_SESSION) || (iRet == SECOND_SENSOR_TRIGGER_DETECTED))
                            {
                                return iRet;
                            }
                        }*/
                    }

					//if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (1 == cameraCount))	//TBD //commented on 26_09_2021
					/*if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
                    {
                        imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
                    }
                    if(imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
                    {
                        //return SECOND_SENSOR_TRIGGER_DETECTED;
                        iRet = SECOND_SENSOR_TRIGGER_DETECTED;
                        return iRet;		//TBD	//UNcommented on 05_07_2021
                        //break;	//TBD	//commented on 05_07_2021
                    }*/
                }

                if (iRet < 0)
                {
                    break;
                }
            }

			//if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (1 == cameraCount))	//TBD //commented on 26_09_2021
			/*if ((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected) && (cameraCount > 0) && (fileTransferInProgress))	//TBD //commented on 26_09_2021	
			{
				imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = autoParking::monitorCameraIndicationPins();
			}
			if(imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
			{
				//return SECOND_SENSOR_TRIGGER_DETECTED;
				iRet = SECOND_SENSOR_TRIGGER_DETECTED;
				return iRet;		//TBD	//UNcommented on 05_07_2021
				//break;	//TBD	//commented on 05_07_2021
			}*/
        }	//Image Upload Loop Finishes
		//------------------------------------------------------------		

		/* File Download Session */
		
		/*if (1 ==  iFileDownloadEnable)
		{
			Ftp_executeFileDownload();		//TBD //Added on 08_10_2021

			if(imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
			{
				bTwoCameraTriggered = 1;
				strcpy(previousFileNameBase, FTPBaseFilename);
				return SECOND_SENSOR_TRIGGER_DETECTED;
			}
		}*/
		//------------------------------------------------------------		
		
		/* SOM Log File Upload Session */
		#if 0
		if ((1 == iSomLogEnable) && (executeUploadSomLogFile))
		{
			/*char fileName[200];
			
			qDebug("\r\n");			
			qDebug()<<"SOM LOG File Upload Session";
			if (1 == iSomLogEnable)
			{
				log0(1, 0x87, " ");
				log0(1, 0x87,"SOM LOG File Upload Session ");
			}
		
			sprintf(fileName, "SOM_LOG_%04d_%05d.tar.gz", iCustomerId, iLeftMID);

			iRet = Ftp_uploadSomLogFile(fileName);

			if (RESTART_FILE_TRANSFER_SESSION == iRet)	//TBD //added on 26_11_2021
			{
				return iRet;
			}
			
			qDebug()<<"SOM LOG File Upload Session Completed";			
			qDebug("\r\n");
			if (1 == iSomLogEnable)
			{
				log0(1, 0x87,"SOM LOG File Upload Session Completed");
				log0(1, 0x87, " ");
			}*/
			SomLog_fileUploadSession();

			if(imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
			{
				bTwoCameraTriggered = 1;
				strcpy(imageCaptureTile0.vars.previousFileNameBase, imageCaptureTile0.vars.currentFileNameBase);
				return SECOND_SENSOR_TRIGGER_DETECTED;
			}
		}
		#endif
		//------------------------------------------------------------		
		/* FTP Close Session */
		//ps:20022023status = Ftp_closeSession();
		//------------------------------------------------------------		
	}
	
    //qDebug()<<"Transparent mode Exit";
    //iRet = fnvSend_TransparentModeExit_ViaUART();
    //fnvSend_TransparentModeExit_ViaUART();

    if(iRet < 0)
    {
        //qDebug()<<"No response from MSP for FTP TM Exit command. SOM-MSP FTP related Communication will halt ";
        return iRet;
    }



    cleanup_flash();
    asm volatile("gettime %0" : "=r" (currentTimer_S));
	printf("\r\n\n\nTime second tme XMOS is ON: %dms.\n", (((currentTimer_S - startTimer_S) / 100) / 1000));
	
    sprintf(tempPrintString, "\r\n\n\nTime second tme XMOS is ON: %dms.\n", (((currentTimer_S - startTimer_S) / 100) / 1000));
    Debug_TextOut(0,tempPrintString);
    printf("\n\nXMOS Second time End.  Number of images %d ************************************************************>>>>>>>>>>>>\n\n", num_images);
    num_images = 0;
    return 0;
#endif
}



#endif
///////////////////////////////////////////////////////////////////////////////////////////
static uint8_t Ftp_setType(void)
{
    uint8_t status = MDM_ERR_NONE;
    char cmdBuffer[200];

    DelayMs(200);

    Debug_TextOut(0,"Set FTP Type");
    sprintf(cmdBuffer, "AT#FTPTYPE=1\r\n");
    //Debug_TextOut(0, (const char*)cmdBuffer);

    status = Ftp_sendATcommand((const char *) cmdBuffer, rx_ans, sizeof(rx_ans)-1, 1, 3000);
    if (MDM_ERR_NONE != status)
    {
        Debug_Output1( 0, "Error Set FTP Type(%d)", status);
        mdm_send_AT_command("AT#FTPTYPE?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
    }
	
    return status;
}

uint8_t Ftp_activateGprs(void)
{
    uint16_t        j;
    //uint8_t         ret        = SOCERR_NONE;
    uint8_t         result     = MDM_ERR_NONE;
    uint16_t         i;

    /*if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE )//3oct18
    {
        return result;
    }
    else*/
    {
        if(MDM_AWAKE == FALSE)
        {
            //$//telit_power_on();
            ////////////////////////////////////////////////////
            Debug_TextOut(0,"Modem Boot-up In Progress in FTP...");

            /* Modem Boot up Settling Time Delay */
            //DelayMs(7000); //TBD
            //$//watchdog_pat();
            for(i = 0; i <= 27; i++)
            {
                Modem_No_Response_Count = 0;
                if(mdm_send_AT_command("AT\r\n", rx_ans, sizeof(rx_ans)-1,DEFAULT_RETRIES, DEFAULT_TIME_OUT*10) == FALSE)
                {
                   DelayMs(100);
                   MDM_AWAKE = TRUE;
                   //$//mdm_init_once_per_bootup_new();
                   break;
                }
                DelayMs(1000);
                //$//watchdog_pat();

                if(i >= 13) return result;
            }
            if(i >= 27) return result;

            if(LE910C4_CN_Flag==TRUE)
                    {
                    DelayMs(100);
                    mdm_send_AT_command("AT#SIMDET=2\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);
                    DelayMs(100);

                    DelayMs(100);
                    mdm_send_AT_command("AT#SIMDET?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

                    DelayMs(100);
                    mdm_send_AT_command("AT#SIMDET=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

                    DelayMs(1000);
                    }
                    else
                    {
                        //DelayMs(100);
                        //mdm_send_AT_command("AT+CGATT=0\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);
                        ////mdm_send_AT_command("AT#SIMDET=2\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);
                        //DelayMs(100);
                        mdm_send_AT_command("AT#SIMDET?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);

                        //DelayMs(100);
                        //mdm_send_AT_command("AT+CGATT=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

                        //DelayMs(100);
                        mdm_send_AT_command("AT+CFUN=5\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);
                        ////mdm_send_AT_command("AT#SIMDET=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

                        //DelayMs(1000);
                    }

            mdm_send_AT_command("AT+CFUN=5\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);

            DelayMs(100);
            mdm_send_AT_command("AT&P0\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);

            DelayMs(100);
            result = mdm_send_AT_command("ATE0&K0\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);

            mdm_send_AT_command("AT&D=2\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);
            ////////////////////////////////////////////////////
            ////telit_sock_open_states();
            Debug_TextOut(0,"MDM_AWAKE == FALSE" );
        }
        else
        {
            //$//telit_wakeup_sleep(FALSE);
        }

        if(mdm_comm_status < 5)    // if socket is not opened already//26-07-12
        {
            memset( (char*) cmd_send, 0, sizeof(cmd_send) );

            result = mdm_send_AT_command("AT#SGACT=1,1\r", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);//25-09-12
            DelayMs(200);
            //$//watchdog_pat();

            //diag_text_Screen( "Chk Auto N/W Actvn", FALSE );

            for ( j=0; j < (45*1000); j++ ) //increased from 20 to 45, Sierra SIM takes 3 minutes to register
            {
                DelayMs(2);

                if((j%1000)==0)
                {
                    if( mdm_send_AT_command("AT#SGACT?\r", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES,(DEFAULT_TIME_OUT*200)) == MDM_ERR_NONE )
                    {
                        if ( mdm_find_response(rx_ans, "#SGACT: 1,1" ) == TRUE )
                        {
                            mdm_comm_status = 4;
                            break;
                        }
                        mdm_send_AT_command("AT#SGACT=1,1\r", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*50);//25-09-12
                    }
                    mdm_comm_status = 0;
                    Last_Comms_Success_Flg = FALSE;
                    //$//watchdog_pat();
                }

                if(j==2000)
                {
                    ////if((j%10000)==0)
                    {
                        DelayMs(100);
                        mdm_send_AT_command("AT&P0\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);

                        if(LE910C4_CN_Flag==TRUE)
                                {
                                DelayMs(100);
                                mdm_send_AT_command("AT#SIMDET=2\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

                                DelayMs(100);
                                mdm_send_AT_command("AT#SIMDET?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

                                DelayMs(100);
                                mdm_send_AT_command("AT#SIMDET=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);
                                }
                                else
                                {
                                    //DelayMs(100);
                                    //mdm_send_AT_command("AT+CGATT=0\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);
                                    ////mdm_send_AT_command("AT#SIMDET=2\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

                                    //DelayMs(100);
                                    mdm_send_AT_command("AT#SIMDET?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);

                                    //DelayMs(100);
                                    //mdm_send_AT_command("AT+CGATT=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

                                    //DelayMs(100);
                                    mdm_send_AT_command("AT+CFUN=5\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);
                                    ////mdm_send_AT_command("AT#SIMDET=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);
                                }
                        //DelayMs(1000);

                        mdm_send_AT_command("AT&D=2\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
                    }
                }

                if((j%10000)==0)
                {
                    DelayMs(100);
                    mdm_send_AT_command("AT&P0\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);

                    if(LE910C4_CN_Flag==TRUE)
                            {
                            DelayMs(100);
                            mdm_send_AT_command("AT#SIMDET=2\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

                            DelayMs(100);
                            mdm_send_AT_command("AT#SIMDET?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

                            DelayMs(100);
                            mdm_send_AT_command("AT#SIMDET=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);
                            }
                            else
                            {
                                //DelayMs(100);
                                //mdm_send_AT_command("AT+CGATT=0\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);
                                ////mdm_send_AT_command("AT#SIMDET=2\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

                                //DelayMs(100);
                                mdm_send_AT_command("AT#SIMDET?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);

                                //DelayMs(100);
                                //mdm_send_AT_command("AT+CGATT=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);

                                //DelayMs(100);
                                mdm_send_AT_command("AT+CFUN=5\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);
                                ////mdm_send_AT_command("AT#SIMDET=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*1000);
                            }
                    //DelayMs(1000);
                    mdm_send_AT_command("AT&D=2\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*100);
                }
            }

            if(mdm_comm_status != 4)
            {
                //diag_text_Screen( "Auto N/W Regn Fail", Meter_Reset ,FALSE );
                //$//nw_operator_name_id_check();//try back up network now
                //$//Read_Mdm_diag_data();
            }
            else if(mdm_comm_status == 4)
            {
                //$//Read_Mdm_diag_data();
                //diag_text_Screen( "N/W Regn Success", Meter_Reset ,FALSE);
            }
        }
        else
        {
            result = MDM_FTP_BUSY;
        }


        Debug_Output1( 0, "mdm_comm_status=%d", mdm_comm_status );
        return result;
    }
}

#pragma stackfunction 1000
uint8_t Ftp_openSession(void)
{
    uint8_t status = MDM_ERR_NONE;
    char cmdBuffer[200];
	uint8_t result = MDM_ERR_NONE;

	uint8_t counts;

	Debug_TextOut(0,"Ftp_openSession");

	Ftp_sendATcommand("AT#E2ESC=?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);
	//DelayMs(100);
	
	Ftp_sendATcommand("AT#E2ESC=0\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);
	//DelayMs(100);
	
	Ftp_sendATcommand("AT#E2ESC?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);
	//DelayMs(100);
	
	Ftp_sendATcommand("ATS12?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);
	//DelayMs(100);
	
	Ftp_sendATcommand("ATS12=20\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);
	//DelayMs(100);						
	
	Ftp_sendATcommand("ATS12?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);
	//DelayMs(100);
	
	Debug_Output1(0,"finish counts = %d",counts);
	
    Debug_TextOut(0,"FTP Config");

    uint16_t count;

    for (count = 0; count < 2; count++)    //TBD //12_04_2021
    {
        /* Open FTP Session */
        Debug_TextOut(0,"Open FTP Session");
        //printSharp16x24("Open FTP Session", 50, 0, 0);
        //LCD_Top_Message_String = TRUE; 

        uint16_t i = 0,j = 0;
#if 1
        //sprintf(cmdBuffer, "AT#FTPOPEN=\"ftp.civicsmart.com\",SmartBollard,SBM3t3R@951#,1\r\n");  //TBD
		for (i = 0; i < 5; i++)
			{
				for(j=0;j<5;j++)
				{
					Ftp_sendATcommand("AT+COPS?\r", rx_ans, sizeof(rx_ans), DEFAULT_RETRIES, DEFAULT_TIME_OUT*20);
					Debug_Output1(0,"AT+COPS? response is: %s", (uint32_t)rx_ans);

					if(mdm_find_response((uint8_t*)rx_ans, "+COPS:" )==true)
					{
						if((mdm_find_response((uint8_t*)rx_ans, "Vodafone IN" )==true)||(mdm_find_response((uint8_t*)rx_ans, "Vi India" )==true))
						{
							Debug_Output1(0,"Module is Connected to Vodafone IN Network",0);
							sprintf(cmdBuffer, "AT#FTPOPEN=\"ftp.civicsmart.com\",SmartBollard,ImageTransferTest,1\r\n");  //TBD
							break;
						}
						else if((mdm_find_response((uint8_t*)rx_ans, "Jio 4G" )==true))  //IND-JIO
						{
							Debug_Output1(0,"Module is Connected to Jio 4G Network",0);
							sprintf(cmdBuffer, "AT#FTPOPEN=\"ftp.civicsmart.com\",SmartBollard,ImageTransferTest,1\r\n");  //TBD
							break;
						}
						else if((mdm_find_response((uint8_t*)rx_ans, "airtel" )==true))
						{
							Debug_Output1(0,"Module is Connected to airtel Network", 0);
							sprintf(cmdBuffer, "AT#FTPOPEN=\"ftp.civicsmart.com\",SmartBollard,ImageTransferTest,1\r\n");  //TBD
							break;
						}
						else if((mdm_find_response((uint8_t*)rx_ans, "Sierra Wireless" )==true)
								||(mdm_find_response((uint8_t*)rx_ans, "Sierra" )==true)
								||(mdm_find_response((uint8_t*)rx_ans, "sierra" )==true))
						{
							Debug_Output1(0,"Module is Connected to Sierra Wireless Network",0);
							sprintf(cmdBuffer, "AT#FTPOPEN=\"172.21.6.161\",SmartBollard,ImageTransferTest,1\r\n"); //--ONLY FOR US TESTING
							break;
						}
						else if(mdm_find_response((uint8_t*)rx_ans, "AT&T" )==true)
						{
							Debug_Output1(0,"Module is Connected to AT&T Network",0);
							sprintf(cmdBuffer, "AT#FTPOPEN=\"172.21.6.161\",SmartBollard,ImageTransferTest,1\r\n"); //--ONLY FOR US TESTING
							break;
						}
						else if((mdm_find_response((uint8_t*)rx_ans, "T-Mobile" )==true)
								||(mdm_find_response((uint8_t*)rx_ans, "t-Mobile" )==true))
						{
							Debug_Output1(0,"Module is Connected to T-Mobile Network",0); //t-mobile main UDP
							sprintf(cmdBuffer, "AT#FTPOPEN=\"172.21.6.161\",SmartBollard,ImageTransferTest,1\r\n"); //--ONLY FOR US TESTING
							break;
						}
						else if(mdm_find_response((uint8_t*)rx_ans, "Verizone" )==true)
						{
							Debug_Output1(0,"Module is Connected to Verizone Network",0);
							sprintf(cmdBuffer, "AT#FTPOPEN=\"172.21.6.161\",SmartBollard,ImageTransferTest,1\r\n"); //--ONLY FOR US TESTING
							break;
						}
						else if(mdm_find_response((uint8_t*)rx_ans, "+COPS: 0,0" )==true)
						{
							Debug_Output1(0,"Module is Connected to Unknown Network",0);
							sprintf(cmdBuffer, "AT#FTPOPEN=\"172.21.6.161\",SmartBollard,ImageTransferTest,1\r\n"); //--ONLY FOR US TESTING
							break;
						}
					}
					DelayMs(1000);
				}	
				DelayMs(500);				
				status = Ftp_sendATcommand((const char *)cmdBuffer, rx_ans, sizeof(rx_ans)-1, 1, 1000/*10000*/);
				
				//DelayMs(2000);
				if (MDM_ERR_NONE == status)
				{
					break;
				}
				else
				{
					////status = Ftp_closeSession();
					////DelayMs(2000);
				}
		   }
#endif
        if (MDM_ERR_NONE != status)
        {
            Debug_Output1( 0, "Error Open FTP Session(%d)", status);
            //printSharp16x24("Error Open FTP Session", 50, 0, 0);
            //LCD_Top_Message_String = TRUE;
            status = Ftp_closeSession();
            //return status;
        }
        else
        {
            Debug_TextOut(0,"Open FTP Session Success");
            //printSharp16x24("Open FTP Session Success", 50, 0, 0);
            //LCD_Top_Message_String = TRUE;

            /* Set the Type */
			Ftp_setType();

            /*if (MDM_ERR_NONE != status)
            {
                Debug_Output1( 0, "Error Set FTP Type(%d)", status);
                //status = Ftp_closeSession();
                //return status;
            }*/
            break;
        }

        ////DelayMs(500);
    }

    return status;
}

uint8_t Ftp_closeSession(void)
{
    uint8_t status = MDM_ERR_NONE;
    char cmdBuffer[200];

    /* Close FTP Session */
    Debug_TextOut(0,"Close FTP Session");
    //printSharp16x24("Close FTP Session", 50, 0, 0);
    //LCD_Top_Message_String = TRUE;

    sprintf(cmdBuffer, "AT#FTPCLOSE\r\n");

    status = Ftp_sendATcommand((const char *)cmdBuffer, rx_ans, sizeof(rx_ans)-1, 1, 3000);

    if (MDM_ERR_NONE != status)
    {
        Debug_Output1( 0, "Error Close FTP Session(%d)", status);
    }
    else
    {
        Debug_TextOut(0,"Close FTP Session Success");
    }

    /* Re-Establish UDP Socket Connection */
    Debug_Output1( 0, "mdm_comm_status (%d)", mdm_comm_status);

    if(mdm_comm_status == 4)
    {
        mdm_comm_status = 5;
        ////mdm_send_AT_command("AT#SO=1\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);
    }

    return status;
}

uint8_t Ftp_checkModemStatus(void)
{
    bool socketConnected = false;
    char command[100];
    uint8_t result = MDM_ERR_NONE;
    uint16_t i;
    uint16_t j;
    uint16_t k;
    uint16_t count;
    uint16_t modem_uart_bytes_counter;

    ////if((RTC_epoch_now() < (Ftp_checkModemStatus_timestamp + 60))&&(mdm_comm_status >= 4)) return mdm_comm_status;
    //$//get_battery_voltage_temp(); //Added to check Diagnostic parameters -- VT
    for (i = 0; i < 3; i++)
    {
        //$//telit_wakeup_sleep(FALSE);
        if ((ERROR_MODEM_RESPONSE == transparentModeErrorCode) || (mdm_comm_status == 0)) // To reinit Modem If MSP recieves as mdm_comm_status = 0
        {
            if (ERROR_MODEM_RESPONSE == transparentModeErrorCode)
            {
                Debug_TextOut(0,"FTP: Re-Initializing Modem");
                mdm_comm_status = 0;
            }

            ////telit_init();
            Debug_TextOut(0,"tried gprs modem recovery ftp");
            Modem_Recovery_Flag = true;
            Debug_TextOut(0,"MODEM Recovery in FTP Session");
            //printSharp16x24("MODEM Recovery in FTP Session", 50, 0, 0);
            //LCD_Top_Message_String = TRUE;

/*
            /////////////////////////////////
            //Forcefully switching OFF MODEM -- Vivek
            Modem_Off_initiated = 0; //To Switch Off Modem For Proper Recovery -- VT
            telit_power_off();
            TELIT_POWER_DISABLE();
            /////////////////////////////////
*/
            Modem_Power_ON_Sequence_Flag = true;
            Debug_Output1( 0, "Modem_Power_ON_Sequence_Flag=%d", Modem_Power_ON_Sequence_Flag);
            //Forcefully switching OFF MODEM -- Vivek
            Modem_Off_initiated = 0; //To Switch Off Modem For Proper Recovery -- VT
            //$//telit_power_off_by_power_line_directly();
            //$//init_Telit_GPIO();
            Ftp_activateGprs();
            Modem_Power_ON_Sequence_Flag = false;
            Debug_Output1( 0, "Modem_Power_ON_Sequence_Flag=%d", Modem_Power_ON_Sequence_Flag);
        }
        else if (mdm_comm_status >= 4)
        {
            mdm_send_AT_command("AT+CFUN=5\r\n", rx_ans, sizeof(rx_ans)-1,DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);
            Debug_TextOut(0,"Check Activation Status");
/*          socketConnected = true;
            Debug_Output1( 0, "socketConnected_Skip", 0);
            break;
*/

            /* Check Activation Status */
            for ( j=0; j < 2; j++ )
            {
                Debug_TextOut(0,"Check AT#SGACT Activation Status");

                if( mdm_send_AT_command("AT#SGACT?\r", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES,(DEFAULT_TIME_OUT*10)) == MDM_ERR_NONE )
                {
                    if ( mdm_find_response(rx_ans, "#SGACT: 1,1" ) == TRUE )
                    {
                        break;
                    }
                    mdm_send_AT_command("AT#SGACT=1,1\r", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*50);
                }
                else
                {
                    if(j==1)
                    {
                        /*Is used to exit from any lock from Modem using AT#SSENDEXT,AT#FTPAPPEND operation VT*/
                        //$//telit_wakeup_sleep(TRUE);
                        DelayUs(100);
                        //$//telit_wakeup_sleep(FALSE); //Flow Changed, socket open is done outside for all packets except DFG, DFS
                        DelayUs(100);
                        mdm_send_AT_command("AT#SS\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);
                        mdm_send_AT_command("AT+COPS?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);

                        for(modem_uart_bytes_counter=0; modem_uart_bytes_counter<=1500; modem_uart_bytes_counter++)
                        {
                            //$//MAP_UART_transmitData(EUSCI_A0_BASE, '5');
                            DelayUs(1);
                        }
                        //$//MAP_UART_transmitData(EUSCI_A0_BASE, '\r');
                        DelayUs(1);
                        //$//MAP_UART_transmitData(EUSCI_A0_BASE, '\n');
                        DelayUs(1);
                        Debug_TextOut(0,"Dummy_Byte_Send_to_Modem_from_FTP_Section");
                    }
                }
            }

            /* Check UCP Socket connection */
            for (k = 0; k < 1; k++)
            {
                Debug_TextOut(0,"Check UCP Socket connection");

                result = mdm_send_AT_command("AT#SS\r", rx_ans, sizeof(rx_ans), DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);

                if(result != MDM_ERR_NONE) break;

                Debug_Output1( 0, "glMdmUart_bytes_recvd=%d", glMdmUart_bytes_recvd);

                for (count = 0; count < glMdmUart_bytes_recvd; count++)
                {
                    if ((0 == strncmp((const char *)&glMdmUart_recv_buf[count], "#SS: 1,1", strlen("#SS: 1,1")))||
                            (0 == strncmp((const char *)&glMdmUart_recv_buf[count], "#SS: 1,2", strlen("#SS: 1,2"))))
                    {
                        mdm_send_AT_command("AT+COPS?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);
                        socketConnected = true;
                        Debug_Output1( 0, "socketConnected", 0);
                        break;
                    }
                }

                if (socketConnected)
                {
                    Debug_Output1( 0, "socketConnected exit loop", 0);
                    Modem_Recovery_Flag = false;
                    glComm_failure_count = 0;
                    DO_IP_SYNC = TRUE;
                    mdm_comm_status = 5;    // socket opened
                    break;
                }

                //$//sprintf (command, "AT#SD=1,1,%d,\"%s\",%d,%d,1\r", glSystem_cfg.UDP_server_port, (char*)glSystem_cfg.UDP_server_ip, 0 , 8181);//LOB:22/12/2015
                result = mdm_send_AT_command( command, rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT * 1200);
                //////////////////////////////////////////////////////////////
                mdm_send_AT_command("AT#SS\r", rx_ans, sizeof(rx_ans), DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);

                Debug_Output1( 0, "glMdmUart_bytes_recvd=%d", glMdmUart_bytes_recvd);

                for (count = 0; count < glMdmUart_bytes_recvd; count++)
                {
                    if ((0 == strncmp((const char *)&glMdmUart_recv_buf[count], "#SS: 1,1", strlen("#SS: 1,1")))||
                            (0 == strncmp((const char *)&glMdmUart_recv_buf[count], "#SS: 1,2", strlen("#SS: 1,2"))))
                    {
                        mdm_send_AT_command("AT+COPS?\r\n", rx_ans, sizeof(rx_ans)-1, DEFAULT_RETRIES, DEFAULT_TIME_OUT*10);
                        socketConnected = true;
                        Debug_Output1( 0, "socketConnected", 0);
                        break;
                    }
                }

                if (socketConnected)
                {
                    Debug_Output1( 0, "socketConnected exit loop", 0);
                    Modem_Recovery_Flag = false;
                    glComm_failure_count = 0;
                    DO_IP_SYNC = TRUE;
                    mdm_comm_status = 5;    // socket opened
                    break;
                }
                ///////////////////////////////////////////////////////////////
            }

            if (!socketConnected)
            {
                mdm_comm_status = 0;
            }

            if (mdm_comm_status >= 4)
            {
                Debug_Output1( 0, "mdm_comm_status exit loop", 0);
                Debug_Output1( 0, "mdm_comm_status=%d", mdm_comm_status);
                return mdm_comm_status;
            }
        }

        Debug_Output1( 0, "mdm_comm_status=%d", mdm_comm_status);

/*        if (mdm_comm_status >= 4)
        {
            Debug_Output1( 0, "mdm_comm_status exit loop", 0);
            break;
        }*/
    }

    return mdm_comm_status;
}

uint8_t Ftp_connectServer(void)
{
    uint16_t        j;
    uint8_t         ret        = SOCERR_NONE;
    uint8_t         result     = MDM_ERR_NONE;
    uint16_t         i;
    uint8_t command = 0;
    uint16_t rxByte = 0;
    bool okStringReceived = false;

    /*if(glSystem_cfg.Disable_GPRS_Comms_Coin_Only == TRUE )//3oct18
    {
        return ret;
    }
    else*/
    {
        //$//telit_wakeup_sleep(FALSE);

        Debug_Output1( 0, "mdm_comm_status=%d", mdm_comm_status );

        /* Close UDP Socket, If in Connect State */
        if (mdm_comm_status > 4)
        {
/*            glClose_socket = TRUE;
            telit_sock_close();*/
            mdm_comm_status = 4;
        }

        if ((ERROR_MODEM_RESPONSE == transparentModeErrorCode) || (mdm_comm_status == 0)) // To reinit Modem If MSP recieves as mdm_comm_status = 0
        {
            if (ERROR_MODEM_RESPONSE == transparentModeErrorCode)
            {
                Debug_TextOut(0,"FTP: Re-Initializing Modem");
                mdm_comm_status = 0;
            }
            Modem_Power_ON_Sequence_Flag = true;
            Debug_Output1( 0, "Modem_Power_ON_Sequence_Flag=%d", Modem_Power_ON_Sequence_Flag);
            //$//telit_init();
            Ftp_activateGprs();
            Modem_Power_ON_Sequence_Flag = false;
            Debug_Output1( 0, "Modem_Power_ON_Sequence_Flag=%d", Modem_Power_ON_Sequence_Flag);
        }
        /*else
        {
            transparentModeErrorCode = ERROR_NONE;
        }*/

        transparentModeErrorCode = ERROR_NONE;

        /* Connect to FTP Server */
        if (mdm_comm_status < 5)
        {
            Debug_TextOut(0,"FTP: Connect FTP Server");

            if (4 == mdm_comm_status)
            {
                Debug_TextOut(0, "FTP: Gprs Activated State");
            }

            //$//watchdog_pat();
            ret = Ftp_openSession();
        }
        else    // restore the socket if it is already open
        {
            Debug_TextOut(0,"FTP: FTP Server Busy");
            ret = MDM_FTP_BUSY;
        }

        return ret;
    }
}

uint8_t Ftp_enterCommandMode(char *fileName,uint8_t commandMode,uint8_t number)
{
    uint8_t status = MDM_ERR_NONE;
    char cmdBuffer[200];
    uint16_t count;

    ////DelayMs(200);

    for (count = 0; count < 2; count++)    //TBD //12_04_2021
    {
		//sprintf(cmdBuffer, "AT#FTPPUT=\"xmosfile.jpeg\",1\r\n", fileName); //COMMAND MODE
        //sprintf(cmdBuffer, "AT#FTPPUT=\"%d%s\",%d\r\n", number,fileName,commandMode); //COMMAND MODE
        sprintf(cmdBuffer, "AT#FTPPUT=\"%s\",%d\r\n", fileName,commandMode); //COMMAND MODE
		status = Ftp_sendATcommand((const char *)cmdBuffer, rx_ans, sizeof(rx_ans)-1, 1, 10000);

        if (MDM_ERR_NONE != status)
        {
            Debug_Output1( 0, "Error Send File FTP PUT(%d)", status);
            //printSharp16x24("Error Send File FTP PUT", 50, 0, 0);
            //LCD_Top_Message_String = TRUE;
            Ftp_closeSession();
            return status;
        }
        else
        {
            break;
        }

        DelayMs(500);
    }

    return status;
}

#if 0
uint8_t Ftp_enterDataMode(char *fileName)
{
    uint8_t status = MDM_ERR_NONE;
    char cmdBuffer[200];

    sprintf(cmdBuffer, "AT#FTPPUT=\"%s\",0\r\n", fileName); //DATA MODE
    status = Ftp_sendATcommand((const char *)cmdBuffer, rx_ans, sizeof(rx_ans)-1, 1, 5000);

    if (MDM_ERR_NONE != status)
    {
        Debug_Output1( 0, "Error Send File FTP PUT(%d)", status);
        Ftp_closeSession();
        return status;
    }

    return status;
}
#endif

uint8_t Ftp_sendPacketData(uint8_t packetNumber, uint8_t numPackets, uint8_t *data, uint16_t size)
{
    uint8_t status = MDM_ERR_NONE;
    char cmdBuffer[200];
    uint32_t timeout = 0;
	uint32_t v = 0;
	//if ((1 == numPackets) && (size <= 1500))
    if ((packetNumber == numPackets) && (size <= 1500))		//TBD	//changed on 10_07_2021
    {
        //if (iLastPacketDelay > 0)
        {
        //    DelayMs(iLastPacketDelay);
			DelayMs(2000);
        }
    }
    else
    {
        //if (iInterPacketDelay > 0)
		if((packetNumber % 10) == 0)
        {
        //    DelayMs(iInterPacketDelay);
			Debug_Output1(0, "((packetNumber % 10) == 0)(%d)", packetNumber);
			DelayMs(200);
        }
    }

	if (imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
	{
        //close(fd);
		Ftp_closeSession();
        return MDM_SECOND_SENSOR_TRIGGER_DETECTED;
	}	
    ////Debug_Output1(0, "Append File(%d)", packetNumber);

    /* Handle single packet file with size 1500 or less */
    /*if (((1 == packetNumber) && (1 == numPackets)) && (size <= 1500))
    {
        printSharp16x24("Append File FTP", 50, 0, 0);
        LCD_Top_Message_String = TRUE;
        sprintf(cmdBuffer, "AT#FTPAPPEXT=%d,0\r\n", size);
    }
    else*/
    {
        //sprintf(cmdBuffer, "AT#FTPAPPEXT=%d,%d\r\n", size, ((packetNumber == numPackets) ? 1 : 0));
		sprintf(cmdBuffer, "AT#FTPAPPEXT=%d,%d\r\n", size, ((packetNumber == numPackets) ? 1 : 0));
    }

    status = Ftp_sendATcommand((const char *)cmdBuffer, rx_ans, sizeof(rx_ans)-1, 1, 5000);

    if (MDM_SEND_DATA != status)
    {
        //printSharp16x24("Error Append File FTP", 50, 0, 0);
        //LCD_Top_Message_String = TRUE;
        Debug_Output1( 0, "Error Append File FTP APPEXT(%d)", status);
        Ftp_closeSession();
        return status;
    }
	else
	{
		//Debug_Output1( 0, "Append File Success FTP APPEXT(%d)", status);
	}

    //Debug_Output1(0, "TX(%d): ", size);   //TBD   //--to be Removed
    //LogManager_printByteArrayHexString(data, size);   //TBD   //--to be Removed
	
	//rtos_printf("before glMdmUart_bytes_recvd = %d\n", glMdmUart_bytes_recvd);
	//rtos_printf("before glMdmUart_recv_buf = %s\n", glMdmUart_recv_buf);
	
	memset(glMdmUart_recv_buf,0,GPRS_GEN_BUF_SZ_VLARGE);
	glMdmUart_bytes_recvd = 0;
	//delay_milliseconds(10);
	
	//rtos_printf("before 2 glMdmUart_bytes_recvd = %d\n", glMdmUart_bytes_recvd);
	//rtos_printf("before 2 glMdmUart_recv_buf = %s\n", glMdmUart_recv_buf);
	
    /* Send Packet Data */
    ////Modem_out_UART((const char *)data, (size+3));
	//printstr("\r");
	//printstr("\n");
	//tx_msg[0] = 'A';
	
	rtos_uart_tx_write(uart_tx_ctx, data, size);
	
	////for(int kt =0;kt<=(size+3);kt++)
	{
		//#if ON_TILE(0) 
			//while(Modem_TX_Busy==TRUE);
		//#endif
		//DelayUs(1);
		//UART_transmitData_TILE_0(0, data[kt]);
		////int state = rtos_osal_critical_enter();
		{
			////UART_transmitData_TILE_0(0, data[kt]);
		}
		////rtos_osal_critical_exit(state);
		/*if(packetNumber == 1)
		{
			printhex(data[kt]);
			printstr(" ");
		}*/
		//uart_tx(&uart, data[kt]);
		
	}
	//printstr("\r");
	//printstr("\n");
	//uart_tx(&uart, '\r');
	//uart_tx(&uart, '\n');
	//UART_transmitData_TILE_0(0, '\r');
	//UART_transmitData_TILE_0(0, '\n');
	//Modem_out_UART(&fontGIF4, 1500);
	//status = Ftp_sendATcommand((const char *)data, rx_ans, sizeof(rx_ans)-1, 1, 5000);
    
	/* Last Packet Takes More Time. */
    if (packetNumber == numPackets)
    {
        //timeout = 4000000;
        timeout = 4000;
    }
    else
    {
        //timeout = 200000;
        timeout = 3000;
    }

    /* Wait For Response */
    /*while ((0 != strncmp((const char *)&glMdmUart_recv_buf[6], "#FTPAPPEXT:", strlen("#FTPAPPEXT:"))) && (timeout > 0))
    {
        timeout--;
    }*/

    uint16_t i;
    uint16_t rxByte = 0;
    bool modmeResponseReceived = false;

    char rxCmpString[50];
    sprintf(rxCmpString, "#FTPAPPEXT: %d\r\nOK\r\n", size);
	//DelayMs(20);
#if 1
    for (i = 0; i < timeout; i++)
    {
        if((glMdmUart_bytes_recvd > 0)&&(glMdmUart_recv_buf[0]!='\0'))
        {
			////DelayMs(200);
            /*for (rxByte = 0; rxByte < glMdmUart_bytes_recvd; rxByte++)
            {
                //if (0 == strncmp((const char *)&glMdmUart_recv_buf[rxByte], "#FTPAPPEXT:", strlen("#FTPAPPEXT:")))
                if (0 == strncmp((const char *)&glMdmUart_recv_buf[rxByte], rxCmpString, strlen(rxCmpString)))
                {
                    //Debug_TextOut(0, "#FTPAPPEXT: Response Received");
                    DelayMs(0);
                    modmeResponseReceived = true;
                    break;
                }
				if (mdm_find_response (glMdmUart_recv_buf, "#FTPAPPEXT:" ) == TRUE )
				{
                    DelayMs(10);
                    modmeResponseReceived = true;
                    break;
				}
            }*/
			if (mdm_find_response (glMdmUart_recv_buf, "#FTPAPPEXT:" ) == TRUE )
				{
					////Debug_TextOut(0, "glMdmUart_recv_buf, #FTPAPPEXT:");
                    if(!modmeResponseReceived)DelayMs(5);
                    modmeResponseReceived = true;
				}
        }

        if (modmeResponseReceived)
        {
            ////Debug_Output1(0, "RX(%d): ", glMdmUart_bytes_recvd); //TBD
			status = MDM_ERR_NONE;
            //LogManager_printByteArrayHexString(glMdmUart_recv_buf, glMdmUart_bytes_recvd); //TBD
            break;
        }

        DelayMs(5);
        //DelayMs(5);    //TBD //12_04_2021
    }
#endif
	//modmeResponseReceived = true;
	//status = MDM_ERR_NONE;
	//rtos_printf("after glMdmUart_bytes_recvd = %d\n", glMdmUart_bytes_recvd);
	//rtos_printf("after glMdmUart_recv_buf = %s\n", glMdmUart_recv_buf);
    //Debug_Output1(0, "timeout: %d", timeout);
    //Debug_Output1(0, "timeout: %d", (timeout - i));

    if (packetNumber == numPackets)
    {
        Debug_Output1(0, "Last Packet Appended", 0);
        //printSharp16x24("Last Packet Appended", 50, 0, 0);
        //LCD_Top_Message_String = TRUE;
        //status = Ftp_closeSession();
    }

    return status;
}

void find_image_file (void)
{
/////////////DIRECTORY SEARCH////////////////////////
    FATFS fs;
    FRESULT res;
    char buff[256];


    //res = f_mount(&fs, "", 1);
    //if (res == FR_OK) 
    {
        strcpy(buff, "/");
        res = scan_files(buff);
    }
/////////////////////////////////////////////////////
    
////////////////FILE SEARCH//////////////////////////
    FRESULT fr;     /* Return value */
    DIR dj;         /* Directory object */
    FILINFO fno;    /* File information */

    fr = f_findfirst(&dj, &fno, "", "*.*"); /* Start to search for photo files */

    while (fr == FR_OK && fno.fname[0]) {         /* Repeat while an item is found */
        printf("%s\n", fno.fname);                /* Print the object name */
        fr = f_findnext(&dj, &fno);               /* Search for next item */
    }
    Debug_Output2(0,"File close %d , %s\n", fr,fno.fname);
    f_closedir(&dj);
//////////////////////////////////////////////////////


///////////////////TXT ADDING ////////////////////////
	/*******************FILE CREATE***/
   	FIL fil;        /* File object */
    	char line1[100] = "FOR TESTING NEW FILE THIS DATA BEING ADDED"; /* Line buffer */
	UINT bw;         /* File read/write count */
	char filename[200];
#if 0		
	sprintf(filename, "/flash/fs/mess%d.txt", 1);
	
	rtos_printf("file name = %s\n", filename);
	
	fr = f_open(&fil, (const TCHAR *)filename, FA_CREATE_NEW | FA_WRITE);
	
	rtos_printf("fr = f_open(&fil, %d)\n", fr);
	
	fr = f_write(&fil, line1, 100, &bw);           /* Write it to the destination file */
	
	rtos_printf("fr = f_write(&fil, %d)\n", fr);
	
	f_close(&fil);
#endif	
//////////////////////////////////////////////    
}

//#pragma stackfunction 2000
int FileOpen(char *FTPFilename,int ImageSeqNum, chanend_t txCmd)
{
	unsigned startTimer = 0;
	unsigned currentTimer = 0;
	char byteString[200];
	char tempPrintString[300];
	char fname[100];
	
	Debug_TextOut(0,"FileOpen\n");
    static FIL image_file;
	uint8_t *FTPImagFileBufferptr;//,*FTPImagFileBufferptr1;//,*FTPImagFileBufferptr2;
    char tempString[200];
    uint32_t uImageSize;
    uint8_t iNum_of_Packets=0;
    uint16_t pktsize;
	FRESULT result;
	UINT bytes_read = 0;
	

	//sprintf(tempString,"tempstring:%s",FTPFilename);
	sprintf(tempString,"%s",FTPFilename);
	
	printf("\nFile name at fopen is ===================================>> %s\n",tempString);
	
		////////////  Get Seq Number from File Name ////////////////////////
	if(strlen(FTPFilename)>43){
					sprintf(fname,"%s",FTPFilename);// IMG_FL_00S8028_32018_20230220_080835_01.jpeg
					sprintf(SF_Filename,"%s",FTPFilename);
					printf("\nFile name before parsing is ===================================>> %s\n",tempString);
					char * token = strtok(fname, "_");
				   // printf( " %s\n", token );
				    token = strtok(NULL, "_");
				   // printf( " %s\n", token );
				    token = strtok(NULL, "S");
				    printf( " %s\n Token for seq number", token );
				    capture_sequence_number = atoi(token);
				    Debug_Output1(0,"Sequence Number is  %d\n", capture_sequence_number);
				   printf("\nSequence Number is  %d\n", capture_sequence_number);
					
					/////////////////////End////////////////////////////////////////////
					
					
					//////////  Get Capture number /////////////////////////////////////
					token = strtok(NULL, "_");
					printf( "File name parsing for capture num %s\n", token );
					token = strtok(NULL, "_");
					printf( "File name parsing for capture num %s\n", token );
					token = strtok(NULL, "_");
					printf( "File name parsing for capture num %s\n", token );
					token = strtok(NULL, "_");
					printf( "File name parsing for capture num %s\n", token );
					token = strtok(NULL, ".");
					printf( "File name parsing for capture num %s\n", token );
					capture_number = atoi(token);
					Debug_Output1(0,"capture_number Number is  %d\n", capture_number);
                   }
   else
				{
					capture_number = 1;
					capture_sequence_number = 1;
					
					
				}

printf("\nFile name after parsing %s   Capture sequence number is %d capture number is %d\n\n",FTPFilename,capture_sequence_number,capture_number);
	
	///////////////////End//////////////////////////////////////////////
	

    /* Initialize filesystem  */
    rtos_fatfs_re_init(qspi_flash_ctx);
    
    Debug_TextOut(0,"In fopen\n");
    sprintf(tempPrintString, "\n In the function FileOpen\n");
			                Debug_TextOut(0, tempPrintString);
   printf("In fopen  \n");
    
	
	if (image_file.obj.fs == NULL) 
	{
		Debug_TextOut(0,"Opening CAMERA IMAGE file\n");
		Debug_TextOut(0,tempString);
		result = f_open(&image_file, tempString, FA_READ);
		//result = f_open(&image_file, "/flash/image/iamgetestpicture.jpg", FA_READ);
		Debug_Output1(0,"CAMERA IMAGE  result is %d\n", result);
		if(result == NULL) result = 1;
	}

    if(result == NULL)
    {
       Debug_TextOut(0,"	File Open Failed.");
       //if (1 == iSomLogEnable)
	   //   log0(1, 0x85, "	File Open Failed.");
       return -1;
    }
    else
    {
        uImageSize = f_size(&image_file);
		
        Debug_TextOut(0,"	File Open Success.");
        Debug_Output1(0,"	File Size: %d", uImageSize);
        //if (1 == iSomLogEnable)
		//{
		//	log0(1, 0x87, "	File Open Success.");
		//	log1(1, 0x87, "	File Size: ", uImageSize);
		//}
		
		#if 0
		if (sendSomLogFile)
		{
			if ((uImageSize > MAXIMUM_SOM_LOG_FILE_SIZE))
			{
				qDebug()<<"SOM LOG file size Exceeds Max. Limit. "<<uImageSize;
				if (1 == iSomLogEnable)
					log1(1, 0x87, "SOM LOG file size Exceeds Max. Limit.", uImageSize);

				return -2;
			}
		}
		else
		{			
			if ((uImageSize <= FTP_MINIMUM_IMAGE_FILE_SIZE) || (uImageSize >= FTP_MAXIMUM_IMAGE_FILE_SIZE))
			{
				qDebug()<<"Image file size is NOT allowed to transfer: "<<uImageSize;
				if (1 == iSomLogEnable)
					log1(1, 0x87, "Image file size is NOT allowed to transfer: ", uImageSize);

				return -2;
			}
		}
		#endif
		
		//#if( configAPPLICATION_ALLOCATED_HEAP == 1 )
		//#if XCOREAI_EXPLORER
		//__attribute__((section(".ExtMem_data"))) uImageSize 1,60,872
		//#endif
#if 0		
		vTaskDelay( pdMS_TO_TICKS( 100 ) );
		debug_printf("\t1Minimum heap free: %d\n", xPortGetMinimumEverFreeHeapSize());
		debug_printf("\t1Current heap free: %d\n", xPortGetFreeHeapSize());
		vTaskDelay( pdMS_TO_TICKS( 100 ) );
		
		FTPImagFileBufferptr1 = pvPortMalloc(uImageSize);
		
		vTaskDelay( pdMS_TO_TICKS( 100 ) );
		debug_printf("\t2Minimum heap free: %d\n", xPortGetMinimumEverFreeHeapSize());
		debug_printf("\t2Current heap free: %d\n", xPortGetFreeHeapSize());
		vTaskDelay( pdMS_TO_TICKS( 100 ) );		
		
			printf(" vPortFree(FTPImagFileBufferptr1)\n");
			vTaskDelay( pdMS_TO_TICKS( 100 ) );
            vPortFree(FTPImagFileBufferptr1);
			vTaskDelay( pdMS_TO_TICKS( 100 ) );
#endif		
		
		//PS:23032023 added uTicksDelay_while_Heap_memory_allocation variable for changing delay at 4 locations
		uint16_t uTicksDelay_while_Heap_memory_allocation = 100;
		//uint16_t uTicksDelay_while_Heap_memory_allocation = 50;
		
		vTaskDelay( pdMS_TO_TICKS( uTicksDelay_while_Heap_memory_allocation ) );
		debug_printf("\t1Minimum heap free: %d\n", xPortGetMinimumEverFreeHeapSize());
		debug_printf("\t1Current heap free: %d\n", xPortGetFreeHeapSize());
		vTaskDelay( pdMS_TO_TICKS( uTicksDelay_while_Heap_memory_allocation ) );
		 
		// Debug_TextOut(0,"1001\n");
		FTPImagFileBufferptr = pvPortMalloc(uImageSize);
		/* Debug_TextOut(0,"In fopen  Allocated buffer for FTP\n");
		 sprintf(tempPrintString, "In fopen  Allocated buffer for FTP\n");
			                Debug_TextOut(0, tempPrintString);
	    printf("In fopen  Allocated buffer for FTP\n");
		 */
		
		vTaskDelay( pdMS_TO_TICKS( uTicksDelay_while_Heap_memory_allocation ) );
		debug_printf("\t2Minimum heap free: %d\n", xPortGetMinimumEverFreeHeapSize());
		debug_printf("\t2Current heap free: %d\n", xPortGetFreeHeapSize());
		vTaskDelay( pdMS_TO_TICKS( uTicksDelay_while_Heap_memory_allocation ) );
       
       if (NULL == FTPImagFileBufferptr)
        {
			f_close( &image_file);
			if(uImageSize < 10)
			{
                  char tempString[200];
                  sprintf(tempString,"%s", FTPFilename);
                        
				  int iRet = f_unlink(tempString);
                        if (iRet == 0)
                        {
                            Debug_Output1(0,"	File Delete Success.(%s)",tempString);
                        }
                        else {
                            Debug_TextOut(0,"	Failed to Delete File.");
                        }				
			}
						
            Debug_TextOut(0,"Memory not allocated.\n");
			//if (1 == iSomLogEnable)
			//	log0(1, 0x87, "Memory not allocated.");
            return -3;
            exit(0);
        }
        else
        {

            // Memory has been successfully allocated
            //printf("Memory successfully allocated using malloc. start\n");
			//printf("At fread3262: uImageSize:%d, bytesread: %d", (int)uImageSize, bytes_read);
			//Debug_TextOut(0,"2001\n");
            result = f_read(&image_file, FTPImagFileBufferptr, uImageSize, &bytes_read );
			//printf("At fread3264: uImageSize:%d, bytesread: %d, result: %d", (int)uImageSize, bytes_read, result);
            Debug_TextOut(0,"In fopen after f_read()\n");
            sprintf(tempPrintString, "In fopen after f_read()\n");
			Debug_TextOut(0, tempPrintString);
		
			//qDebug()<<"Number of bytes read from image file:"<<count;
            f_close( &image_file);
            
            num_images = num_images + 1;
            
            /////// Calculating  HASH and printing every 10th byte//////Longitudinal Redundancy
            #if 1
	         uint8_t lrc = 0x00;
	         uint8_t lrc1 = 0x00;
	         int i = 0;
	         int j = 0;
	          printf("\nImage size is %d\n",(int)uImageSize);
	        // printf("\n Value of every 1000 data in image is\n");
	          for(i=0;i<(int)uImageSize; i++)
	          { 
				  lrc = (lrc + FTPImagFileBufferptr[i]) & 0xFF;
				  j++;
				  //PS:20032023 commented below image data printing code
				  /*
				  if(j== 1000){ j = 0; printf(" %02X ", FTPImagFileBufferptr[i]);
					  lrc1 =  lrc;
					  lrc1 = ((lrc1 ^ 0xFF) + 1) & 0xFF; printf(" lrc=>%2X",lrc1);
					  lrc = 0;
					 }*/
					 
                     //if(((int)uImageSize - i)==300)	printf("\n\n Last bytes\n\n");
					//if(((int)uImageSize - i)<300)	printf(" %2x",FTPImagFileBufferptr[i]); 				  
				  
			  }
			  
			// lrc =   ((lrc ^ 0xFF) + 1) & 0xFF;
			 
			// printf("\n LRC is %2x\n", lrc); 
			#endif           
            
            ////////////////////////////////////////////////////////////     
            
            /////////////////////SF/////////////////////////////////////
            //capture_sequence_number = get_seq_num();
             printf("In fopen after f_read()\n");
             printf("\n	FTP: Shared flash write function enter \n");
             Debug_TextOut(0,"\nFTP: Shared flash write function enter\n");
             sprintf(tempPrintString, "\nFTP: Before Shared flash func Image siz %lu  Capture Seq Number ==>%d<===\n",uImageSize,capture_sequence_number);
			 Debug_TextOut(0, tempPrintString);           
             printf("\nFTP: Before Shared flash func Image siz %lu  Capture Seq Number ==>%d<===\n",uImageSize,capture_sequence_number);  
			 printf("\n\nValue of image buffer\n");
			 
			 sprintf(tempPrintString, "\nValue of image buffer\n");
			 printf("\nFirst 900 bytes of Image data after fread\n");
			// int32_t i;
			 //ps:27022023 
			//for(i=0;i<uImageSize;i++)
				//PS:20032023 commented below data printing code
			/*for(i=0; i<900; i++)
			
			 {	 printf("%02X ",(uint8_t)FTPImagFileBufferptr[i]);}*/
			  //Debug_TextOut(0, tempPrintString);
             FTPImagFileBufferptr_sf = FTPImagFileBufferptr;
			 
			 printf("FTPImagFileBufferptr_sf = %lu \n",FTPImagFileBufferptr_sf);			 
             
			 sf_write(FTPImagFileBufferptr_sf,SF_Filename,uImageSize,capture_sequence_number,capture_number);
             //  sf_write_test();
            // shared_flash_write();
             printf("\n	FTP: Shared flash write function exit \n");
             Debug_TextOut(0,"\nFTP: Shared flash write function exit \n");
             printf("Memory successfully allocated using malloc. end = %d = bytes %d\n",result,bytes_read);
             ///////////////////////////////////////////////////////////
             
           	///Added by PS:21022023
			//if(uImageSize < 10)
			{
                  char tempString[200];
                  sprintf(tempString,"%s", SF_Filename);
                        
				  /*int iRet = f_unlink(tempString);
                        if (iRet == 0)
                        {
                            Debug_Output1(0,"	File Delete Success.(%s)",tempString);
                        }
                        else {
                            Debug_TextOut(0,"	Failed to Delete File.");
                        }	*/			
			}
			int iRet = 0;

}

///////
//#if 0

//#endif
            }
			printf(" vPortFree(FTPImagFileBufferptr)\n");
			vTaskDelay( pdMS_TO_TICKS( 100 ) );
            vPortFree(FTPImagFileBufferptr);
			vTaskDelay( pdMS_TO_TICKS( 100 ) );
		}
	//}
//}

//ImageInfo_t * unsafe imageCaptureTile0ConfigPara ;

static void ftp_ctrl_t0(chanend_t txCmd)
{
	uint8_t  result = 0;

	rtos_printf("ftp_ctrl_t0 task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());
	uint8_t j = 0;
	//unsafe {imageCaptureTile0ConfigPara = &imageCaptureTile0;}
	
	imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = 1;
	imageCaptureTile0.vars.transparentReadyStatus = 0;
	
	for(;;)
	{
		DelayMs(500);
		if((bootUpFlag == 1) && (bootUpCount > 5) && ((bootUpCount % 5) == 0)) 
			{
				//If bootUp Flag become more 5Second then
				//we can say XMOS rebooted due to some issue ,
				//So MSP Can Turn off XMOS by sending End Of Transparent Mode
				rtos_printf("HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE from Tile 0\n");
				InterTileCommTile0_sendPacketToHostMcuViaTile1(txCmd, HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE);				
			}
		if(bootUpFlag == 1)bootUpCount++;	
#if 1				
		if((!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)/* && (imageCaptureTile0.vars.transparentReadyStatus == 1)*/) 
		{
			
			char tempPrintString[300];
			uint32_t uImageSize = 12345;
			sprintf(tempPrintString, "	ENTERED TO FTP PROCESS111: ");
			Debug_TextOut(0, tempPrintString);
			uint16_t txStringLength = 0;
			currentFileHeaderInfoTile0[txStringLength++] = 0;	/* File Number */
			strncpy(&currentFileHeaderInfoTile0[txStringLength], tempPrintString, 200); /* File Name */
			txStringLength += 200;
			currentFileHeaderInfoTile0[txStringLength++] = uImageSize >> 24; /* File Size */
			currentFileHeaderInfoTile0[txStringLength++] = uImageSize >> 16; /* File Size */
			currentFileHeaderInfoTile0[txStringLength++] = uImageSize >> 8; /* File Size */
			currentFileHeaderInfoTile0[txStringLength++] = uImageSize & 0xff; /* File Size */
			InterTileCommTile0_sendPacketToHostMcuViaTile1(txCmd, HOST_MCU_IMAGE_FILE_HEADER_MESSAGE);
								
			imageCaptureTile0.vars.ftpRunStatus = 1;
			////f_unmount ("");
			rtos_fatfs_re_init(qspi_flash_ctx);
			DelayMs(300);
			
			//for(int SendImage_Count=0;SendImage_Count < 4;SendImage_Count++)
			for(int SendImage_Count=0;SendImage_Count < 1;SendImage_Count++)
			{
				if(!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
				{
					/*	printf("\n\nXMOS SendImageFile_viaFTP**************************************************************************>>>>>>>>>>>>\n\n");
					//DelayMs(500);
					rtos_fatfs_re_init(qspi_flash_ctx);
					DelayMs(300);
					sprintf(tempPrintString, "\n Calling the function SendImageFile_viaFTP ===========>");
			                Debug_TextOut(0, tempPrintString);
					rtos_printf("**********SendImage_Count %d**********\n", SendImage_Count);*/
					SendImageFile_viaFTP(currentSession_global,txCmd);
				}
				if (imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
				{
					break;
				}				
			}

			rtos_printf("SendImageFile_viaFTP EXIT: %d\n", imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected);
			
			if(!imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected)
			{
				imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = 1;
				DelayMs(200);
				rtos_printf("HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE from Tile 0\n");
				InterTileCommTile0_sendPacketToHostMcuViaTile1(txCmd, HOST_MCU_END_OF_FILE_TRANSFER_MESSAGE);
				imageCaptureTile0.vars.exitLoopUponAnotherCameraDetected = 1;
				bootUpFlag = 1;
			}
			imageCaptureTile0.vars.ftpRunStatus = 0;
			imageCaptureTile0.vars.transparentReadyStatus = 0;
		}
#endif	
	}
}

void ftp_ctrl_create( UBaseType_t priority , chanend_t txCmd)
{	
	xTaskCreate(ftp_ctrl_t0, "ftp_ctrl_t0", portTASK_STACK_DEPTH(ftp_ctrl_t0), txCmd, tskIDLE_PRIORITY, &ftp_handler_task);			
	//xTaskCreate(modem_ctrl_t0, "modem_ctrl_t0", portTASK_STACK_DEPTH(modem_ctrl_t0), dev, priority, &modem_handler_task);	 //tskIDLE_PRIORITY		
}
