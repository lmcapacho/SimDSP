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
    explicit SDEditLib(QString value = QString::null, QWidget *parent = nullptr);
    ~SDEditLib();

    QString getValue();

private:
    Ui::SDEditLib *ui;
};

#endif // SDEDITLIB_H
