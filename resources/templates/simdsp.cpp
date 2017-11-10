#include "simdsp.h"

 __attribute__ ((visibility("default"))) void dsp_init()
{
    initSDCore();
    dsp_setup();
}

 __attribute__ ((visibility("default"))) void loop()
{
    dsp_loop();
}

 __attribute__ ((visibility("default"))) void* getSDCore()
{
    return sd_void;
}
