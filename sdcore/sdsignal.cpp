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

/*****************************
 * SimDSP Signal constructor
 *****************************/
SDSignal::SDSignal()
{
    signalType = SIGNAL_SIN;
    signalAmplitude = 1.0;
    signalFrequency = 100.0;
    omegafq = 0.0;
    fs = 8000.0;

    bSyncronize = true;
    baseTime = 1;
    screenWidth =  850;
    fftWidth =  1024;
    n = 0;

    fftInADC = fftw_alloc_real(fftWidth);
    fftOutADC = fftw_alloc_complex((fftWidth/2)+1);
    fftInDAC = fftw_alloc_real(fftWidth);
    fftOutDAC = fftw_alloc_complex((fftWidth/2)+1);

    xs = new QVector<double>(screenWidth);
    ys = new QVector<double>(screenWidth);
    Xs = new QVector<double>(fftWidth);
    Ys = new QVector<double>(fftWidth);

    xs->fill(0.0);
    ys->fill(0.0);

    planADC = fftw_plan_dft_r2c_1d ( fftWidth, fftInADC, fftOutADC, FFTW_ESTIMATE );
    planDAC = fftw_plan_dft_r2c_1d ( fftWidth, fftInDAC, fftOutDAC, FFTW_ESTIMATE );

    pFileInBuffer = NULL;
    pFileOutBuffer = NULL;

    soundCard = NULL;

    bReadADC = false;
    bProcessBlocks = false;
    bMicRecording =  false;
    bMicEnabled =  false;

    timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &SDSignal::processBlocks);
}

SDSignal::~SDSignal()
{
    fftw_free(fftInADC);
    fftw_free(fftOutADC);
    fftw_free(fftInDAC);
    fftw_free(fftOutDAC);

    fftw_destroy_plan (planADC);
    fftw_destroy_plan (planDAC);

    if( pFileInBuffer )
        delete pFileInBuffer;
    if( pFileOutBuffer )
        delete pFileOutBuffer;
}

void SDSignal::start()
{
    timer->start(500);
}

void SDSignal::stop()
{
    timer->stop();
    block_capture = NULL;
    block_play = NULL;

    if( bMicEnabled ) {
        if(soundCard) delete soundCard;
        soundCard = NULL;
        bMicEnabled = false;
    }
}

short SDSignal::readADC()
{
    double value = getSample();

    if ( bReadADC ){
        writeDAC( ys->value(n) );
    }
    bReadADC = true;

    if( bSyncronize ){
        xs->replace(n++, value);
        if ( n == screenWidth ){
            bSyncronize = false;
            updateFFT();
            emit newData(xs, Xs, ys, Ys);
        }
    }else{
      if (bSyncpulse)
      {
        n = 0;
        bSyncronize = true;
      }
    }

    return value * MIDVALUE;
}

void SDSignal::writeDAC(short value)
{
    if (value >  MIDVALUE) value =  MIDVALUE;
    if (value < -MIDVALUE) value = -MIDVALUE;

    if ( !bReadADC ) readADC();

    if( bSyncronize )
    {
        (*ys)[n] = (double)value /(double) MIDVALUE;
        if( signalType == SIGNAL_FILE)
        {
            if( pFileOutBuffer )
            {
                pFileOutBuffer[ iFileOutIndex++ ] = (double)value /(double) MIDVALUE;
                if ( iFileOutIndex == iFileLength )
                    iFileOutIndex = 0;
            }
        }
    }

    bReadADC = false;

}

void SDSignal::updateFFT()
{
    double real, img;

    for( int i=0; i<fftWidth; i++){
        fftInADC[i]=xs->value(i);
        fftInDAC[i]=ys->value(i);
    }


    fftw_execute (planADC);
    fftw_execute (planDAC);

    for( int i=0; i<fftWidth/2; i++)
    {
        real = fftOutADC[i][0];
        img = fftOutADC[i][1];
        Xs->replace(i, qSqrt(qPow(real,2.0) + qPow(img,2.0))/511.0);

        real = fftOutDAC[i][0];
        img = fftOutDAC[i][1];
        Ys->replace(i, qSqrt(qPow(real,2.0) + qPow(img,2.0))/511.0);
    }
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
 * Get a new sample
 **************************/
double SDSignal::getSample()
{
    double sample = 0;

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
            if (signalFrequency != 0)
                sample = -1.0 + (omegafq/M_PI);
        break;

        case SIGNAL_FILE:
            if( pFileInBuffer )
            {
                sample = pFileInBuffer[ iFileInIndex++ ];
                if ( iFileInIndex == iFileLength )
                    iFileInIndex = 0;
            }
            else
            {
                sample = 0;
            }
        break;

        default:
        break;
    }

    sample *= signalAmplitude;
    if ( sample >  1.0 ) sample =  1.0;
    if ( sample < -1.0 ) sample = -1.0;

    if ( (signalFrequency == 0) || (signalType == SIGNAL_FILE) )
    {
        omegafq += 2*M_PI;
    }
    else
    {
        omegafq += 2*M_PI*signalFrequency/fs;
    }

    if (omegafq >= 2*M_PI)
    {
        omegafq -= 2*M_PI;
        bSyncpulse = true;
    }
    else
    {
        bSyncpulse = false;
    }

    return sample;
}


