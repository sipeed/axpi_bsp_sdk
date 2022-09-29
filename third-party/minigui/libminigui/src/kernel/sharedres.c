///////////////////////////////////////////////////////////////////////////////
//
//                          IMPORTANT NOTICE
//
// The following open source license statement does not apply to any
// entity in the Exception List published by FMSoft.
//
// For more information, please visit:
//
// https://www.fmsoft.cn/exception-list
//
//////////////////////////////////////////////////////////////////////////////
/*
 *   This file is part of MiniGUI, a mature cross-platform windowing
 *   and Graphics User Interface (GUI) support system for embedded systems
 *   and smart IoT devices.
 *
 *   Copyright (C) 2002~2020, Beijing FMSoft Technologies Co., Ltd.
 *   Copyright (C) 1998~2002, WEI Yongming
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   Or,
 *
 *   As this program is a library, any link to this program must follow
 *   GNU General Public License version 3 (GPLv3). If you cannot accept
 *   GPLv3, you need to be licensed from FMSoft.
 *
 *   If you have got a commercial license of this program, please use it
 *   under the terms and conditions of the commercial license.
 *
 *   For more information about the commercial license, please refer to
 *   <http://www.minigui.com/blog/minigui-licensing-policy/>.
 */
/*
** sharedres.c: Load and init shared resource.
**
** Create date: 2000/12/22
*/

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "common.h"

#ifdef _MGRM_PROCESSES

#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/mman.h>

#include "minigui.h"
#include "gdi.h"
#include "window.h"
#include "cliprect.h"
#include "gal.h"
#include "internals.h"
#include "cursor.h"
#include "icon.h"
#include "menu.h"
#include "ial.h"
#include "ourhdr.h"
#include "client.h"
#include "server.h"
#include "sharedres.h"
#include "misc.h"
#include "sysres.h"

#define SHM_PARAM 0644
#define SEM_PARAM 0666

// define to use mmap instead of shared memory.
#undef  _USE_MMAP
// #define _USE_MMAP 1

#ifdef _MGHAVE_CURSOR

#define CURSORSECTION   "cursorinfo"

#ifdef _MGSCHEMA_COMPOSITING

static BOOL LoadCursorRes (void)
{
    int number;
    int i;
    char szValue [12];

    if (GetMgEtcValue (CURSORSECTION, "cursornumber", szValue, 10) < 0) {
        _ERR_PRINTF ("Failed to get number of system cursors\n");
        return FALSE;
    }

    number = atoi (szValue);
    if (number < 0) {
        _ERR_PRINTF ("Bad number of system cursor: %d\n", number);
        return FALSE;
    }

    number = number < (MAX_SYSCURSORINDEX + 1) ? number : (MAX_SYSCURSORINDEX + 1);
    // realloc for shared resource
    mgSharedRes = realloc (mgSharedRes, mgSizeRes + number * (sizeof (HCURSOR)));
    if (mgSharedRes == NULL) {
        _ERR_PRINTF ("Failed to realloc shared memory for system cursor.\n");
        return FALSE;
    }

    // set cursor number
    ((PG_RES)mgSharedRes)->csrnum = number;
    mgSizeRes += number * (sizeof (HCURSOR));

    for (i = 0; i < number; i++) {
        ((PG_RES)mgSharedRes)->sys_cursors[i] = NULL;
    }

    for (i = 0; i < number; i++) {
        PCURSOR tempcsr;

        if (!(tempcsr = sysres_load_system_cursor (i)))
            goto error;

        ((PG_RES)mgSharedRes)->sys_cursors[i] = tempcsr;
    }

    return TRUE;

error:
    for (i = 0; i < number; i++) {
        PCURSOR tempcsr;
        tempcsr = ((PG_RES)mgSharedRes)->sys_cursors[i];
        if (tempcsr) {
            GAL_FreeCursorSurface (tempcsr->surface);
            mg_slice_delete (CURSOR, tempcsr);
            ((PG_RES)mgSharedRes)->sys_cursors[i] = NULL;
        }
    }

    return FALSE;
}

#else /* _MGSCHEMA_COMPOSITING */

