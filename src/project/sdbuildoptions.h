#ifndef SDBUILDOPTIONS_H
#define SDBUILDOPTIONS_H

#include <QDialog>
#include <QFileDialog>
#include <QListWidgetItem>
#include <QLineEdit>
#include <QDebug>
#include <QDir>
#include <QRegExp>

#include "sdeditlib.h"

namespace Ui {
class SDBuildOptions;
}

class SDBuildOptions : public QDialog
{
    Q_OBJECT

public:
    explicit SDBuildOptions(QString path, QWidget *parent = 0);
    ~SDBuildOptions();

public slots:
    void addInclude();
    void editInclude();
    void removeInclude();
    void addLibrary();
    void editLibrary();
    void removeLibrary();
    void addLib();
    void editLib();
    void removeLib();
    void changeStd();

private:
    void addOption(QString option, QString value, QString prefix);
    void editOption(QString option, QString oldValue, QString newValue);
    void removeOption(QString option, QString value, QString prefix);
    void loadOptions();
    QString searchOption(QString option);

    Ui::SDBuildOptions *ui;

    QFile mkFile;
};

#endif // SDBUILDOPTIONS_H
