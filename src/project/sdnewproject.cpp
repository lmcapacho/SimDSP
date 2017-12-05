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

#include "sdnewproject.h"
#include "ui_sdnewproject.h"

SDNewProject::SDNewProject(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SDNewProject)
{
    ui->setupUi(this);

    ui->projectPath->setText(QDir::homePath());
    ui->projectName->setFocus();

    connect(ui->browseButton, &QPushButton::clicked, this, &SDNewProject::browsePath);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SDNewProject::accepted);
}

SDNewProject::~SDNewProject()
{
    delete ui;
}

void SDNewProject::getProjectInfo(QString &name, QString &path)
{
    path = ui->projectPath->text();
    name = ui->projectName->text();
}

void SDNewProject::browsePath()
{
    QString path = QFileDialog::getExistingDirectory(this,
            tr("Save SimDSP Project"),QDir::homePath(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    ui->projectPath->setText(path);
}


void SDNewProject::accepted()
{
    QDir dir(ui->projectPath->text());

    if(dir.exists()){
        dir.mkpath(ui->projectPath->text()+"/"+ui->projectName->text());
    }
}
