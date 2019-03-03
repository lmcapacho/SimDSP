/*
 * SimDSP Keyboard.
 *
 * Copyright (c) 2017 lmcapacho
 *
 * This file is part of SimDSP.
 *
 * SimDSP is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SimDSP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SimDSP.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    explicit SDKeyboard(QWidget *parent = nullptr);
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
