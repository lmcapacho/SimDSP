#ifndef SDHELP_H
#define SDHELP_H

#include <QDialog>
#include <QHelpEngine>

namespace Ui {
class SDHelp;
}

class SDHelp : public QDialog
{
    Q_OBJECT

public:
    explicit SDHelp(QWidget *parent = 0);
    ~SDHelp();

private:
    Ui::SDHelp *ui;

    QHelpEngine* helpEngine;
};

#endif // SDHELP_H
