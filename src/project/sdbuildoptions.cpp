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

#include "sdbuildoptions.h"
#include "ui_sdbuildoptions.h"

SDBuildOptions::SDBuildOptions(QString path, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SDBuildOptions)
{
    ui->setupUi(this);

    QDir currentPath(path);
    QString mkName = currentPath.dirName()+".mk";

    mkFile.setFileName(mkName);
    if( !mkFile.exists() ){
        QFile::copy(":/resources/templates/Makefile_simdsp", mkName);
        mkFile.setPermissions(QFile::WriteUser | QFile::ReadUser);
    }

    loadOptions();

#if QT_VERSION >= 0x050700
    connect(ui->addFlags, &QPushButton::clicked, this, &SDBuildOptions::addFlags);
    connect(ui->editFlags, &QPushButton::clicked, this, &SDBuildOptions::editFlags);
    connect(ui->removeFlags, &QPushButton::clicked, this, &SDBuildOptions::removeFlags);
    connect(ui->addInclude, &QPushButton::clicked, this, &SDBuildOptions::addInclude);
    connect(ui->editInclude, &QPushButton::clicked, this, &SDBuildOptions::editInclude);
    connect(ui->removeInclude, &QPushButton::clicked, this, &SDBuildOptions::removeInclude);
    connect(ui->addLibrary, &QPushButton::clicked, this, &SDBuildOptions::addLibrary);
    connect(ui->editLibrary, &QPushButton::clicked, this, &SDBuildOptions::editLibrary);
    connect(ui->removeLibrary, &QPushButton::clicked, this, &SDBuildOptions::removeLibrary);
    connect(ui->addLib, &QPushButton::clicked, this, &SDBuildOptions::addLib);
    connect(ui->editLib, &QPushButton::clicked, this, &SDBuildOptions::editLib);
    connect(ui->removeLib, &QPushButton::clicked, this, &SDBuildOptions::removeLib);
    connect(ui->comboBoxStd, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SDBuildOptions::changeStd);
#else
    connect(ui->addInclude, SIGNAL(clicked()), this, SLOT(addInclude()));
    connect(ui->editInclude, SIGNAL(clicked()), this, SLOT(editInclude()));
    connect(ui->removeInclude, SIGNAL(clicked()), this, SLOT(removeInclude()));
    connect(ui->addLibrary, SIGNAL(clicked()), this, SLOT(addLibrary()));
    connect(ui->editLibrary, SIGNAL(clicked()), this, SLOT(editLibrary()));
    connect(ui->removeLibrary, SIGNAL(clicked()), this, SLOT(removeLibrary()));
    connect(ui->addLib, SIGNAL(clicked()), this, SLOT(addLib()));
    connect(ui->editLib, SIGNAL(clicked()), this, SLOT(editLib()));
    connect(ui->removeLib, SIGNAL(clicked()), this, SLOT(removeLib()));
    connect(ui->comboBoxStd, SIGNAL(currentIndexChanged(int)), this, SLOT(changeStd()));
#endif

}

SDBuildOptions::~SDBuildOptions()
{
    delete ui;
}

void SDBuildOptions::loadOptions()
{
    QStringList incPath = searchOption("INCPATH").split(" ");
    foreach (QString path, incPath) {
        path.remove("-I");
        if(!path.isNull() && !path.isEmpty())
            ui->listWidgetInclude->addItem(path);
    }

    QStringList libsPath = searchOption("LIBPATH").split(" ");
    foreach (QString path, libsPath) {
        path.remove("-L");
        if(!path.isNull() && !path.isEmpty())
            ui->listWidgetLibraries->addItem(path);
    }

    QStringList libs = searchOption("LIBS").split(" ");
    foreach (QString lib, libs) {
        lib.remove("-l");
        if(!lib.isNull() && !lib.isEmpty())
            ui->listWidgetLib->addItem(lib);
    }

    QStringList flags = searchOption("EXTRA_FLAGS").split(" ");
    foreach (QString flag, flags) {
        if(!flag.isNull() && !flag.isEmpty())
            ui->listWidgetFlags->addItem(flag);
    }

    QStringList cxxFlags = searchOption("CXXFLAGS").split(" ");
    foreach (QString flag, cxxFlags) {
        QStringList std = {"c++11", "c++14", "c++0x", "c++98"};
        int index = flag.indexOf(QRegExp("c[+][+][a-z0-9][a-z0-9]"));
        if(index > 0){
            int option = std.indexOf( flag.mid(index, 5) );
            ui->comboBoxStd->setCurrentIndex(option);
        }
    }
}

