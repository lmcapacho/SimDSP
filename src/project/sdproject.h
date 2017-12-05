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

#include "sdprojectexplorer.h"
#include "../actions/sdbuilder.h"
#include "../editor/sdeditortab.h"

class Project
{
public:
    Project(QString name, QString path, int column){projectName = name; projectPath = path; position=column;}
    QString projectName;
    QString projectPath;
    int position;
};

class SDProject : public QObject
{
    Q_OBJECT
public:
    explicit SDProject(SDProjectexplorer *explorer,
                       SDEditortab *editorTab,
                       QTextEdit *appOutput);
    void newProject(QString projectName, QString projectPath);
    bool openProject(QString projectPath);
    void closeProject();
    bool saveProject(QString projectName, QString projectPath);
    bool buildProject();
    void cleanProject();

    void newFile(QString fileName);
    void saveFile();

    SDEditortab *editor;

public slots:
    void builderOutput(QByteArray data);
    void doubleClickedFile(QString fileName, int column);

private:
    SDProjectexplorer *projectExplorer;
    SDBuilder *builder;

    QTextEdit *output;
    QString path;
    QList<Project> projects;
    int currentProject;
};

#endif // SDPROJECT_H
