/*
 * utils.h
 *
 *  Created on: Jul 30, 2023
 *      Author: mille
 */

#ifndef INC_UTILS_H_
#define INC_UTILS_H_


#define U_MAX(A,B)			((A)>(B) ? (A) : (B))
#define U_MIN(A,B)			((A)<(B) ? (A) : (B))


void zero_mem(void* address, int size);
int copy_mem(void* p_source, void* p_dest, int size);

int str_len(const char* cstr);


#endif /* INC_UTILS_H_ */
