
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
	char *xdst = (char *)dst;
	char *xsrc = (char *)src;
	int i;
	for(i=0; i<len; i++){
		*xdst = *xsrc;
		*xdst++;
		*xsrc++;
	}
	return dst;
}

int k_memcmp(const void* b1, const void* b2, int len)
{
	unsigned char *xb1 = (unsigned char *)b1;
	unsigned char *xb2 = (unsigned char *)b2;
	int i;
	for(i=0; i<len; i++){
		if(*xb1 == *xb2){
			*xb1++;
			*xb2++;
		}
		else {
			return *xb1 - *xb2;
		}
	}
	return 0;
}