extern unsigned int __mg_csrimgpitch;
static BOOL LoadCursorRes (void)
{
    int number;
    int i;
    PCURSOR tempcsr;
    char *temp;
    char szValue [12];

    __mg_csrimgsize = GAL_GetBoxSize (__gal_screen, CURSORWIDTH, CURSORHEIGHT, &__mg_csrimgpitch);

    if (GetMgEtcValue (CURSORSECTION, "cursornumber", szValue, 10) < 0)
        goto error;

    number = atoi (szValue);
    if (number < 0) goto error;
    number = number < (MAX_SYSCURSORINDEX + 1) ? number : (MAX_SYSCURSORINDEX + 1);

    // realloc for shared resource
    mgSharedRes = realloc (mgSharedRes, mgSizeRes + __mg_csrimgsize +
                    number * (sizeof (HCURSOR) + sizeof (CURSOR) + 2*__mg_csrimgsize));
    if (mgSharedRes == NULL) {
        perror ("realloc shared memory for system cursor");
        return FALSE;
    }

    // set cursor number
    ((PG_RES)mgSharedRes)->csrnum    = number;
    // set cursor data offset
    ((PG_RES)mgSharedRes)->svdbitsoffset = mgSizeRes;
    mgSizeRes += __mg_csrimgsize;
    ((PG_RES)mgSharedRes)->csroffset = mgSizeRes;

    // pointer to begin of cursor struct,
    // and reserve a space for handles for system cursors.
    temp = (char*)mgSharedRes + mgSizeRes + sizeof (PCURSOR) * number;

    for (i = 0; i < number; i++) {
        if ( !(tempcsr = sysres_load_system_cursor (i)) )
            goto error;

        memcpy (temp, tempcsr, sizeof(CURSOR));
        temp += sizeof(CURSOR);
        memcpy (temp, tempcsr->AndBits, __mg_csrimgsize);
        temp += __mg_csrimgsize;
        memcpy (temp, tempcsr->XorBits, __mg_csrimgsize);
        temp += __mg_csrimgsize;
        free (tempcsr->AndBits);
        free (tempcsr->XorBits);
        mg_slice_delete (CURSOR, tempcsr);
    }

    mgSizeRes += (sizeof (HCURSOR) + sizeof(CURSOR) + 2 * __mg_csrimgsize) * number;
    return TRUE;

error:
    return FALSE;
}

#endif /* _MGSCHEMA_COMPOSITING */

#endif /* _MGHAVE_CURSOR */

BOOL kernel_IsOnlyMe (void)
{
    int fd;

    if ((fd = open (LOCKFILE, O_RDONLY)) == -1) {
        return TRUE;
    }

    if (flock (fd, LOCK_EX | LOCK_NB) == 0) {
        // It is time to remove the old SysV IPC objects.
        key_t sem_key = get_sem_key_for_system ();

        // remove semaphore set for system.
        int semid = semget (sem_key, 0, SEM_PARAM);
        if (semid >= 0) {
            union semun ignored;
            if (semctl (semid, 0, IPC_RMID, ignored) < 0) {
                goto failed;
            }
            _WRN_PRINTF("The old semaphore set for system (0x%06x) discarded\n",
                    semid);
        }

        // remove semaphore set for layers.
        sem_key = get_sem_key_for_layers ();
        semid = semget (sem_key, 0, SEM_PARAM);
        if (semid >= 0) {
            union semun ignored;
            if (semctl (semid, 0, IPC_RMID, ignored) < 0) {
                goto failed;
            }
            _WRN_PRINTF("The old semaphore set for layers (0x%06x) discarded\n",
                    semid);
        }

        // remove semaphore set for shared surfaces.
        sem_key = get_sem_key_for_shared_surf ();
        semid = semget (sem_key, 0, SEM_PARAM);
        if (semid >= 0) {
            union semun ignored;
            if (semctl (semid, 0, IPC_RMID, ignored) < 0) {
                goto failed;
            }
            _WRN_PRINTF("The old semaphore set for shared surfaces (0x%06x) discarded\n",
                    semid);
        }

        close (fd);
        return TRUE;
    }

failed:
    close (fd);
    return FALSE;
}

void __mg_delete_sharedres_sem (void)
{
    union semun ignored;
    if (semctl (SHAREDRES_SEMID, 0, IPC_RMID, ignored) < 0)
        goto error;

    return;

error:
    perror("remove semaphore");
}

static int lockfd;