QString SDBuildOptions::searchOption(QString option)
{
    mkFile.open(QIODevice::ReadOnly);
    QTextStream in(&mkFile);
    QString line;

    do {
        line = in.readLine();
        if (line.contains(option, Qt::CaseSensitive)) {
            break;
        }
    } while (!line.isNull());

    mkFile.close();

    if(!line.isNull()){
        int n = line.indexOf("=")+1;
        line = line.mid(n);
    }

    return line.simplified();
}

void SDBuildOptions::changeStd()
{
    const QStringList std = {"c++11", "c++14", "c++0x", "c++98"};
    int index = ui->comboBoxStd->currentIndex();
    QString value = std[index];

    mkFile.open(QIODevice::ReadWrite);
    QTextStream textStream(&mkFile);
    QString fileContents = textStream.readAll();
    mkFile.close();

    fileContents.replace(QRegExp("c[+][+][a-z0-9][a-z0-9]"), value);

    mkFile.open(QIODevice::WriteOnly);
    mkFile.write(fileContents.toLatin1());
    mkFile.close();
}

void SDBuildOptions::addFlags()
{
    SDEditOption* edit = new SDEditOption("Flag:");
    if(edit->exec()){
        QString value = edit->getValue();
        if(!value.isEmpty()){
            ui->listWidgetFlags->addItem(value);
            addOption("EXTRA_FLAGS", value);
        }
    }
}

void SDBuildOptions::editFlags()
{
    QList<QListWidgetItem*> items = ui->listWidgetFlags->selectedItems();

    ui->listWidgetFlags->clearSelection();

    if(!items.empty()){
        QListWidgetItem* item = items[0];
        QString oldValue = item->text();
        SDEditOption* edit = new SDEditOption("Flag:", oldValue);
        if(edit->exec()){
            QString newValue = edit->getValue();
            if(!newValue.isEmpty()){
                item->setText(newValue);
                editOption("EXTRA_FLAGS", oldValue, newValue);
            }
        }
    }
}

void SDBuildOptions::removeFlags()
{
    QList<QListWidgetItem*> items = ui->listWidgetFlags->selectedItems();

    foreach (QListWidgetItem* item, items) {
        removeOption("EXTRA_FLAGS", item->text());
        delete ui->listWidgetFlags->takeItem(ui->listWidgetFlags->row(item));
    }
}

void SDBuildOptions::addInclude()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Add Include Path"));
    if(!path.isEmpty()){
        addOption("INCPATH", path, "-I");
        ui->listWidgetInclude->addItem(path);
    }
}

void SDBuildOptions::editInclude()
{
    QList<QListWidgetItem*> items = ui->listWidgetInclude->selectedItems();

    ui->listWidgetInclude->clearSelection();

    if(!items.empty()){
        QString path = QFileDialog::getExistingDirectory(this, tr("Add Include Path"));

        if(!path.isEmpty()){
            editOption("INCPATH", items[0]->text(), path);
            items[0]->setText(path);
        }
        ui->listWidgetInclude->setCurrentItem(items[0]);
    }
}

void SDBuildOptions::removeInclude()
{
    QList<QListWidgetItem*> items = ui->listWidgetInclude->selectedItems();

    foreach (QListWidgetItem* item, items) {
        removeOption("INCPATH", item->text(), "-I");
        delete ui->listWidgetInclude->takeItem(ui->listWidgetInclude->row(item));
    }
}

void SDBuildOptions::addLibrary()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Add Library Path"));
    if(!path.isEmpty()){
        addOption("LIBPATH", path, "-L");
        ui->listWidgetLibraries->addItem(path);
    }
}

