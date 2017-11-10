#ifndef SIMDSP_H
#define SIMDSP_H

#include "sdfunctions.h"

extern "C"  __attribute__ ((visibility("default"))) void dsp_setup();
extern "C"  __attribute__ ((visibility("default"))) void dsp_loop();
extern "C"  __attribute__ ((visibility("default"))) void dsp_init();
extern "C"  __attribute__ ((visibility("default"))) void* getSDCore();

#endif /* SIMDSP */
