#include "sdkeyboard.h"
#include "ui_sdkeyboard.h"

SDKeyboard::SDKeyboard(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SDKeyboard)
{
    ui->setupUi(this);

    status = -1;

    connect(ui->keyboardGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),this, &SDKeyboard::updateKey);
}

SDKeyboard::~SDKeyboard()
{
    delete ui;
}

int SDKeyboard::keyboardStatus()
{
    return status;
}

void SDKeyboard::updateKey(QAbstractButton *button)
{
    status = button->text().toInt(Q_NULLPTR, 16);
    emit keyboardClicked();
}