void *kernel_LoadSharedResource (void)
{
    key_t sem_key;
#ifndef _USE_MMAP
    key_t shm_key;
    void *memptr;
#endif
    int semid;
#ifndef _USE_MMAP
    int shmid;
#endif
    union semun sunion;
    PG_RES pG_res = (PG_RES) calloc (1, sizeof(G_RES));

    mgSharedRes = pG_res;
    mgSizeRes = sizeof (G_RES);

#ifdef _MGHAVE_CURSOR
    if (!LoadCursorRes()) {
        _ERR_PRINTF ("KERNEL>LoadSharedResource: %m\n");
        return NULL;
    }
#endif

#ifndef _USE_MMAP
    if ((shm_key = get_shm_key_for_system ()) == -1) {
        goto error;
    }
    shmid = shmget (shm_key, mgSizeRes, SHM_PARAM | IPC_CREAT | IPC_EXCL);
    if (shmid == -1) {
        goto error;
    }

    // Attach to the share memory.
    memptr = shmat (shmid, 0, 0);
    if (memptr == (char*)-1)
        goto error;
    else {
        memcpy (memptr, mgSharedRes, mgSizeRes);
        free (mgSharedRes);
    }

    if (shmctl (shmid, IPC_RMID, NULL) < 0)
        goto error;
#endif

    // Write shmid into the lock file.
    if ((lockfd = open (LOCKFILE, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
        goto error;

#ifdef _USE_MMAP
    if (write (lockfd, mgSharedRes, mgSizeRes) < mgSizeRes) {
        close (lockfd);
        goto error;
    }
    else
    {
        free(mgSharedRes);
        mgSharedRes = mmap( 0, mgSizeRes, PROT_READ|PROT_WRITE, MAP_SHARED, lockfd, 0);
    }
#else
    if (write (lockfd, &shmid, sizeof (shmid)) < sizeof (shmid)) {
        close (lockfd);
        goto error;
    }
#endif

    if (flock (lockfd, LOCK_EX) == -1)
        goto error;

    // close lockfd will release the exclusive lock
    // close (lockfd);

    // Obtain the semophore syncing drawing.
    if ((sem_key = get_sem_key_for_system ()) == -1) {
        goto error;
    }
    semid = semget (sem_key, _NR_SYS_SEM, SEM_PARAM | IPC_CREAT | IPC_EXCL);
    if (semid == -1) {
        goto error;
    }
    atexit (__mg_delete_sharedres_sem);

    // Initially drawing and cursor all should be available.
    sunion.val = 1;
    semctl (semid, _IDX_SEM_DRAW, SETVAL, sunion);
    sunion.val = 1;
    semctl (semid, _IDX_SEM_SCR, SETVAL, sunion);
#ifdef _MGHAVE_CURSOR
    sunion.val = 1;
    semctl (semid, _IDX_SEM_CURSOR, SETVAL, sunion);
    sunion.val = 0;
    semctl (semid, _IDX_SEM_HIDECOUNT, SETVAL, sunion);
#endif /* _MGHAVE_CURSOR */
#ifdef _MGRM_PROCESSES
    sunion.val = 1;
    semctl (semid, _IDX_SEM_MOUSEMOVE, SETVAL, sunion);
#endif /* _MGRM_PROCESSES */

#ifndef _USE_MMAP
    mgSharedRes = memptr;
    SHAREDRES_SHMID = shmid;
#endif
    SHAREDRES_SEMID = semid;

    return mgSharedRes;

error:
    _ERR_PRINTF ("KERNEL>LoadSharedResource: %m\n");
    return NULL;
}

void kernel_UnloadSharedResource (void)
{
    close (lockfd);
    unlink (LOCKFILE);
}

void* kernel_AttachSharedResource (void)
{
#ifndef _USE_MMAP
    int shmid;
#endif
    int lockfd;
    void* memptr;

    if ((lockfd = open (LOCKFILE, O_RDONLY)) == -1)
        goto error;

#ifdef _USE_MMAP
    mgSizeRes = lseek (lockfd, 0, SEEK_END );
    memptr = mmap( 0, mgSizeRes, PROT_READ, MAP_SHARED, lockfd, 0);
#else
    if (read (lockfd, &shmid, sizeof (shmid)) < sizeof (shmid))
        goto error;
    close (lockfd);

    memptr = shmat (shmid, 0, SHM_RDONLY);
#endif
    if (memptr == (char*)-1)
        goto error;
    return memptr;

error:
    perror ("AttachSharedResource");
    return NULL;
}

void kernel_UnattachSharedResource (void)
{
#ifdef _USE_MMAP
    if (munmap(mgSharedRes, mgSizeRes))
        perror("detaches shared resource");
#else
    if (shmdt (mgSharedRes) < 0)
        perror("detaches shared resource");
#endif
}

#endif /* _MGRM_STANDALONE */

