/*
 * SimDSP Signal.
 *
 * Copyright (c) 2017 Jorge I. Marin H.
 *                    Alexander López Parrado
 *                    Luis Miguel Capacho V.
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

#include "sdsignal.h"
#include <string>
#include <iostream>

using namespace std;

/*****************************
 * SimDSP Signal constructor
 *****************************/
SDSignal::SDSignal(QVector<QSharedMemory*> *iSharedMemoryArray)
{
    signalType = SIGNAL_SIN;
    signalAmplitude = 1.0;
    signalFrequency = 100.0;
    omegafq = 0.0;
    fs = 8000.0;

    bSyncronize = true;

    screenWidth =  1024;
    currentBufferWidth=-1;
    n = 0;

    iFileInIndex=0;
    iFileOutIndex=0;
    fileSize=screenWidth;

    xs = new QVector<double>(screenWidth);
    ys = new QVector<double>(screenWidth);

    xs->fill(0.0);
    ys->fill(0.0);

    soundCard = nullptr;

    bReadADC = false;
    bProcessBlocks = false;
    bMicRecording =  false;
    bMicEnabled =  false;

    bAWGN = false;
    snr = 40;

    isPlayCallBack=false;
    isRecordCallBack=false;

    sharedMemoryArray=iSharedMemoryArray;
}

SDSignal::~SDSignal()
{

}

void SDSignal::start()
{
    xs->fill(0.0);
    ys->fill(0.0);

    timer = new std::thread(&SDSignal::processBlocks,this);
}

void SDSignal::stop()
{
    // timer->stop();
    block_capture = nullptr;
    block_play = nullptr;

    if( bMicEnabled ) {
        if(soundCard) delete soundCard;
        soundCard = nullptr;
        bMicEnabled = false;
    }
}

short SDSignal::readADC()
{
    double *outPlotPointer,*inPlotPointer;


    double value = getSample();

    if ( bReadADC ){
        writeDAC( static_cast<short>(ys->value(n)) );
    }
    bReadADC = true;

    if( bSyncronize ){
        xs->replace(n++, value);
        if ( n == screenWidth ){
            bSyncronize = false;

            if(signalType==SIGNAL_FILE){
                n=0;
                bSyncronize=true;

            }

            outPlotPointer = static_cast<double *>((*sharedMemoryArray)[1]->data());
            inPlotPointer = static_cast<double *>((*sharedMemoryArray)[0]->data());
            // emit newData(xs, ys );

            (*sharedMemoryArray)[0]->lock();
            (*sharedMemoryArray)[1]->lock();
            memcpy(inPlotPointer,xs->data(),static_cast<size_t>(screenWidth)*sizeof(double));
            memcpy(outPlotPointer,ys->data(),static_cast<size_t>(screenWidth)*sizeof(double));
            (*sharedMemoryArray)[1]->unlock();
            (*sharedMemoryArray)[0]->unlock();
        }
    }else{
        if (bSyncpulse)
        {
            n = 0;
            bSyncronize = true;
        }
    }

    return static_cast<short>(value * MIDVALUE);
}

void SDSignal::writeDAC(short value)
{
    if (value >  MIDVALUE) value =  MIDVALUE;
    if (value < -MIDVALUE) value = -MIDVALUE;

    double *outFilePointer;

    if ( !bReadADC ) readADC();

    if( bSyncronize )
    {
        (*ys)[n] = static_cast<double>(value) / static_cast<double>(MIDVALUE);
        if( signalType == SIGNAL_FILE)
        {
            outFilePointer=static_cast<double *>((*sharedMemoryArray)[3]->data());
            (*sharedMemoryArray)[3]->lock();
            outFilePointer[iFileOutIndex++]=static_cast<double>(value) / static_cast<double>(MIDVALUE);
            (*sharedMemoryArray)[3]->unlock();
            if ( iFileOutIndex == fileSize )
                iFileOutIndex = 0;
        }
    }

    bReadADC = false;

}

