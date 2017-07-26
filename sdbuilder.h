#ifndef SDBUILDER_H
#define SDBUILDER_H

#include <QObject>
#include <QDebug>
#include <QProcess>
#include <QDirIterator>

class SDBuilder : public QObject
{
    Q_OBJECT
public:
    explicit SDBuilder();
    bool build();
    void create();

signals:
    void errorOutput(QByteArray error);
    void msgOutput(QByteArray msg);

public slots:
    void readProcess();

private:
    QProcess *process;
};

#endif // SDBUILDER_H
