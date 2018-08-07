/*
 * SimDSP build options.
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
