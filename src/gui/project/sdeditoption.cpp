/*
 * SimDSP edit library.
 *
 * Copyright (c) 2018 lmcapacho
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

#include "sdeditoption.h"
#include "ui_sdeditoption.h"

SDEditOption::SDEditOption(QString text, QString value, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SDEditOption)
{
    ui->setupUi(this);

    ui->label->setText(text);
    ui->lineEdit->setText(value);
}

SDEditOption::~SDEditOption()
{
    delete ui;
}

QString SDEditOption::getValue()
{
    return ui->lineEdit->text();
}
