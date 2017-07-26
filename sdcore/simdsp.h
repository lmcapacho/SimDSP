#ifndef SIMDSP_H
#define SIMDSP_H

#include "sdfunctions.h"

extern "C" void dsp_setup();
extern "C" void dsp_loop();
extern "C" void dsp_init();
extern "C" void* getSDCore();

#endif /* SIMDSP */
