
#include "common_api.h"


void *GxCore_MallocRelease(size_t size)
{
	void *p = malloc(size);

	if(p == NULL)
		p = malloc(size);

	return p;
}

void *GxCore_MalloczRelease(size_t size)
{
	void *p = malloc(size);

	if (p)
		memset(p, 0, size);

	return p;
}

void GxCore_FreeRelease(void *ptr)
{
	if (ptr)
		free(ptr);
}

void *GxCore_CallocRelease(size_t nmemb, size_t size)
{
	return GxCore_MalloczRelease(nmemb * size);
}

void *GxCore_ReallocRelease(void *mem, size_t size)
{
	void *ptr = GxCore_MalloczRelease(size);
	if(NULL == ptr)
	{
		return (ptr);
	}
	memcpy(ptr, mem, size);
	free(mem);

	return (ptr);
}

