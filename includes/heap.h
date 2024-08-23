#pragma once
#ifndef __HEAP_MSXDOS_H__
#define __HEAP_MSXDOS_H__

#ifdef __HEAP_H__
#error You cannot use both MSXDOS and non-DOS heap functions
#endif

#include <stdint.h>


extern uint8_t *heap_top;

extern void *malloc(uint16_t size);
extern void free(uint16_t size);

#endif//__HEAP_MSXDOS_H__
