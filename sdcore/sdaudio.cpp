/*
 * SimDSP Audio.
 *
 * Copyright (c) 2018 parrado
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

#include "sdaudio.h"

SDAudio::SDAudio(QObject *parent) :
    QObject(parent)
{
    adc = new RtAudio();
    dac = new RtAudio();
    mutex_adc=new QMutex();
    mutex_dac=new QMutex();
}

SDAudio::~SDAudio()
{
    adc->stopStream();
    dac->stopStream();
    adc->closeStream();
    dac->closeStream();

    delete buffer_adc;
    delete buffer_dac;
    delete mutex_adc;
    delete mutex_dac;
}

//Callback de captura de la tarjeta de sonido
int in( void  * /*outputBuffer*/, void *inputBuffer, unsigned int nBufferFrames,
           double /*streamTime*/, RtAudioStreamStatus /*status*/, void * data )
{
    SDAudio *sda;

    sda=(SDAudio*)data;

    QMutex *mtx_adc=sda->mutex_adc;

    QByteArray *bufferin=sda->buffer_adc;

    //Se copian los datos al buffer de captura de SimDSP (exclusión mútua)
    mtx_adc->lock();
     memcpy(bufferin->data(),inputBuffer,nBufferFrames*sizeof(short));
    mtx_adc->unlock();

    //Se emite la señal de final de captura a la GUI
    emit sda->recordFinish((short*)bufferin->data()) ;

    return 0;
}

//Callback de reproducción de la tarjeta de sonido
int out( void *outputBuffer, void * /*inputBuffer*/, unsigned int nBufferFrames,
           double /*streamTime*/, RtAudioStreamStatus /*status*/, void * data )
{
    SDAudio *sda;

    sda=(SDAudio*)data;

    QMutex *mtx_dac=sda->mutex_dac;

    QByteArray *bufferout=sda->buffer_dac;
    short *outaux=(short *)outputBuffer;
    short *bufferoutaux=(short *)bufferout->constData();

    //Se copian los datos mezclados a los canales estereo usando exclusión mútua.
    mtx_dac->lock();
    for(unsigned int i=0;i<nBufferFrames;i++){
        outaux[2*i]=bufferoutaux[i];
        outaux[2*i+1]=bufferoutaux[i];
    }
    mtx_dac->unlock();

    //Emite señal de final de reproducción a la GUI
    emit sda->playFinish();

    return 0;
}

//Inicializa tarjeta de sonido con rtaudio
int SDAudio::initSoundCard(int bSize, double fs)
{

    bufferSize = bSize*sizeof(short);
    buffer_adc =  new QByteArray(bufferSize, 0);
    buffer_dac =  new QByteArray(bufferSize, 0);

    unsigned int fsl = fs;

    unsigned int bufferFrames = bSize;
    RtAudio::StreamParameters iParams, oParams;
    iParams.deviceId = 0;
    iParams.nChannels = 1;
    iParams.firstChannel = 0;

    oParams.deviceId = 0;
    oParams.nChannels = 2;
    oParams.firstChannel = 0;

    iParams.deviceId = adc->getDefaultInputDevice();
    oParams.deviceId = dac->getDefaultOutputDevice();

    RtAudio::StreamOptions options;

    if(adc->isStreamOpen()){
        if(adc->isStreamRunning())
            adc->stopStream();
        adc->closeStream();
    }
    if(dac->isStreamOpen()){
        if(dac->isStreamRunning())
            dac->stopStream();
        dac->closeStream();
    }

    try{
        adc->openStream( NULL, &iParams, RTAUDIO_SINT16, fsl, &bufferFrames, &in, (void *)this, &options );
        dac->openStream( &oParams, NULL, RTAUDIO_SINT16, fsl, &bufferFrames, &out, (void *)this, &options );
    }catch(...){
        return -1;
    }

    return 0;
}

void SDAudio::record()
{
    if(adc->isStreamRunning()==false)
    adc->startStream();
}

void SDAudio::play(short *outBuffer)
{
    if(dac->isStreamRunning()==false)
    dac->startStream();

    mutex_dac->lock();
    memcpy(buffer_dac->data(),outBuffer,bufferSize);
    mutex_dac->unlock();
}
