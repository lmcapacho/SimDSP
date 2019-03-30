/*
 * SimDSP Functions.
 *
 * Copyright (c) 2017 lmcapacho
 *
 * This file is part of SimDSP.
 *
 * SimDSP is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SimDSP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SimDSP.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sdfunctions.h"
#include "sdcore.h"

#include <QString>
#include <QDebug>

SimDSPCore* sd;
void* SIMDSPCORESHARED_EXPORT sd_void;

/******************************************
 * SimDSP Core functions
 ******************************************/

void SIMDSPCORESHARED_EXPORT attachInterrupt(void (*callback)())
{
    sd->attachInterrupt(callback);
}

void SIMDSPCORESHARED_EXPORT println(string text)
{
    sd->println(text);
}

void SIMDSPCORESHARED_EXPORT println(short number)
{
    sd->println(number);
}

void SIMDSPCORESHARED_EXPORT println(int number)
{
    sd->println(number);
}

void SIMDSPCORESHARED_EXPORT println(double number)
{
    sd->println(number);
}

void SIMDSPCORESHARED_EXPORT println(float number)
{
    sd->println(number);
}

void SIMDSPCORESHARED_EXPORT println(const char *format, ...)
{
    QString buffer;
    va_list args;
    va_start(args, format);

    buffer.vsprintf(format, args);
    va_end(args);

    sd->println(buffer.toStdString());
}

int SIMDSPCORESHARED_EXPORT readKeyboard()
{
    return sd->readKeyboard();
}

void SIMDSPCORESHARED_EXPORT setfs(double fs)
{
    sd->setfs(fs);
}

short SIMDSPCORESHARED_EXPORT readADC()
{
    return sd->readADC();
}

void SIMDSPCORESHARED_EXPORT writeDAC(short value)
{
    sd->writeDAC(value);
}

void SIMDSPCORESHARED_EXPORT captureBlock( short* pBuffer, int length, void (*callback)() )
{
    sd->captureBlock( pBuffer, length, callback );
}

void SIMDSPCORESHARED_EXPORT playBlock( short* pBuffer, int length, void (*callback)() )
{
    sd->playBlock( pBuffer, length, callback );
}

void SIMDSPCORESHARED_EXPORT enableMic(int length)
{
    sd->enableMic(length);
}

void SIMDSPCORESHARED_EXPORT initSDCore()
{
    if(!sd){
        sd = new SimDSPCore();
        sd_void = reinterpret_cast<void*>(sd);
    }
    sd->setfs(8000);
    sd->start();
}







