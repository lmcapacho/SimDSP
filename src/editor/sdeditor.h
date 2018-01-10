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

#include <QCompleter>
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QStringListModel>
#include <QScrollBar>
#include <QPlainTextEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QApplication>
#include <QPainter>
#include <QTextBlock>
#include <QDebug>

class SDEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    SDEditor();

    void newFile(QString fileName);
    void loadTemplate(QString fileName);
    bool loadFile(const QString &fileName);
    bool save();
    bool saveAs();
    bool saveFile(const QString &fileName);
    QString userFriendlyCurrentFile();
    QString currentFile() { return curFile; }
    bool isFileLoad();
    void isFileLoad(bool load);
    bool closeEditor();

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    void setFont(QFont &);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void documentWasModified();
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &, int);
    void insertCompletion(const QString &completion);

private:
    bool maybeSave();
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);
    QAbstractItemModel *modelFromFile(const QString& fileName);
    QString textUnderCursor() const;

    QCompleter *completer;
    QWidget *lineNumberArea;

    QFont font;

    QString curFile;
    bool isUntitled;
    bool isLoad;
};

// Line Number Area
class SDEditorLNA : public QWidget
{

public:
    SDEditorLNA(SDEditor *sdeditor) : QWidget(sdeditor) {
        sdEditor = sdeditor;
    }

    QSize sizeHint() const override {
        return QSize(sdEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        sdEditor->lineNumberAreaPaintEvent(event);
    }

private:
    SDEditor *sdEditor;
};

#endif // SDEditor_H