/**************************
 * Set signal type
 **************************/
void SDSignal::setSignalType(int st)
{
    signalType = static_cast<SignalTypes>(st);

    /*if(signalType == SIGNAL_MIC){

    }else{
        if(soundCard){
            disconnect(soundCard, 0, this, 0);
            delete soundCard;
        }
        soundCard = NULL;
    }*/

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


/**************************
 * Set base time
 **************************/
void SDSignal::setBaseTime(int bt)
{
    baseTime = bt;
}

/**************************
 * Load File
 **************************/
void SDSignal::loadFile(QFile *file)
{
    iFileInIndex = 0;
    iFileOutIndex = 0;
    iFileLength = 0;

    QTextStream in(file);

    if ( pFileInBuffer ) delete pFileInBuffer;
    if ( pFileOutBuffer ) delete pFileOutBuffer;

    pFileInBuffer = new double[64000];
    pFileOutBuffer = new double[64000];

    while (!in.atEnd())
    {
        in >> pFileInBuffer[iFileLength++];

        if (iFileLength == 64000)
            break;
    }
}


void SDSignal::saveFile(QFile *file)
{
    QTextStream out(file);
    int index = 0;

    if ( pFileOutBuffer )
        while ( index < iFileLength )
            out << pFileOutBuffer[index++] << " ";
}

void SDSignal::captureBlock(short *pBuffer, int length, void (*callback)())
{
    static sdBlock tmpblockADC;

    tmpblockADC.pBuffer = pBuffer;
    tmpblockADC.length = length;
    tmpblockADC.callback = callback;

    block_capture = &tmpblockADC;

    if ( signalType == SIGNAL_MIC )
    {
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

    block_play = &tmpblockDAC;

    if ( signalType == SIGNAL_MIC )
    {
        soundCard->play(block_play->pBuffer);
    }

    bMicRecording = (signalType == SIGNAL_MIC);
}

void SDSignal::processBlocks()
{
    if ( !bMicRecording )
    {
        sdBlock* block;

        if (block_capture)
        {
            block = block_capture;
            block_capture = NULL;
            captureBlockFnc(block);
        }
        /*else
        {
            xs->fill(0.0);
        }*/

        if (block_play)
        {
            block = block_play;
            block_play = NULL;
            playBlockFnc(block);
        }
        /*else
        {
            ys->fill(0.0);
        }*/
    }

    updateFFT();
    emit newData(xs, Xs, ys, Ys);
}

void SDSignal::captureBlockFnc(sdBlock *block)
{
    int i;

    for (i=0; i<block->length; i++)
    {
        double val = getSample();
        block->pBuffer[i] = val * MIDVALUE;
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
    for (int i=0; i<screenWidth; i++)
    {
        if ( i < block->length )
            ys->replace(i, (double)block->pBuffer[i] /(double) MIDVALUE);
        else
            ys->replace(i, 0.0);
    }

    if( signalType == SIGNAL_FILE && pFileOutBuffer )
    {
        for (iFileOutIndex=0; iFileOutIndex<block->length; iFileOutIndex++)
        {
            if( iFileOutIndex < iFileLength )
                pFileOutBuffer[ iFileOutIndex ] = (double)block->pBuffer[iFileOutIndex] /(double) MIDVALUE;
            else
                break;
        }
        iFileOutIndex = 0;
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

    connect(soundCard, QOverload<short*>::of(&SDAudio::recordFinish), this, &SDSignal::recordFinish);
    connect(soundCard, &SDAudio::playFinish, this, &SDSignal::playFinish);

    soundCard->initSoundCard(length, fs);

    bMicEnabled = true;
}

void SDSignal::recordFinish(short *data)
{
  if ( block_capture == NULL )
  return;

  memcpy(block_capture->pBuffer, data, block_capture->length*2);

  updateBuffer(block_capture, xs);

  if (block_capture->callback)
  {
    block_capture->callback();
  }
}

void SDSignal::playFinish()
{
  if ( block_play == NULL )
  return;

  updateBuffer(block_play, ys);

  if (block_play->callback)
  {
    block_play->callback();
  }
}

void SDSignal::updateBuffer(sdBlock *block, QVector<double> *values)
{
    for (int i=0; i<screenWidth; i++)
    {
        if ( i < block->length )
            values->replace(i, (double)block->pBuffer[i] /(double) MIDVALUE);
        else
            values->replace(i, 0.0);
    }
}
