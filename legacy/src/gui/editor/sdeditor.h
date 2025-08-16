/*
 * SimDSP editor.
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

#ifndef SDEditor_H
#define SDEditor_H

#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QTextCodec>
#include <QApplication>
#include <QDebug>

#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexercpp.h>
#include <Qsci/qsciapis.h>

class SDEditor : public QsciScintilla
{
    Q_OBJECT

public:
    SDEditor();

    void newFile(QString fileName);
    void loadTemplate(QString fileName);
    bool loadFile(const QString &fileName, bool readOnly=false);
    bool save();
    bool saveAs();
    bool saveFile(const QString &fileName);
    QString userFriendlyCurrentFile();
    QString currentFile() { return curFile; }
    bool isFileLoad();
    void isFileLoad(bool load);
    bool closeEditor();

    void setFont(QFont &);

private slots:
    void documentWasModified(bool m);
    void test();

private:
    bool isSaved();
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);

    QFont font;
    QsciLexerCPP *lexer;
    QsciAPIs *lexer_apis;

    QString curFile;
    bool isUntitled;
    bool isLoad;
};

#endif // SDEditor_H
