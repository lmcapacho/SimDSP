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

#include "sdeditor.h"

SDEditor::SDEditor()
{
    setAttribute(Qt::WA_DeleteOnClose);
    isUntitled = true;
}

void SDEditor::newFile()
{
    static int sequenceNumber = 1;

    isUntitled = true;
    curFile = tr("untitled%1").arg(sequenceNumber++);

    connect(document(), &QTextDocument::contentsChanged, this, &SDEditor::documentWasModified);
}

void SDEditor::newFile(QString fileName)
{
    isUntitled = false;
    curFile = fileName;

    connect(document(), &QTextDocument::contentsChanged, this, &SDEditor::documentWasModified);
}

void SDEditor::documentWasModified()
{
    setWindowModified(document()->isModified());
}

bool SDEditor::save()
{
    if (isUntitled) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

bool SDEditor::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),curFile);
    if (fileName.isEmpty())
        return false;
    return saveFile(fileName);
}

bool SDEditor::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("SimDSP"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << toPlainText();
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    return true;
}


bool SDEditor::maybeSave()
{
    if (!document()->isModified())
        return true;

    const QMessageBox::StandardButton ret
            = QMessageBox::warning(this, tr("SimDSP"),
                                   tr("'%1' has been modified.\n"
                                      "Do you want to save your changes?")
                                   .arg(userFriendlyCurrentFile()),
                                   QMessageBox::Save | QMessageBox::Discard
                                   | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        return save();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}


void SDEditor::setCurrentFile(const QString &fileName)
{
    curFile = QFileInfo(fileName).canonicalFilePath();
    isUntitled = false;
    document()->setModified(false);
    setWindowModified(false);
}


bool SDEditor::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("SimDSP"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    setPlainText(in.readAll());
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);

    connect(document(), &QTextDocument::contentsChanged,
            this, &SDEditor::documentWasModified);

    return true;
}

void SDEditor::loadTemplate(QString fileName)
{
    QFile file(fileName);
    file.open(QFile::ReadOnly | QFile::Text);

    QTextStream in(&file);
    setPlainText(in.readAll());
}

QString SDEditor::userFriendlyCurrentFile()
{
    return strippedName(curFile);
}

bool SDEditor::closeEditor()
{
    if (maybeSave()) {
        this->close();
        return true;
    } else {
        return false;
    }
}

QString SDEditor::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

bool SDEditor::isFileLoad()
{
    return isLoad;
}

void SDEditor::isFileLoad(bool load)
{
    isLoad = load;
}
