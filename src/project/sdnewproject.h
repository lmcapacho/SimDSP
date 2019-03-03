/*
 * SimDSP new project.
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

#ifndef SDNEWPROJECT_H
#define SDNEWPROJECT_H

#include <QDialog>
#include <QFileDialog>
#include <QDir>
#include <QDebug>

namespace Ui {
class SDNewProject;
}

class SDNewProject : public QDialog
{
    Q_OBJECT

public:
    explicit SDNewProject(QWidget *parent = nullptr);
    ~SDNewProject();
    void getProjectInfo(QString &name, QString &path);

public slots:
    void browsePath();
    void accepted();
    void changeLineEdit(const QString& value);

private:
    Ui::SDNewProject *ui;
};

#endif // SDNEWPROJECT_H
