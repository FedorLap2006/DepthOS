#pragma once

#include <std/types.h>

uint32_t kalloc_ap(uint32_t size,int align,uint32_t *phys);
uint32_t kalloc_p(uint32_t size,uint32_t *phys);
uint32_t kalloc_a(uint32_t size,int align);
uint32_t kalloc(uint32_t size);

