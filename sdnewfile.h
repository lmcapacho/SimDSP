#ifndef SDNEWFILE_H
#define SDNEWFILE_H

#include <QDialog>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QPushButton>
#include <QRegExp>
#include <QRegExpValidator>
#include <QDebug>

#define SOURCE 0
#define HEADER 1

namespace Ui {
class SDNewFile;
}

class SDNewFile : public QDialog
{
    Q_OBJECT

public:
    explicit SDNewFile(QWidget *parent = 0);
    ~SDNewFile();
    QString getFileName();

public slots:
    void changeTemplateSelect(int index);
    void changeFileName(const QString &fileName);
    void accepted();

private:
    void loadTemplate(int index);

    Ui::SDNewFile *ui;
};

#endif // SDNEWFILE_H
