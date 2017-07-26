#ifndef SIMDSPCORE_H
#define SIMDSPCORE_H

#include <QWidget>
#include <QTextEdit>
#include <QVector>
#include <QAbstractButton>

#include <string>
#include "sdcore_global.h"

#include "sdsignal.h"

using namespace std;

namespace Ui {
class SimDSPCore;
}

class SIMDSPCORESHARED_EXPORT SimDSPCore : public QWidget
{
    Q_OBJECT
public:
    SimDSPCore();
    ~SimDSPCore();

    void attachInterrupt(void (*callback)());
    void println( string text );
    void println( short number );
    void println( int number ){ println((short)number);}
    void println( double number );
    void println( float number ){ println((double)number);}
    int  readKeyboard();
    void setfs(double fs);
    short readADC();
    void writeDAC(short value);
    void captureBlock( short* pBuffer, int length, void (*callback)() );
    void playBlock( short* pBuffer, int length, void (*callback)() );
    void enableMic(int length);

    void setTextOutput(QTextEdit *textOutput);

    void init();
    void start();
    void stop();
    void loadFile();
    void saveFile();

public slots:
    void keyboardClicked();

    void changeInput(int inputIndex);
    void changeFrequency(int freq);
    void changeAmplitude(int amp);
    void changeBaseTime(int bt);

    void changeInOutSelect(QAbstractButton *button);

    void newData(const QVector<double> *inTime, const QVector<double> *inFreq,
                 const QVector<double> *outTime, const QVector<double> *outFreq);

private:
    Ui::SimDSPCore *ui;

    QTextEdit *output;

    void (*sdKeyboardISRFnc)();

    SDSignal *sd;
};

#endif // SIMDSPCORE_H
