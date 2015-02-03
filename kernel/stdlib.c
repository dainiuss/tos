
#include <kernel.h>


int k_strlen(const char* str)
{
	int i;
	for(i=0; *str != '\0'; i++){
		str++;
	}
	return i;
}

void* k_memcpy(void* dst, const void* src, int len)
{
	int i;
	for(i=0; i<len; i++){
		(char *)dst = (char *)src;
		*dst++;
		*src++;
	}
	return dst;
}

int k_memcmp(const void* b1, const void* b2, int len)
{
	int i;
	for(i=0; i<len; i++){
		if((unsigned char *)b1 == (unsigned char *)b2){
			*b1++;
			*b2++;
		}
		else {
			return (unsigned char *)b1 - (unsigned char *)b2;
		}
	}
	return 0;
}