void SDBuildOptions::editLibrary()
{
    QList<QListWidgetItem*> items = ui->listWidgetLibraries->selectedItems();

    ui->listWidgetLibraries->clearSelection();

    if(!items.empty()){
            QString path = QFileDialog::getExistingDirectory(this, tr("Add Library Path"));

        if(!path.isEmpty()){
            editOption("LIBPATH", items[0]->text(), path);
            items[0]->setText(path);
        }
        ui->listWidgetLibraries->setCurrentItem(items[0]);
    }
}

void SDBuildOptions::removeLibrary()
{
    QList<QListWidgetItem*> items = ui->listWidgetLibraries->selectedItems();

    foreach (QListWidgetItem* item, items) {
        removeOption("LIBPATH", item->text(), "-L");
        delete ui->listWidgetLibraries->takeItem(ui->listWidgetLibraries->row(item));
    }
}

void SDBuildOptions::addLib()
{
    SDEditOption* edit = new SDEditOption("Link Library:");
    if(edit->exec()){
        QString value = edit->getValue();
        if(!value.isEmpty()){
            ui->listWidgetLib->addItem(value);
            addOption("LIBS", value, "-l");
        }
    }
}

void SDBuildOptions::editLib()
{
    QList<QListWidgetItem*> items = ui->listWidgetLib->selectedItems();

    ui->listWidgetLib->clearSelection();

    if(!items.empty()){
        QListWidgetItem* item = items[0];
        QString oldValue = item->text();
        SDEditOption* edit = new SDEditOption("Link Library:", oldValue);
        if(edit->exec()){
            QString newValue = edit->getValue();
            if(!newValue.isEmpty()){
                item->setText(newValue);
                editOption("LIBS", oldValue, newValue);
            }
        }
    }
}

void SDBuildOptions::removeLib()
{
    QList<QListWidgetItem*> items = ui->listWidgetLib->selectedItems();

    foreach (QListWidgetItem* item, items) {
        removeOption("LIBS", item->text(), "-l");
        delete ui->listWidgetLib->takeItem(ui->listWidgetLib->row(item));
    }
}

void SDBuildOptions::addOption(QString option, QString value, QString prefix)
{
    if(!value.isEmpty()){
        mkFile.open(QIODevice::ReadWrite);
        QList<QByteArray> lines = mkFile.readAll().split('\n');
        mkFile.close();

        for(int index=0; index<lines.size(); index++){
            if(lines[index].contains(option.toLatin1())){
                //lines[index].replace('\r', ' ');
                lines[index].append(" "+prefix+value);
                break;
            }
        }

        mkFile.open(QIODevice::WriteOnly);
        mkFile.write(lines.join('\n'));
        mkFile.close();
    }
}

void SDBuildOptions::editOption(QString option, QString oldValue, QString newValue)
{
    mkFile.open(QIODevice::ReadWrite);
    QList<QByteArray> lines = mkFile.readAll().split('\n');
    mkFile.close();

    oldValue.append(" ");
    newValue.append(" ");
    for(int index=0; index<lines.size(); index++){
        if(lines[index].contains(option.toLatin1())){
            lines[index].replace(lines[index].indexOf(oldValue), oldValue.size(), newValue.toLatin1());
            break;
        }
    }

    mkFile.open(QIODevice::WriteOnly);
    mkFile.write(lines.join('\n'));
    mkFile.close();
}

void SDBuildOptions::removeOption(QString option, QString value, QString prefix)
{
    if(!value.isEmpty()){
        value = prefix+value;
        mkFile.open(QIODevice::ReadWrite);
        QList<QByteArray> lines = mkFile.readAll().split('\n');
        mkFile.close();

        for(int index=0; index<lines.size(); index++){
            if(lines[index].contains(option.toLatin1())){
                lines[index].replace(lines[index].indexOf(value), value.size(), "");
                lines[index] = lines[index].simplified();
                break;
            }
        }

        mkFile.open(QIODevice::WriteOnly);
        mkFile.write(lines.join('\n'));
        mkFile.close();
    }
}
