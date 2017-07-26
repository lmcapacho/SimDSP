#include "sdfunctions.h"
#include "sdcore.h"

#include <QString>
#include <QDebug>

SimDSPCore* sd;
void* sd_void;

/******************************************
 * SimDSP Core functions
 ******************************************/

void attachInterrupt(void (*callback)())
{
    sd->attachInterrupt(callback);
}

void println(string text)
{
    sd->println(text);
}

void println(short number)
{
    sd->println(number);
}

void println(int number)
{
    sd->println(number);
}

void println(double number)
{
    sd->println(number);
}

void println(float number)
{
    sd->println(number);
}

void println(const char *format, ...)
{
    QString buffer;
    va_list args;
    va_start(args, format);

    buffer.vsprintf(format, args);
    va_end(args);

    sd->println(buffer.toStdString());
}

int  readKeyboard()
{
    return sd->readKeyboard();
}

void setfs(double fs)
{
    sd->setfs(fs);
}

short readADC()
{
    return sd->readADC();
}

void writeDAC(short value)
{
    sd->writeDAC(value);
}

void captureBlock( short* pBuffer, int length, void (*callback)() )
{
    sd->captureBlock( pBuffer, length, callback );
}

void playBlock( short* pBuffer, int length, void (*callback)() )
{
    sd->playBlock( pBuffer, length, callback );
}

void enableMic(int length)
{
    sd->enableMic(length);
}

void initSDCore()
{
    if(!sd){
        sd = new SimDSPCore();
        sd_void = reinterpret_cast<void*>(sd);
    }
    sd->setfs(8000);
}