void SDSignal::changeFileSize(int length){

    if(length>(1LL<<20))
        fileSize=(1LL<<20);
    else
        fileSize=length;
    iFileInIndex=0;
    iFileOutIndex=0;
}

double SDSignal::getSignalFrequency()
{
    return signalFrequency;
}


/**************************
 * Get sampling frequency
 **************************/
double SDSignal::getfs()
{
    return fs;
}

/**************************
 * Set sampling frequency
 **************************/
void SDSignal::setfs(double fsValue)
{
    fs = fsValue;
    signalFrequency = fs/100.0;
}

/**************************
 * White Gaussian noise
 **************************/
double SDSignal::wgn()
{
    double randNumber = distribution(generator);
    double np = qPow(10.0, -snr/10.0);

    return qSqrt(np)*randNumber;
}

/**************************
 * Get a new sample
 **************************/
double SDSignal::getSample()
{
    double sample = 0;

    double *inFilePointer;

    switch (signalType)
    {
    case SIGNAL_SIN:
        sample = qSin( omegafq );
        break;

    case SIGNAL_SQUARE:
        if (qSin(omegafq) >= 0 )
            sample = 0.9;
        else
            sample = -0.9;
        break;


    case SIGNAL_SAWTOOTH:
        if (signalFrequency != 0.0)
            sample = -1.0 + (omegafq/M_PI);
        break;

    case SIGNAL_NOISE:
        sample = wgn();
        break;

    case SIGNAL_FILE:

        inFilePointer=static_cast<double *>((*sharedMemoryArray)[2]->data());

        (*sharedMemoryArray)[2]->lock();
        sample = inFilePointer[iFileInIndex++];
        (*sharedMemoryArray)[2]->unlock();
        if ( iFileInIndex == fileSize )
            iFileInIndex = 0;

        break;

    default:
        break;
    }

    if( bAWGN ) sample += wgn();

    sample *= signalAmplitude;
    if ( sample >  1.0 ) sample =  1.0;
    if ( sample < -1.0 ) sample = -1.0;

    if ( (signalFrequency == 0.0) || (signalType == SIGNAL_FILE) )
    {
        omegafq += 2*M_PI;
    }
    else
    {
        omegafq += 2*M_PI*signalFrequency/fs;
    }

    if (signalType!=SIGNAL_FILE)
    {
        if( omegafq >= 2*M_PI){
            omegafq -= 2*M_PI;
            bSyncpulse = true;
        }
        else
        {
            bSyncpulse = false;
        }
    }

    return sample;
}


/**************************
 * Set signal type
 **************************/
void SDSignal::setSignalType(int st)
{
    signalType = static_cast<SignalTypes>(st);
}

/**************************
 * Set signal frequency
 **************************/
void SDSignal::setSignalFrequency(double freq)
{
    if (freq > fs/2) freq = fs/2;
    signalFrequency = freq;
}

/**************************
 * Set signal amplitude
 **************************/
void SDSignal::setSignalAmplitude(double amp)
{
    signalAmplitude = amp;
}


void SDSignal::changeAWGN(bool state)
{
    bAWGN = state;
}

void SDSignal::changeSNR(int value)
{
    snr = static_cast<double>(value);
}

void SDSignal::captureBlock(short *pBuffer, int length, void (*callback)())
{
    static sdBlock tmpblockADC;

    tmpblockADC.pBuffer = pBuffer;
    tmpblockADC.length = length;
    tmpblockADC.callback = callback;

    if(length!=currentBufferWidth){
        currentBufferWidth=length;
        std::cout<<"3,"<<QString::number(length).toStdString()<<endl;
    }

    block_capture = &tmpblockADC;

    if ( signalType == SIGNAL_MIC )
    {
        isRecordCallBack=true;
        soundCard->record();
    }

    bMicRecording = (signalType == SIGNAL_MIC);
}

