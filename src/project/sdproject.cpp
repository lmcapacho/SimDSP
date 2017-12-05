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

#include "sdproject.h"

SDProject::SDProject(SDProjectexplorer *explorer,
                     SDEditortab *editorTab,
                     QTextEdit *appOutput)
{
    projectExplorer = explorer;
    output = appOutput;
    editor = editorTab;

    currentProject = 0;

    connect(projectExplorer, QOverload<QString,int>::of(&SDProjectexplorer::DoubleClickedFile), this, &SDProject::doubleClickedFile);
}

void SDProject::newProject(QString projectName, QString projectPath)
{
    path = projectPath;
    QString codePath = projectName+".cpp";

    QDir::setCurrent(path);

    QFile::copy(":/resources/templates/sdBaseTPL.txt", codePath);
    QFile code(codePath);
    code.setPermissions(QFile::WriteUser | QFile::ReadUser);

    builder = new SDBuilder();
    builder->create();

    connect(builder, QOverload<QByteArray>::of(&SDBuilder::errorOutput), this, &SDProject::builderOutput);
    connect(builder, QOverload<QByteArray>::of(&SDBuilder::msgOutput), this, &SDProject::builderOutput);

    int column = projectExplorer->addTreeRoot(projectName);
    projectExplorer->addTreeChild(projectName);
    projectExplorer->setExpanded(true);

    projects.append(Project(projectName, projectPath, column));

    editor->newFile(projectName);
    editor->activeEditor()->loadFile(projectName+".cpp");
}


bool SDProject::openProject(QString projectPath)
{
    QDir dir(projectPath);
    if (!dir.exists())
        return false;

    QFileInfo file(projectPath + QDir::separator() + "dsp_code.cpp" );
    if (!file.exists())
        return false;

    path =  projectPath;
    projectExplorer->removeTreeRoot();
    projectExplorer->addTreeRoot(dir.dirName());

    dir.setNameFilters(QStringList() << "*.cpp" << "*.h");
    foreach (QString f, dir.entryList(QDir::Files)) {
        projectExplorer->addTreeChild(f);
    }

    projectExplorer->setExpanded(true);

    QDir::setCurrent(path);

    editor->closeAll();
    editor->newFile("dsp_code");
    editor->activeEditor()->loadFile("dsp_code.cpp");
    editor->setClosable(false);

    return true;
}

void SDProject::closeProject()
{
    editor->closeAll();
    projectExplorer->removeTreeRoot();
}

bool SDProject::saveProject(QString projectName, QString projectPath)
{
    QDir dir(path);
    if (! dir.exists())
        return false;

    if(!dir.mkpath(projectPath))
        return false;

    foreach (QString f, dir.entryList(QDir::Files)) {
        QFile::copy(path + QDir::separator() + f, projectPath + QDir::separator() + projectName + QDir::separator() + f);
    }

    dir.rmpath(path);
    path = projectPath;
    projectExplorer->setTreeRootName(projectName);

    QDir::setCurrent(path + QDir::separator() + projectName);

    editor->closeAll();
    editor->newFile("dsp_code");
    editor->activeEditor()->loadFile("dsp_code.cpp");
    editor->setClosable(false);

    return true;
}

void SDProject::newFile(QString fileName)
{
    editor->openFile(fileName);
    projectExplorer->addTreeChild(fileName);
}

void SDProject::saveFile()
{
    editor->saveFile();
}

bool SDProject::buildProject()
{
    editor->saveAll();
    output->append(tr("<b>Compiling SimDSP Project...</b><br>"));

    if( !builder->build() ){
        output->append(tr("<span style='color:#AE0000;'><b>Failed compile SimDSP</b></span>"));
        return false;
    }else{
        output->append(tr("<span style='color:#017500;'><b>SimDSP compile successfully</b></span>"));
        return true;
    }
}

void SDProject::cleanProject()
{
    output->append(tr("<b>Cleaning SimDSP Project...</b><br>"));
    builder->clean();
    output->append(tr("<span style='color:#017500;'><b>Cleaned SimDSP Project</b><br></span>"));
}

void SDProject::builderOutput(QByteArray data)
{
    output->append(data);
}

void SDProject::doubleClickedFile(QString fileName, int column)
{
    editor->openFile(fileName);
}
