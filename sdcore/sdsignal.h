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

#include <fftw3.h>

#define NBITS 15
#define MIDVALUE	(1<<(NBITS-1))

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
      SIGNAL_FILE,
      SIGNAL_MIC
    };

    SDSignal();
    ~SDSignal();

    double getfs();

    void setfs(double fsValue);
    void setSignalAmplitude(double amp);
    void setSignalType(int st);
    void setSignalFrequency(double freq);
    double getSignalFrequency();
    void setBaseTime(int bt);

    double getSample();
    short readADC();
    void  writeDAC(short value);
    void captureBlock( short* pBuffer, int length, void (*callback)() );
    void playBlock( short* pBuffer, int length, void (*callback)() );

    void enableMic(int length);

    void loadFile(QFile *file);
    void saveFile(QFile *file);

    void start();
    void stop();

signals:
   void newData(const QVector<double> *inTime, const QVector<double> *inFreq,
                     const QVector<double> *outTime, const QVector<double> *outFreq);

public slots:
   void processBlocks();
   void recordFinish(short* data);
   void playFinish();

private:

    void updateFFT();
    void updateBuffer(sdBlock *block, QVector<double> *values);
    void captureBlockFnc(sdBlock *block);
    void playBlockFnc(sdBlock *block);

    double fs;
    double signalFrequency;
    double signalAmplitude;
    double omegafq;

    SignalTypes signalType;

    QVector<double> *xs;
    QVector<double> *ys;
    QVector<double> *Xs;
    QVector<double> *Ys;

    QTimer *timer;

    int n;
    int baseTime;
    int screenWidth;
    int fftWidth;
    double fMax;

    bool bSyncronize;
    bool bSyncpulse;

    double *fftInADC;
    fftw_complex *fftOutADC;

    double *fftInDAC;
    fftw_complex *fftOutDAC;

    fftw_plan planADC;
    fftw_plan planDAC;

    int iFileLength;
    int iFileInIndex;
    int iFileOutIndex;
    double *pFileInBuffer;
    double *pFileOutBuffer;

    bool bReadADC;
    bool bProcessBlocks;
    bool bMicRecording;
    bool bMicEnabled;

    SDAudio *soundCard;

    sdBlock* block_capture = NULL;
    sdBlock* block_play = NULL;
};

#endif // QSDSIGNAL_H
