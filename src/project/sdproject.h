/*
 * SimDSP project.
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

#ifndef SDPROJECT_H
#define SDPROJECT_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QTextEdit>
#include <QMessageBox>
#include <QRegExp>
#include <QSettings>
#include <QStringList>

#include "sdprojectexplorer.h"
#include "../actions/sdbuilder.h"
#include "../editor/sdeditortab.h"

class SDProject : public QObject
{
    Q_OBJECT
public:
    explicit SDProject(SDProjectexplorer *explorer,
                       SDEditortab *editorTab,
                       QTextEdit *appOutput,
                       QTextEdit *issuesOutput);
    void newProject(QString projectName, QString projectPath);
    bool openProject(QString projectPath);
    bool openExample(QString examplePath);
    int  closeProject();
    int  closeActiveProject();
    bool buildProject();
    void cleanProject();
    void closeAll();

    void newFile(QString fileName);
    void saveFile();
    int  closeTab();
    void closeAllTabs();

    void addRecentProjects(QString path);

    SDEditortab *editor;

signals:
    void tabOpen();
    void buildIssues(int total);

public slots:
    void builderOutput(QByteArray data);
    void itemActivated(QString fileName, bool isExample);
    void removeFile(QString fileName);

private:
    SDProjectexplorer *projectExplorer;
    SDBuilder *builder;

    QTextEdit *output;
    QTextEdit *issues;
    QString path;

    int totalIssues;
    QRegularExpression msgRegExp;
    QRegularExpression ldRegExp;
};

#endif // SDPROJECT_H
