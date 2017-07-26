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

    connect(projectExplorer, QOverload<QString>::of(&SDProjectexplorer::DoubleClickedFile), this, &SDProject::doubleClickedFile);
}

void SDProject::newProject(QString projectName, QString projectPath)
{
    path = projectPath;
    QString codePath = "dsp_code.cpp";

    QDir::setCurrent(path);

    QFile::copy(":/files/sdBaseTPL.txt", codePath);
    QFile code(codePath);
    code.setPermissions(QFile::WriteUser | QFile::ReadUser);

    builder = new SDBuilder();
    builder->create();

    connect(builder, QOverload<QByteArray>::of(&SDBuilder::errorOutput), this, &SDProject::builderOutput);
    connect(builder, QOverload<QByteArray>::of(&SDBuilder::msgOutput), this, &SDProject::builderOutput);

    projectExplorer->addTreeRoot(projectName);
    projectExplorer->addTreeChild(projectName);
    projectExplorer->setExpanded(true);

    editor->newFile("dsp_code");
    editor->activeEditor()->loadFile("dsp_code.cpp");
    editor->setClosable(false);
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
    output->append(tr("Compiling SimDSP code..."));

    if( !builder->build() ){
        output->append(tr("Failed compile SimDSP"));
        return false;
    }else{
        output->append(tr("SimDSP compile successfully"));
        return true;
    }
}

void SDProject::builderOutput(QByteArray data)
{
    output->append(data);
}

void SDProject::doubleClickedFile(QString fileName)
{
    if(fileName != "dsp_code")
        editor->openFile(fileName);
    else
        editor->openFile("dsp_code.cpp");
}
