#include "simdsp.h"

int main()
{
    initSDCore();
    dsp_setup();

    while(true){
        dsp_loop();
    }
}
