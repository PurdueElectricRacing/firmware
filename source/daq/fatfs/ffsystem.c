/*------------------------------------------------------------------------*/
/* A Sample Code of User Provided OS Dependent Functions for FatFs        */
/*------------------------------------------------------------------------*/

#include "ff.h"

#if FF_USE_LFN == 3 /* Use dynamic memory allocation */

/*------------------------------------------------------------------------*/
/* Allocate/Free a Memory Block                                           */
/*------------------------------------------------------------------------*/

#include <stdlib.h> /* with POSIX API */

void* ff_memalloc(/* Returns pointer to the allocated memory block (null if not enough core) */
                  UINT msize /* Number of bytes to allocate */
) {
    return malloc((size_t)msize); /* Allocate a new memory block */
}

void ff_memfree(
    void* mblock /* Pointer to the memory block to free (no effect if null) */
) {
    free(mblock); /* Free the memory block */
}

#endif // FF_USE_LFN /* Use dynamic memory allocation */

#if FF_FS_REENTRANT /* Mutal exclusion */
/*------------------------------------------------------------------------*/
/* Definitions of Mutex                                                   */
/*------------------------------------------------------------------------*/

#include "common/freertos/freertos.h"
static SemaphoreHandle_t Mutex[FF_VOLUMES + 1]; /* Table of mutex handle */

/*------------------------------------------------------------------------*/
/* Create a Mutex                                                         */
/*------------------------------------------------------------------------*/
/* This function is called in f_mount function to create a new mutex
/  or semaphore for the volume. When a 0 is returned, the f_mount function
/  fails with FR_INT_ERR.
*/
int ff_mutex_create(/* Returns 1:Function succeeded or 0:Could not create the mutex */
                    int vol /* Mutex ID: Volume mutex (0 to FF_VOLUMES - 1) or system mutex (FF_VOLUMES) */
) {
    Mutex[vol] = xSemaphoreCreateMutex();
    return (int)(Mutex[vol] != NULL);
}

/*------------------------------------------------------------------------*/
/* Delete a Mutex                                                         */
/*------------------------------------------------------------------------*/
/* This function is called in f_mount function to delete a mutex or
/  semaphore of the volume created with ff_mutex_create function.
*/
void ff_mutex_delete(/* Returns 1:Function succeeded or 0:Could not delete due to an error */
                     int vol /* Mutex ID: Volume mutex (0 to FF_VOLUMES - 1) or system mutex (FF_VOLUMES) */
) {
    vSemaphoreDelete(Mutex[vol]);
}

/*------------------------------------------------------------------------*/
/* Request a Grant to Access the Volume                                   */
/*------------------------------------------------------------------------*/
/* This function is called on enter file functions to lock the volume.
/  When a 0 is returned, the file function fails with FR_TIMEOUT.
*/
int ff_mutex_take(/* Returns 1:Succeeded or 0:Timeout */
                  int vol /* Mutex ID: Volume mutex (0 to FF_VOLUMES - 1) or system mutex (FF_VOLUMES) */
) {
    return (int)(xSemaphoreTake(Mutex[vol], FF_FS_TIMEOUT) == pdTRUE);
}

/*------------------------------------------------------------------------*/
/* Release a Grant to Access the Volume                                   */
/*------------------------------------------------------------------------*/
/* This function is called on leave file functions to unlock the volume.
*/
void ff_mutex_give(
    int vol /* Mutex ID: Volume mutex (0 to FF_VOLUMES - 1) or system mutex (FF_VOLUMES) */
) {
    xSemaphoreGive(Mutex[vol]);
}

#endif /* FF_FS_REENTRANT */
