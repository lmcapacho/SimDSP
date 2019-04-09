/*
 * SimDSP Core.
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

#ifndef SIMDSPCORE_H
#define SIMDSPCORE_H


#include <QVector>


#include <string>
#include <thread>
#include "sdcore_global.h"

#include "sdsignal.h"


using namespace std;



class SIMDSPCORESHARED_EXPORT SimDSPCore: public QObject
{
    Q_OBJECT
public:
    SimDSPCore();
    ~SimDSPCore();

    void attachInterrupt(void (*callback)());
    void println( string text );
    void println( short number );
    void println( int number ){ println(static_cast<short>(number));}
    void println( double number );
    void println( float number ){ println(static_cast<double>(number));}
    int  readKeyboard();
    void setfs(double fs);
    short readADC();
    void writeDAC(short value);
    void captureBlock( short* pBuffer, int length, void (*callback)() );
    void playBlock( short* pBuffer, int length, void (*callback)() );
    void enableMic(int length);

    void init();
    void start();
    void stop();

    void keyboardClicked(int iKey);

    void changeInput(int inputIndex);
    void changeFrequency(int freq);
    void changeAmplitude(int amp);
    void changeFileSize(int length);
    void changeAWGN(bool checked);
    void changeSNR(int snr);

private:

    QSharedMemory *plotBufferIn;
    QSharedMemory *plotBufferOut;
    QSharedMemory *fileBufferIn;
    QSharedMemory *fileBufferOut;
    int key;
    void (*sdKeyboardISRFnc)();

    QVector<QSharedMemory*> *sharedMemoryArray;


    SDSignal *sd;

    void standardInputTask();
    std::thread *standardInputThread;
};

#endif // SIMDSPCORE_H
