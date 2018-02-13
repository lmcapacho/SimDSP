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

#include "sdnewfile.h"
#include "ui_sdnewfile.h"

SDNewFile::SDNewFile(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SDNewFile)
{
    ui->setupUi(this);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    loadTemplate(SOURCE);

#if QT_VERSION >= 0x050700
    connect(ui->templateSelect, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SDNewFile::changeTemplateSelect);
    connect(ui->fileName, QOverload<const QString&>::of(&QLineEdit::textChanged), this, &SDNewFile::changeFileName);
#else
    connect(ui->templateSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(changeTemplateSelect(int)));
    connect(ui->fileName, SIGNAL(textChanged(QString)), this, SLOT(changeFileName(QString)));
#endif
    connect(ui->buttonBox,&QDialogButtonBox::accepted, this, &SDNewFile::accepted);
}

SDNewFile::~SDNewFile()
{
    delete ui;
}

void SDNewFile::loadTemplate(int index)
{
    QFile file;

    if( index == SOURCE )
        file.setFileName(":/resources/templates/sdSourceTPL.txt");
    else
        file.setFileName(":/resources/templates/sdHeaderTPL.txt");

    file.open(QFile::ReadOnly | QFile::Text);

    QTextStream in(&file);
    ui->preview->setPlainText(in.readAll());
}

void SDNewFile::changeTemplateSelect(int index)
{
    loadTemplate(index);
    changeFileName(ui->fileName->text());
}

void SDNewFile::changeFileName(const QString &fileName)
{
    QRegExp rx;

    if( ui->templateSelect->currentIndex() == SOURCE )
        rx.setPattern("\\w+\\.cpp");
    else
        rx.setPattern("\\w+\\.h");


    QValidator *validator = new QRegExpValidator(rx, this);
    ui->path->setText(QDir::currentPath() + QDir::separator() + fileName);

    int pos = 0;
    if( validator->validate((QString&)fileName, pos) == QValidator::Acceptable ){
        QFileInfo fi(QDir::currentPath() + QDir::separator() + fileName);
        if( fi.exists() ){
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
            ui->path->setText("The file exists in this project");
        } else{
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        }
    }
    else{
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
}

void SDNewFile::accepted()
{
    QString fileName = ui->fileName->text();

    if( ui->templateSelect->currentIndex() == SOURCE )
        QFile::copy(":/resources/templates/sdSourceTPL.txt", fileName);
    else
        QFile::copy(":/resources/templates/sdHeaderTPL.txt", fileName);

    QFile code(fileName);
    code.setPermissions(QFile::WriteUser | QFile::ReadUser);
}

QString SDNewFile::getFileName()
{
   return ui->fileName->text();
}
