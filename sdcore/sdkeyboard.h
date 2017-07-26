#ifndef SDKEYBOARD_H
#define SDKEYBOARD_H

#include <QWidget>
#include <QAbstractButton>

namespace Ui {
class SDKeyboard;
}

class SDKeyboard : public QWidget
{
    Q_OBJECT

public:
    explicit SDKeyboard(QWidget *parent = 0);
    ~SDKeyboard();

    int keyboardStatus();

signals:
   void keyboardClicked();

public slots:
    void updateKey(QAbstractButton *button);

private:
    Ui::SDKeyboard *ui;

    int status;
};

#endif // SDKEYBOARD_H
