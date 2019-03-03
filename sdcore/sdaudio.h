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


#ifndef SDAUDIO_H
#define SDAUDIO_H

#include <QObject>
#include <QMutex>
#include <QDebug>
#include <QIODevice>
#include <QtEndian>
#include <QBuffer>
#include <QAudioInput>
#include <QAudioOutput>
#include <RtAudio.h>
#include <QThread>

class SDAudio : public QObject
{
    Q_OBJECT
public:
    explicit SDAudio(QObject *parent = nullptr);
    ~SDAudio();

    int initSoundCard() { return initSoundCard(4096, 8000);}
    int initSoundCard(int bSize, double fs);


    void record();
    void play(short *outBuffer);

    QByteArray *buffer_adc;
    QByteArray *buffer_dac;
    QMutex *mutex_adc;
    QMutex *mutex_dac;

signals:
    void recordFinish( short *inBuffer);
    void playFinish();

private:
    RtAudio *adc,*dac;
    int bufferSize;
};

#endif // SDAUDIO_H
