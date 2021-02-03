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

SDProject::SDProject(SDProjectexplorer *explorer, SDEditortab *editorTab,
                     QTextEdit *appOutput, QTextEdit *issuesOutput)
{
    projectExplorer = explorer;
    output = appOutput;
    editor = editorTab;
    issues = issuesOutput;

    builder = new SDBuilder();

    msgRegExp.setPattern(QLatin1String("^(([A-Za-z]:)?[^:]+):(\\d+):(\\d+:)?\\s+((fatal |#)?(warning|error|note):?\\s)?([^\\s].+)$"));
    ldRegExp.setPattern(QLatin1String("^(([A-Za-z]:)?[^:]+):?\\s+((fatal|warning|error|note):?\\s)+([A-Za-z])"));

#if QT_VERSION >= 0x050700
    connect(builder, QOverload<QByteArray>::of(&SDBuilder::builderOutput), this, &SDProject::builderOutput);
    connect(projectExplorer, QOverload<QString, bool>::of(&SDProjectexplorer::itemActivated), this, &SDProject::itemActivated);
    connect(projectExplorer, QOverload<QString>::of(&SDProjectexplorer::deleteFile), this, &SDProject::removeFile);
#else
    connect(builder, SIGNAL(builderOutput(QByteArray)), this, SLOT(builderOutput(QByteArray)));
    connect(projectExplorer, SIGNAL(itemActivated(QString,bool)), this, SLOT(itemActivated(QString,bool)));
    connect(projectExplorer, SIGNAL(deleteFile(QString)), this, SLOT(removeFile(QString)));
#endif
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
        QMessageBox::information(nullptr, "SimDSP", tr("There is not a project in this folder\n"));
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

    addRecentProjects(path);

    return true;
}

bool SDProject::openExample(QString examplePath)
{
    QDir dir(examplePath);
    if (!dir.exists())
        return false;

    QString exampleName = QDir(examplePath).dirName();
    QString filePath = examplePath + QDir::separator() + exampleName + ".cpp";

    QFileInfo file(filePath);
    if (!file.exists()){
        QMessageBox::information(nullptr, "SimDSP", tr("There is not an example in this folder\n"));
        return false;
    }

    QString mainFile = dir.dirName()+".cpp";

    path =  examplePath;
    projectExplorer->addExample(path, dir.dirName());

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
    editor->activeEditor()->loadFile(mainFile, true);

    return true;
}

int SDProject::closeActiveProject()
{
    int index = projectExplorer->removeActiveProject();
    path = QDir::currentPath();
    return index;
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

void SDProject::addRecentProjects(QString path)
{
    QSettings settings;
    QStringList recentsProjects;
    recentsProjects = settings.value("Projects/recent").toStringList();
    if(recentsProjects.contains(path))
        recentsProjects.removeAt(recentsProjects.indexOf(path));

    recentsProjects.append(path);

    if(recentsProjects.length()>5)
        recentsProjects.removeFirst();

    settings.setValue("Projects/recent", recentsProjects);
}

bool SDProject::buildProject()
{
    editor->saveAll();
    issues->clear();
    output->clear();
    totalIssues = 0;
    output->append(tr("<b style='font-size: 10pt;'>Compiling SimDSP Project...</b><br>"));

    if( !builder->build() ){
        output->append(tr("<span style='color:#AE0000; font-size: 10pt;'><b>Failed compile SimDSP</b></span>"));
        return false;
    }else{
        output->append(tr("<span style='color:#017500; font-size: 10pt;'><b>SimDSP compile successfully</b></span>"));
        return true;
    }
}

void SDProject::cleanProject()
{
    issues->clear();
    output->clear();
    output->append(tr("<b>Cleaning SimDSP Project...</b><br>"));
    builder->clean();
    output->append(tr("<span style='color:#017500; font-size: 10pt;'><b>Cleaned SimDSP Project</b><br></span>"));
}

void SDProject::builderOutput(QByteArray data)
{
    QString dataString(data);
    QStringList outputLines = dataString.split(QRegExp("[\r\n]"));

    for(int i=0; i<outputLines.size(); i++){
        QRegularExpressionMatch matchMsg = msgRegExp.match(outputLines[i]);
        QRegularExpressionMatch matchLd = ldRegExp.match(outputLines[i]);

        if(matchMsg.hasMatch() || matchLd.hasMatch()){
            QStringList issue = outputLines[i].split(QRegExp(":(?!:)"));

            if(outputLines[i].contains("collect2")){
                QString msg = issue[0] + ":" + issue[1] + ": " + issue[2] ;
                issues->append("<span style='color:#AE0000; font-size: 10pt;'>"+msg+"</span>");
                totalIssues++;
            }
            else if(outputLines[i].contains("error")){
                QString msg = issue[0].remove("../") + ":" + issue[1] + ": " + issue[4] ;
                if(issue.size()>5)
                    msg += issue[5];
                issues->append("<span style='color:#AE0000; font-size: 10pt;'>"+msg+"</span>");
                totalIssues++;
            }
            else if(outputLines[i].contains("warning")){
                QString msg = issue[0].remove("../") + ":" + issue[1] + ": " + issue[4] ;
                issues->append("<span style='color:#AD6D11; font-size: 10pt;'>"+msg+"</span>");
                totalIssues++;
            }

            if(totalIssues > 0)
                emit buildIssues(totalIssues);
        }
    }
    output->append(data);
}

void SDProject::itemActivated(QString fileName, bool isExample)
{
    QString name;
    QFileInfo fi(fileName);
    if( fi.suffix().isNull() )
        name = fileName+".cpp";
    else
        name = fileName;

    QString currentPath = QDir::currentPath();
    QString canonicalPath = QFileInfo(name).canonicalPath();
    QDir::setCurrent(canonicalPath);
    editor->openFile(fi.fileName(), isExample);
    QDir::setCurrent(currentPath);

    emit tabOpen();
}
