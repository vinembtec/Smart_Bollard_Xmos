
/*** Include Files ***/

#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "stdio.h"
#include "math.h"
#include "stdlib.h"
#include "record_queue.h"


/*** Macro Definitions ***/

//#define RECORD_QUEUE_SIZE    100


/*** Type Definitions ***/

/*** Function Prototypes ***/

/*** Variable Declarations ***/

static int8_t readIndex = 0;
static int8_t writeIndex = -1;
static QueueEntry_t recordQueue[RECORD_QUEUE_SIZE];

extern int iPrintDebugMessage;


/*** Function Definitions ***/

/* If Record Queue is FULL, process it to clear Queue */
Status_t RecordQueue_addEntry(QueueEntry_t *entry)
{
	if (NULL == entry)
	{
		return STATUS_ERROR;
	}

	if (-1 == writeIndex)
	{
		/* The queue is empty */
		writeIndex = 0;
	}
	else if (writeIndex == readIndex)
	{
		/* The queue is full */
		return STATUS_BUSY;
	}

	/* Compare New Entry with old records, if match found DO NOT ADD new entry */
	for (uint16_t count = 0; count < writeIndex; count++)
	{
		if (0 == memcmp(&recordQueue[count], entry, sizeof(recordQueue[count])))
		{
			return STATUS_RECORD_EXITS_IN_OLD_ENTRY;
		}
	}

	memcpy(&recordQueue[writeIndex], entry, sizeof(recordQueue[writeIndex]));

	writeIndex = ((writeIndex + 1) % RECORD_QUEUE_SIZE);

	return STATUS_OK;
}

QueueEntry_t * RecordQueue_getEntry(void)
{
	if (-1 == writeIndex)
	{
		/* The queue is empty */
		return NULL;
	}

	return (&recordQueue[readIndex]);
}

uint16_t RecordQueue_getNumEntries(void)
{
	if (-1 == writeIndex)
	{
		/* The queue is empty */
		return 0;
	}

	return ((uint16_t)(writeIndex - readIndex));
}

/* Call this function only when Queue is NOT empty */
Status_t RecordQueue_deleteEntry(QueueEntry_t *entry)
{
	if (NULL == entry)
	{
		/* The queue is empty */
		return STATUS_ERROR;
	}

	readIndex = ((readIndex + 1) % RECORD_QUEUE_SIZE);

	if (readIndex == writeIndex)
	{	
		/* The queue has just been emptied */
		writeIndex = -1;
		readIndex = 0;

		if (iPrintDebugMessage > 1)
		{
			printf("\r\n	Record Queue Emptied.");
		}
	}

	return STATUS_OK;
}

