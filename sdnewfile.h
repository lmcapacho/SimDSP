/*
 * SimDSP new file.
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

#ifndef SDNEWFILE_H
#define SDNEWFILE_H

#include <QDialog>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QPushButton>
#include <QRegExp>
#include <QRegExpValidator>
#include <QDebug>

#define SOURCE 0
#define HEADER 1

namespace Ui {
class SDNewFile;
}

class SDNewFile : public QDialog
{
    Q_OBJECT

public:
    explicit SDNewFile(QWidget *parent = 0);
    ~SDNewFile();
    QString getFileName();

public slots:
    void changeTemplateSelect(int index);
    void changeFileName(const QString &fileName);
    void accepted();

private:
    void loadTemplate(int index);

    Ui::SDNewFile *ui;
};

#endif // SDNEWFILE_H
