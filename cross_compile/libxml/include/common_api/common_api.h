/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2004 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org
*/

/* Main include header for the SDL library */

#ifndef _COMMON_API_H
#define _COMMON_API_H

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

#include "gxtype.h"


#ifdef MEMORY_DEBUG

void *GxCore_MallocDebug(const char *file, int line, size_t size);
void *GxCore_MalloczDebug(const char *file, int line, size_t size);
void *GxCore_CallocDebug(const char *file, int line, size_t nmemb, size_t size);
void *GxCore_ReallocDebug(const char *file, int line, void *mem, size_t size);
void GxCore_FreeDebug(const char *file, int line, void *ptr);
char *GxCore_StrdupDebug(const char *file, int line, const char* sting);
void GxCore_MemoryShowDebug(MemoryDebugShow show);
int GxCore_MemoryTryDebug(const char *file, int line, void *p);

#define GxCore_Malloc(size)              GxCore_MallocDebug(__FILE__, __LINE__, size)
#define GxCore_Mallocz(size)             GxCore_MalloczDebug(__FILE__, __LINE__, size)
#define GxCore_Calloc(nmemb, size)       GxCore_CallocDebug(__FILE__, __LINE__, nmemb, size)
#define GxCore_Realloc(mem, size)        GxCore_ReallocDebug(__FILE__, __LINE__, mem, size)
#define GxCore_Free(ptr)                 GxCore_FreeDebug(__FILE__, __LINE__, ptr)
#define GxCore_MemoryTry(ptr)            GxCore_MemoryTryDebug(__FILE__, __LINE__, ptr)
#define GxCore_Strdup(ptr)               GxCore_StrdupDebug(__FILE__, __LINE__, ptr)
#define GxCore_MemoryShow(stat)          GxCore_MemoryShowDebug(stat)

#else

void *GxCore_MallocRelease(size_t size);
void *GxCore_MalloczRelease(size_t size);
void *GxCore_CallocRelease(size_t nmemb, size_t size);
void *GxCore_ReallocRelease(void *mem, size_t size);
void GxCore_FreeRelease(void *ptr);

#define GxCore_Malloc(size)              GxCore_MallocRelease(size)
#define GxCore_Mallocz(size)             GxCore_MalloczRelease(size)
#define GxCore_Calloc(nmemb, size)       GxCore_CallocRelease(nmemb, size)
#define GxCore_Realloc(mem, size)        GxCore_ReallocRelease(mem, size)
#define GxCore_Free(ptr)                 GxCore_FreeRelease(ptr)
#define GxCore_Strdup(ptr)               strdup(ptr)
#define GxCore_MemoryCheck()
#define GxCore_MemoryShow(stat)
#define GxCore_MemoryTry(ptr)

#endif

#ifdef __cplusplus
}
#endif

#endif /* _COMMON_API_H*/



