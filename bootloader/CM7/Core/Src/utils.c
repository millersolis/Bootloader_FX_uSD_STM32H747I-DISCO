/*
 * utils.c
 *
 *  Created on: Jul 30, 2023
 *      Author: mille
 */

#include "utils.h"


void zero_mem(void* address, int size)
{
	unsigned char* p_data=address;
	int i;
	for (i=0; i<size; i++)
	{
		p_data[i]=0;
	}
}

int copy_mem(void* p_source, void* p_dest, int size)
{
	unsigned char* p_from = p_source;
	unsigned char* p_to = p_dest;
	int i;

	for (i=0; i<size; i++)
	{
		p_to[i] = p_from[i];
	}

	return i;
}

int str_len(const char* cstr)
{
    int size = 0;
    while (cstr[size] != '\0') size++;
    return size;
}

