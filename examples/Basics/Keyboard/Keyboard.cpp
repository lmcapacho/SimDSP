// SimDSP main file

#include <simdsp.h>

void printKey()
{
	println(readKeyboard());
}

void dsp_setup()
{
	attachInterrupt(printKey);
}		

void dsp_loop()
{}