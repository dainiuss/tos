
#include <kernel.h>


void poke_b (MEM_ADDR addr, BYTE value)
{
	BYTE ch = value;
	BYTE *ptr = (BYTE*) addr;
	*ptr = ch;
}

void poke_w (MEM_ADDR addr, WORD value)
{
	WORD ch = value;
	WORD *ptr = (WORD*) addr;
	*ptr = ch;
}

void poke_l (MEM_ADDR addr, LONG value)
{
	LONG ch = value;
	LONG *ptr = (LONG*) addr;
	*ptr = ch;
}

BYTE peek_b (MEM_ADDR addr)
{
	BYTE* ptr = (BYTE*) addr;
	BYTE ch = *ptr;
	return ch;
}

WORD peek_w (MEM_ADDR addr)
{
	WORD* ptr = (WORD*) addr;
	WORD ch = *ptr;
	return ch;
}

LONG peek_l (MEM_ADDR addr)
{
	LONG* ptr = (LONG*) addr;
	LONG ch = *ptr;
	return ch;
}


