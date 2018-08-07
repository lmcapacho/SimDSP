#include "sdeditlib.h"
#include "ui_sdeditlib.h"

SDEditLib::SDEditLib(QString value, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SDEditLib)
{
    ui->setupUi(this);

    ui->lineEdit->setText(value);
}

SDEditLib::~SDEditLib()
{
    delete ui;
}

QString SDEditLib::getValue()
{
    return ui->lineEdit->text();
}
