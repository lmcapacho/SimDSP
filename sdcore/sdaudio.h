/*
 * SimDSP Audio.
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


#ifndef SDAUDIO_H
#define SDAUDIO_H

#include <QObject>

#include <QDebug>
#include <QIODevice>
#include <QtEndian>
#include <QBuffer>
#include <QAudioInput>
#include <QAudioOutput>

class SDAudio : public QObject
{
    Q_OBJECT
public:
    explicit SDAudio(QObject *parent = 0);
    ~SDAudio();

    void initSoundCard() {initSoundCard(4096, 8000);}
    void initSoundCard(int bSize, double fs);
    void createAudioInput();
    void createAudioOutput();

    void record();
    void play(short *outBuffer);

    void setSampleRate(int sampleRate);
    void setSampleSize(int sampleSize);
    void setChannelCount(int channelCount);

signals:
    void recordFinish( short *inBuffer);
    void playFinish();

public slots:
    void readMore();
    void stateChanged(QAudio::State state);

private:

    QAudioDeviceInfo audioInputDevice;
    QAudioDeviceInfo audioOutputDevice;
    QAudioInput *audioInput;
    QAudioOutput *audioOutput;
    QIODevice *inputData;
    QAudioFormat audioFormat;
    QIODevice *outputData;
    QByteArray *buffer;

    int bufferSize;
};

#endif // SDAUDIO_H
