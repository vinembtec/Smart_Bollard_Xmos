 
#ifndef RECORD_QUEUE_H_
#define RECORD_QUEUE_H_

/*** Include Files ***/

#include "host_mcu/host_mcu_types.h"


/*** Macro Definitions ***/

/*** Type Definitions ***/

#ifdef __cplusplus
extern "C" {
#endif


/*** Function Prototypes ***/

Status_t RecordQueue_addEntry(QueueEntry_t *entry);
QueueEntry_t * RecordQueue_getEntry(void);
Status_t RecordQueue_deleteEntry(QueueEntry_t *entry);
uint16_t RecordQueue_getNumEntries(void);


#ifdef __cplusplus
}
#endif


#endif /* RECORD_QUEUE_H_ */
