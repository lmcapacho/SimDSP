#include "simdsp.h"

void dsp_init()
{
    initSDCore();
    dsp_setup();
}

void loop()
{
    dsp_loop();
}

void* getSDCore()
{
    return sd_void;
}
