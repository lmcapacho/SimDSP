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

#ifndef SDSIGNAL_H
#define SDSIGNAL_H

#include <QtCore>
#include <QtMath>
#include <QTimer>
#include <QVector>
#include <QFile>

#include <sdaudio.h>
#include <random>
#include <thread>

#define NBITS 15
#define MIDVALUE	(1<<(NBITS-1))
#define MAXFILESIZE 1048576

class SDSignal : public QObject
{
    typedef struct{
        short* pBuffer;
        int  length;
        void (*callback)();
    }sdBlock;

    Q_OBJECT
    Q_ENUMS(SignalTypes)

public:

    enum SignalTypes{
        SIGNAL_SIN,
        SIGNAL_SQUARE,
        SIGNAL_SAWTOOTH,
        SIGNAL_NOISE,
        SIGNAL_FILE,
        SIGNAL_MIC
    };

    SDSignal(QVector<QSharedMemory*> *);
    ~SDSignal();

    double getfs();

    void setfs(double fsValue);
    void setSignalAmplitude(double amp);
    void setSignalType(int st);
    void setSignalFrequency(double freq);

    void changeSNR(int value);
    void changeAWGN(bool state);

    double getSignalFrequency();

    double wgn();
    double getSample();
    short readADC();
    void  writeDAC(short value);
    void captureBlock( short* pBuffer, int length, void (*callback)() );
    void playBlock( short* pBuffer, int length, void (*callback)() );

    void enableMic(int length);

    void changeFileSize(int length);

    void start();
    void stop();


    void recordFinish(short* data);
    void playFinish();

    void processBlocks();

private:

    void updateBuffer(sdBlock *block, QVector<double> *values);
    void captureBlockFnc(sdBlock *block);
    void playBlockFnc(sdBlock *block);

    double fs;
    double signalFrequency;
    double signalAmplitude;
    double omegafq;
    int fileSize;
    double snr;
    bool bAWGN;
    std::default_random_engine generator;
    std::normal_distribution<double> distribution;

    SignalTypes signalType;

    QVector<double> *xs;
    QVector<double> *ys;


    std::thread *timer;

    int n;

    int screenWidth;

    int currentBufferWidth;

    double fMax;

    bool bSyncronize;
    bool bSyncpulse;

    int iFileInIndex=0;
    int iFileOutIndex=0;

    bool bReadADC;
    bool bProcessBlocks;
    bool bMicRecording;
    bool bMicEnabled;

    bool isPlayCallBack;
    bool isRecordCallBack;

    SDAudio *soundCard;

    sdBlock* block_capture = nullptr;
    sdBlock* block_play = nullptr;

    QVector<QSharedMemory*> *sharedMemoryArray;
};

#endif // QSDSIGNAL_H
