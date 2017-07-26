#include "simdsp.h"

short x;
short xant = 0; 
short y;

void dsp_setup()
{
}

void dsp_loop()
{
	x = readADC();
	
	y = x - xant;
	xant = x;
	
	writeDAC(y);
}
