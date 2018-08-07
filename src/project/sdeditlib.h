#ifndef SDEDITLIB_H
#define SDEDITLIB_H

#include <QDialog>

namespace Ui {
class SDEditLib;
}

class SDEditLib : public QDialog
{
    Q_OBJECT

public:
    explicit SDEditLib(QString value = QString::null, QWidget *parent = 0);
    ~SDEditLib();

    QString getValue();

private:
    Ui::SDEditLib *ui;
};

#endif // SDEDITLIB_H
