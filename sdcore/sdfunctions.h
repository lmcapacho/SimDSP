#ifndef SDFUNCTIONS_H
#define SDFUNCTIONS_H

#include <string>

extern void* sd_void;

using namespace std;

void attachInterrupt(void (*callback)());
void println(string text);
void println(int number);
void println(short number);
void println(double number);
void println(float number);
void println(const char *format, ...);
int  readKeyboard();
void setfs(double fs);
short readADC();
void writeDAC(short value);
void captureBlock( short* pBuffer, int length, void (*callback)() );
void playBlock( short* pBuffer, int length, void (*callback)() );
void enableMic(int length);

void initSDCore();

#endif // SDFUNCTIONS_H
