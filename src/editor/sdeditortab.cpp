/*
 * SimDSP editor tabs.
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

#include "sdeditortab.h"
#include "ui_sdeditortab.h"

SDEditortab::SDEditortab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SDEditortab)
{
    ui->setupUi(this);

    connect(ui->tabWidget, &QTabWidget::tabCloseRequested, this, &SDEditortab::tabCloseRequested);

    font.setFamily("Consolas");
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    font.setPointSize(10);
}

SDEditortab::~SDEditortab()
{
    delete ui;
}

void SDEditortab::newFile()
{
    SDEditor *editor = createEditor();

    setFont(editor);
    editor->newFile();

    highlighter = new Highlighter(editor->document());
    //textEdit = qobject_cast<QTextEdit*>(ui->tabWidget->currentWidget());

    ui->tabWidget->insertTab(ui->tabWidget->count(),editor,QString(editor->userFriendlyCurrentFile()));
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);

    connect(editor->document(), &QTextDocument::contentsChanged, this, &SDEditortab::asterisk);
}

void SDEditortab::newFile(QString fileName)
{
    SDEditor *editor = createEditor();

    setFont(editor);
    editor->newFile(fileName);

    highlighter = new Highlighter(editor->document());
    //textEdit = qobject_cast<QTextEdit*>(ui->tabWidget->currentWidget());

    ui->tabWidget->insertTab(ui->tabWidget->count(),editor,QString(editor->userFriendlyCurrentFile()));
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);

    connect(editor->document(), &QTextDocument::contentsChanged, this, &SDEditortab::asterisk);
}


bool SDEditortab::openFile(const QString &fileName)
{
    if (findFile(fileName)) {
        ui->tabWidget->setCurrentIndex(findTab);
        return true;
    }

    const bool succeeded = loadFile(fileName);

    return succeeded;
}

bool SDEditortab::findFile(const QString &fileName)
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    if(!fileName.isEmpty()){
        for(int i=0;i < ui->tabWidget->count();i++)
        {
            SDEditor *editor = qobject_cast<SDEditor *>(ui->tabWidget->widget(i));
            if (editor->currentFile() == canonicalFilePath){
                findTab = i;
                return true;
            }
        }
    }
      return 0;
}


bool SDEditortab::loadFile(const QString &fileName)
{
    SDEditor *editor = createEditor();

    ui->tabWidget->insertTab(ui->tabWidget->count(),editor,QFileInfo(fileName).fileName());
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);

    setFont(editor);

    highlighter = new Highlighter(editor->document());
    //textEdit = qobject_cast<SDEditor*>(ui->tabWidget->currentWidget());

    const bool succeeded = editor->loadFile(fileName);
    if (succeeded)
        editor->show();
    else
        editor->close();

    connect(editor->document(), &QTextDocument::contentsChanged, this, &SDEditortab::asterisk);

    return succeeded;
}

void SDEditortab::saveFile()
{
    if( activeEditor() && activeEditor()->save() ){
        if (QFileInfo(activeEditor()->currentFile()).fileName() == "dsp_code.cpp")
            ui->tabWidget->setTabText(ui->tabWidget->currentIndex(), "dsp_code");
        else
            ui->tabWidget->setTabText(ui->tabWidget->currentIndex(),
                                      QFileInfo(activeEditor()->currentFile()).fileName());
    }
}

void SDEditortab::asterisk()
{
    SDEditor *editor = activeEditor();

    if(editor->isFileLoad()){
        QString nameFile = ui->tabWidget->tabText(ui->tabWidget->currentIndex());
        if(nameFile.endsWith("*"))
           ui->tabWidget->setTabText(ui->tabWidget->currentIndex(),nameFile);
        else
           ui->tabWidget->setTabText(ui->tabWidget->currentIndex(),nameFile.insert(nameFile.size(),"*"));
    }

    editor->isFileLoad(true);
}

SDEditor *SDEditortab::activeEditor()
{
    if ((ui->tabWidget->currentWidget()) != NULL)
        return qobject_cast<SDEditor *>(ui->tabWidget->currentWidget());
    return 0;
}

SDEditor *SDEditortab::createEditor()
{
    SDEditor *editor = new SDEditor;

#ifndef QT_NO_CLIPBOARD
    //connect(editor, &QTextEdit::copyAvailable, ui->action_Cut, &QAction::setEnabled);
    //connect(editor, &QTextEdit::copyAvailable, ui->action_Copy, &QAction::setEnabled);
#endif

    return editor;
}

void SDEditortab::tabCloseRequested()
{
    tabClose();
}

void SDEditortab::tabClose()
{
    SDEditor *editor = activeEditor();

    bool close = editor->closeEditor();

    if( close ){
        ui->tabWidget->removeTab(ui->tabWidget->currentIndex());
    }

}

void SDEditortab::setFont(SDEditor *editor)
{
    editor->setFont(font);

    const int tabStop = 4;

    QFontMetrics metrics(font);
    editor->setTabStopWidth(tabStop * metrics.width(' '));
}

void SDEditortab::saveAll()
{
    for( int index=0; index < ui->tabWidget->count(); index++ ){
        SDEditor *tab = reinterpret_cast<SDEditor*>(ui->tabWidget->widget(index));
        tab->save();
        if (QFileInfo(tab->currentFile()).fileName() == "dsp_code.cpp")
            ui->tabWidget->setTabText(index, "dsp_code");
        else
            ui->tabWidget->setTabText(index, QFileInfo(tab->currentFile()).fileName());
    }
}

void SDEditortab::closeAll()
{
    for( int index=0; index < ui->tabWidget->count(); index++ ){
        reinterpret_cast<SDEditor*>(ui->tabWidget->widget(index))->closeEditor();
    }
}

void SDEditortab::setClosable(bool closable)
{
    if( closable )
        ui->tabWidget->tabBar()->tabButton(ui->tabWidget->currentIndex(), QTabBar::RightSide)->show();
    else
        ui->tabWidget->tabBar()->tabButton(ui->tabWidget->currentIndex(), QTabBar::RightSide)->hide();
}


void SDEditortab::setFontAll()
{
    for( int index=0; index < ui->tabWidget->count(); index++ ){
        setFont(reinterpret_cast<SDEditor*>(ui->tabWidget->widget(index)));
    }
}

void SDEditortab::increaseFontSize()
{
    font.setPointSize(font.pointSize()+1);
    setFontAll();
}

void SDEditortab::decreaseFontSize()
{
    if( font.pointSize() > 8 )
        font.setPointSize(font.pointSize()-1);
    setFontAll();
}

void SDEditortab::resetFontSize()
{
    font.setPointSize(10);
    setFontAll();
}

void SDEditortab::selectFont()
{
    bool ok;
    font =QFontDialog::getFont(&ok, this);
    if(ok){
        setFontAll();
    }else{
        return;
    }
}
