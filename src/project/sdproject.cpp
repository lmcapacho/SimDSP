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

    builder = new SDBuilder();

    connect(builder, QOverload<QByteArray>::of(&SDBuilder::errorOutput), this, &SDProject::builderOutput);
    connect(builder, QOverload<QByteArray>::of(&SDBuilder::msgOutput), this, &SDProject::builderOutput);
    connect(projectExplorer, QOverload<QString>::of(&SDProjectexplorer::itemActivated), this, &SDProject::itemActivated);
    connect(projectExplorer, QOverload<QString>::of(&SDProjectexplorer::deleteFile), this, &SDProject::removeFile);
}

void SDProject::newProject(QString projectName, QString projectPath)
{
    path = projectPath;
    QString mainFile = projectName+".cpp";

    QDir::setCurrent(path);

    QFile::copy(":/resources/templates/sdBaseTPL.txt", mainFile);
    QFile code(mainFile);
    code.setPermissions(QFile::WriteUser | QFile::ReadUser);

    builder->create();

    projectExplorer->addProject(projectPath, projectName);
    projectExplorer->addFile(projectName);
    projectExplorer->setExpanded(true);

    editor->newFile(projectName);
    editor->activeEditor()->loadFile(projectName+".cpp");
}


bool SDProject::openProject(QString projectPath)
{
    QDir dir(projectPath);
    if (!dir.exists())
        return false;

    QString projectName = QDir(projectPath).dirName();

    QString oldFile = projectPath + QDir::separator() + "dsp_code.cpp";
    QString filePath = projectPath + QDir::separator() + projectName + ".cpp";

    QFileInfo file(oldFile);
    if (file.exists()){
        QFile::rename(oldFile, filePath);
    }

    file.setFile(filePath);
    if (!file.exists()){
        QMessageBox::information(0, "SimDSP", tr("There is not a project in this folder\n"));
        return false;
    }

    QString mainFile = dir.dirName()+".cpp";

    path =  projectPath;
    projectExplorer->addProject(projectPath, dir.dirName());

    dir.setNameFilters(QStringList() << "*.cpp" << "*.h");
    foreach (QString f, dir.entryList(QDir::Files)) {
        QFileInfo fi(f);
        if( fi.baseName() == dir.dirName() && fi.suffix() == "cpp" )
            projectExplorer->addFile(fi.baseName());
        else
            projectExplorer->addFile(f);
    }

    projectExplorer->setExpanded(true);
    projectExplorer->sortProject();

    QDir::setCurrent(path);

    editor->newFile(dir.dirName());
    editor->activeEditor()->loadFile(mainFile);

    return true;
}

int SDProject::closeProject()
{
    int index = projectExplorer->removeProject();
    path = QDir::currentPath();
    return index;
}

void SDProject::closeAll()
{
    editor->closeAll();
    projectExplorer->removeAllProjects();
}

void SDProject::newFile(QString fileName)
{
    editor->openFile(fileName);
    projectExplorer->addFile(fileName);
}

void SDProject::removeFile(QString fileName)
{
    editor->openFile(fileName);
    editor->tabClose();
}

void SDProject::saveFile()
{
    editor->saveFile();
}

int SDProject::closeTab()
{
    return editor->tabClose();
}

void SDProject::closeAllTabs()
{
    editor->closeAll();
}

bool SDProject::buildProject()
{
    editor->saveAll();
    output->clear();
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
    output->clear();
    output->append(tr("<b>Cleaning SimDSP Project...</b><br>"));
    builder->clean();
    output->append(tr("<span style='color:#017500;'><b>Cleaned SimDSP Project</b><br></span>"));
}

void SDProject::builderOutput(QByteArray data)
{
    output->append(data);
}

void SDProject::itemActivated(QString fileName)
{
    QFileInfo fi(fileName);
    QString currentPath = QDir::currentPath();
    QDir::setCurrent(fi.canonicalPath());
    editor->openFile(fi.fileName());
    QDir::setCurrent(currentPath);

    emit tabOpen();
}
