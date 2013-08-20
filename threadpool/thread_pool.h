#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

/*---------------------------------------------------------------------------*/
/* FILE INCLUSION */
/*---------------------------------------------------------------------------*/

#include "task_queue.h"

/*---------------------------------------------------------------------------*/
/* MACRO DEFINITIONS */
/*---------------------------------------------------------------------------*/

/* N/A */

/*---------------------------------------------------------------------------*/
/* ENUMERATIONS */
/*---------------------------------------------------------------------------*/

/* N/A */

/*---------------------------------------------------------------------------*/
/* TYPE DECLARATIONS */
/*---------------------------------------------------------------------------*/

// forward declaration to hide implementation
typedef struct THREAD_POOL_DATA_STRUCT THREAD_POOL_DATA;

/*---------------------------------------------------------------------------*/
/* FUNCTION PROTOTYPES */
/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

// this should only be called once per task queue
THREAD_POOL_DATA*   CreateThreadPool(
	unsigned int numThreads,
	TASK_QUEUE_DATA *taskQueueData,
	void* (*threadInit)(),
	void (*threadPostInit)(void* threadContext),
	void (*threadDestory)(void* threadContext));

// this should only be called once per thread pool and prior to destorying the associated task queue
void                DestroyThreadPool(THREAD_POOL_DATA *threadPool);

#ifdef __cplusplus
}
#endif

/*---------------------------------------------------------------------------*/
/* OBJECT DECLARATIONS */
/*---------------------------------------------------------------------------*/

#ifndef _THREAD_POOL_C_

/* N/A */

#endif /* _THREAD_POOL_C_ */

/*****************************************************************************/

#endif /* _THREAD_POOL_H_ */