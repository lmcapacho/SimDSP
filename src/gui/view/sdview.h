/*
 * SimDSP View.
 *
 * Copyright (c) 2019 lmcapacho
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

#ifndef SDVIEW_H
#define SDVIEW_H

#include <QWidget>
#include <QTextEdit>
#include <QVector>
#include <QAbstractButton>
#include <QSharedMemory>
#include <QTimer>

#include <string>
#include <fftw3.h>

#include "sdmat.h"

#define MAXFILESIZE 1048576

using namespace std;

namespace Ui {
class SDView;
}

class SDView : public QWidget
{
    Q_OBJECT
public:

    enum SignalTypes{
      SIGNAL_SIN,
      SIGNAL_SQUARE,
      SIGNAL_SAWTOOTH,
      SIGNAL_NOISE,
      SIGNAL_FILE,
      SIGNAL_MIC
    };

    SDView(QWidget *parent = nullptr);
    ~SDView();

    void println( QString text );
    int  readKeyboard();
    void setfs(double fs);
    void enableMic();
    void clearOutput();
    void newData();
    void updateFFT();

    void init();
    void start();
    void stop();

signals:
   void changeView(QByteArray data);

public slots:
    void autoScale();
    void resetZoom();
    void loadMatFile();
    void saveMatFile();

    void keyboardClicked();

    void setInput(int inputIndex);
    void setFrequency(int freq);
    void setAmplitude(int amp);
    void changeAmplitude(int inc);
    void setBaseTime(int bt);
    void changeBaseTime(int inc);

    void setInOutSelect(QAbstractButton *button);

    void setAWGN(bool checked);
    void setSNR(int value);

    void setSizeWindow(int size);

    void loadFile(QString path, QString varName);

private:
    Ui::SDView *ui;

    QTextEdit *output;

    QTimer *refresh;

    SDMat *sdmat;

    QSharedMemory SDPlotBufferIn;
    QSharedMemory SDPlotBufferOut;
    QSharedMemory SDFileBufferIn;
    QSharedMemory SDFileBufferOut;

    QVector<double> *inTime;
    QVector<double> *outTime;
    QVector<double> *inFreq;
    QVector<double> *outFreq;

    size_t fileLength;

    int fftWidth;
    double *fftInADC;
    fftw_complex *fftOutADC;
    double *fftInDAC;
    fftw_complex *fftOutDAC;

    fftw_plan planADC;
    fftw_plan planDAC;
};

#endif // SDVIEW_H
