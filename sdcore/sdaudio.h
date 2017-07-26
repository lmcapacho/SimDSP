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
