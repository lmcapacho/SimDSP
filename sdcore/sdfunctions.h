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

void  initSDCore();

#endif // SDFUNCTIONS_H