void SDSignal::playBlock(short *pBuffer, int length, void (*callback)())
{
    static sdBlock tmpblockDAC;

    tmpblockDAC.pBuffer = pBuffer;
    tmpblockDAC.length = length;
    tmpblockDAC.callback = callback;

    if(length!=currentBufferWidth){
        currentBufferWidth=length;
        std::cout<<"3,"<<QString::number(length).toStdString()<<endl;
    }

    block_play = &tmpblockDAC;

    if ( signalType == SIGNAL_MIC )
    {
        isPlayCallBack=true;
        soundCard->play(block_play->pBuffer);
    }

    bMicRecording = (signalType == SIGNAL_MIC);
}

void SDSignal::processBlocks()
{
    double *outPlotPointer,*inPlotPointer;
    sdBlock* block;
    while(true){

        if ( !bMicRecording )
        {
            if (block_capture)
            {
                block = block_capture;
                block_capture = nullptr;
                captureBlockFnc(block);
            }

            if (block_play)
            {
                block = block_play;
                block_play = nullptr;
                playBlockFnc(block);
            }
        }


        outPlotPointer=static_cast<double *>((*sharedMemoryArray)[1]->data());
        inPlotPointer=static_cast<double *>((*sharedMemoryArray)[0]->data());

        (*sharedMemoryArray)[0]->lock();
        (*sharedMemoryArray)[1]->lock();
        memcpy(inPlotPointer,xs->data(),static_cast<size_t>(screenWidth)*sizeof (double));
        memcpy(outPlotPointer,ys->data(),static_cast<size_t>(screenWidth)*sizeof (double));
        (*sharedMemoryArray)[1]->unlock();
        (*sharedMemoryArray)[0]->unlock();

        QThread::msleep(500);
    }
}

void SDSignal::captureBlockFnc(sdBlock *block)
{
    int i;

    for (i=0; i<block->length; i++)
    {
        double val = getSample();
        block->pBuffer[i] = static_cast<short>(val * MIDVALUE);
        if ( i < screenWidth )
        {
            xs->replace(i, val);
        }
    }

    for ( ; i<screenWidth; i++)
    {
        xs->replace(i, 0.0);
    }

    if (block->callback)
    {
        block->callback();
    }
}

void SDSignal::playBlockFnc(sdBlock *block)
{
    double *outFilePointer;


    for (int i=0; i<screenWidth; i++)
    {
        if ( i < block->length )
            ys->replace(i, static_cast<double>(block->pBuffer[i]) / static_cast<double>(MIDVALUE));
        else
            ys->replace(i, 0.0);
    }

    if( signalType == SIGNAL_FILE  )
    {
        outFilePointer=static_cast<double *>((*sharedMemoryArray)[3]->data());

        (*sharedMemoryArray)[3]->lock();
        for (int j=0; j<block->length; j++)
        {

            outFilePointer[iFileOutIndex++]=static_cast<double>(block->pBuffer[j]) / static_cast<double>(MIDVALUE);
            if ( iFileOutIndex == fileSize )
                iFileOutIndex = 0;

        }
        (*sharedMemoryArray)[3]->unlock();

    }

    if (block->callback)
    {
        block->callback();
    }
}

void SDSignal::enableMic(int length)
{


    if(soundCard) delete soundCard;

    soundCard = new SDAudio(this);



    soundCard->initSoundCard(length, fs);

    bMicEnabled = true;
}

void SDSignal::recordFinish(short *data)
{
    if ( block_capture == nullptr )
        return;

    memcpy(block_capture->pBuffer, data, static_cast<size_t>(block_capture->length*2));

    updateBuffer(block_capture, xs);

    if (block_capture->callback && isRecordCallBack==true)
    {
        isRecordCallBack=false;
        block_capture->callback();


    }
}

void SDSignal::playFinish()
{
    if ( block_play == nullptr )
        return;

    updateBuffer(block_play, ys);

    if (block_play->callback && isPlayCallBack==true)
    {
        isPlayCallBack=false;
        block_play->callback();

    }
}

void SDSignal::updateBuffer(sdBlock *block, QVector<double> *values)
{
    for (int i=0; i<screenWidth; i++)
    {
        if(i<block->length)
            values->replace(i, static_cast<double>(block->pBuffer[i]) / static_cast<double>(MIDVALUE));
        else
            values->replace(i, 0.0);

    }
}
