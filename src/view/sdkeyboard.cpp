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
