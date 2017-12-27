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

#ifndef SDEditortab_H
#define SDEditortab_H

#include <QObject>
#include <QMainWindow>
#include <QTextEdit>
#include <QDebug>
#include <QFontDialog>

#include "sdeditor.h"
#include "highlighter.h"

namespace Ui {
class SDEditortab;
}

class SDEditor;
class Highlighter;

class SDEditortab : public QWidget
{
    Q_OBJECT

public:
    explicit SDEditortab(QWidget *parent = 0);
    ~SDEditortab();
    void newFile(QString fileName);
    bool openFile(QString fileName);
    SDEditor *createEditor();
    SDEditor *activeEditor();
    void asterisk();
    int tabClose();
    void saveFile();
    void saveAll();
    void closeAll();
    void setClosable(bool closable);
    void increaseFontSize();
    void decreaseFontSize();
    void resetFontSize();
    void selectFont();

private slots:
    void tabCloseRequested();

private:
    bool findFile(QString fileName);
    bool loadFile(QString fileName);
    void setFont(SDEditor *editor);
    void setFontAll();

    QFont font;

    Ui::SDEditortab *ui;

    Highlighter *highlighter;
    //QTextEdit *textEdit;
    int findTab;
};

#endif // SDEditortab_H
