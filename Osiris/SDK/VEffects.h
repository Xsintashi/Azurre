#pragma once
#include "DLight.h"

class VEffects
{
public:
	DLight* clAllocDLight(int key) {
		using original_fn = DLight * (__thiscall*)(void*, int);
		return (*(original_fn**)this)[4](this, key);
	}
	DLight* clAllocELight(int key) {
		using original_fn = DLight * (__thiscall*)(void*, int);
		return (*(original_fn**)this)[5](this, key);
	}
	DLight* getElightByKey(int key) {
		using original_fn = DLight * (__thiscall*)(void*, int);
		return (*(original_fn**)this)[8](this, key);
	}

};