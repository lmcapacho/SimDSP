/*
 * SimDSP main.
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

#include "simdsp.h"
#include "ui_simdsp.h"

SimDSP::SimDSP(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SimDSP)
{
    ui->setupUi(this);

    connect(ui->actionNewFile, &QAction::triggered, this, &SimDSP::actionNewFile);
    connect(ui->actionSaveFile, &QAction::triggered, this, &SimDSP::actionSaveFile);

    connect(ui->actionRun, &QAction::triggered, this, &SimDSP::actionRun);    
    connect(ui->actionStop, &QAction::triggered, this, &SimDSP::actionStop);
    connect(ui->actionLoad, &QAction::triggered, this, &SimDSP::actionLoad);
    connect(ui->actionSave, &QAction::triggered, this, &SimDSP::actionSave);

    connect(ui->actionNewProject, &QAction::triggered, this, &SimDSP::actionNewProject);
    connect(ui->actionSaveProject, &QAction::triggered, this, &SimDSP::actionSaveProject);
    connect(ui->actionOpenProject, &QAction::triggered, this, &SimDSP::actionOpenProject);
    connect(ui->actionBuild, &QAction::triggered, this, &SimDSP::actionBuild);

    connect(ui->actionIncreaseFontSize, &QAction::triggered, this, &SimDSP::actionIncreaseFontSize);
    connect(ui->actionDecreaseFontSize, &QAction::triggered, this, &SimDSP::actionDecreaseFontSize);
    connect(ui->actionResetFontSize, &QAction::triggered, this, &SimDSP::actionResetFontSize);
    connect(ui->actionFont, &QAction::triggered, this, &SimDSP::actionFont);

    connect(ui->actionHelpContents, &QAction::triggered, this, &SimDSP::actionHelpContents);

    connect(ui->actionQuit, &QAction::triggered, this, &SimDSP::close);    

    initSimDSP();
}

SimDSP::~SimDSP()
{
    delete ui;
}

void SimDSP::initSimDSP()
{
    ui->tabWidget->setTabEnabled(ui->tabWidget->indexOf(ui->runTab), false);

    ui->statusBar->showMessage(tr("Ready"));


    sdproject = new SDProject(ui->widgetProject,
                              ui->widgetEditor,
                              ui->appOutput);
    sdproject->newProject("dsp_code", tmpProjectPath.path());

    isProjectPathTmp = true;

    codeLibrary =  new QLibrary(QDir::currentPath()+"/build/libsdcode.so");

    SimDSPTimer = new QTimer();
    connect(SimDSPTimer, &QTimer::timeout, this, &SimDSP::loop);

    ui->actionStop->setDisabled(true);
    ui->actionLoad->setDisabled(true);
    ui->actionSave->setDisabled(true);

    sdproject->editor->addAction(ui->actionIncreaseFontSize);
    sdproject->editor->addAction(ui->actionDecreaseFontSize);
    sdproject->editor->addAction(ui->actionResetFontSize);
}

void SimDSP::closeEvent(QCloseEvent *event)
{
    if( isProjectPathTmp ){
        QMessageBox::StandardButton saveBtn = QMessageBox::question( this, "SimDSP",
                                                                     tr("Do you want to save the project?\n"),
                                                                     QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                                                     QMessageBox::Yes);
        if (saveBtn == QMessageBox::Yes) {
            SDNewProject* newProject = new SDNewProject(this);

            if(newProject->exec()){
                QString projectPath, projectName;
                newProject->getProjectInfo(projectName, projectPath);
                if( sdproject->saveProject(projectName, projectPath) ){
                  isProjectPathTmp = false;
                }
            }
        }
    }

    event->accept();
}

/******************************************
 * Public slots
 ******************************************/
void SimDSP::actionLoad()
{
    sdcore->loadFile();
}

void SimDSP::actionSave()
{
    sdcore->saveFile();
}

void SimDSP::actionNewFile()
{
    SDNewFile* newFile =  new SDNewFile(this);

    if(newFile->exec()){
        sdproject->newFile(newFile->getFileName());
    }
}

void SimDSP::actionSaveFile()
{
    sdproject->saveFile();
}

void SimDSP::actionNewProject()
{
    SDNewProject* newProject = new SDNewProject(this);

    if(newProject->exec()){
        QString projectPath, projectName;
        newProject->getProjectInfo(projectName, projectPath);
        sdproject->newProject(projectName, projectPath+"/"+projectName);
        isProjectPathTmp = false;
        codeLibrary->setFileName(QDir::currentPath()+"/build/libsdcode.so");
    }
}

void SimDSP::actionOpenProject()
{
    QString path = QFileDialog::getExistingDirectory(this,
            tr("Open SimDSP Project"),QDir::homePath(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if( sdproject->openProject(path) ){
        isProjectPathTmp = false;
        codeLibrary->setFileName(QDir::currentPath()+"/build/libsdcode.so");
    }
}

void SimDSP::actionBuild()
{
    ui->appOutput->clear();
    sdproject->buildProject();
}

void SimDSP::actionSaveProject()
{
    SDNewProject* newProject = new SDNewProject(this);

    if(newProject->exec()){
        QString projectPath, projectName;
        newProject->getProjectInfo(projectName, projectPath);
        if( sdproject->saveProject(projectName, projectPath) ){
          isProjectPathTmp = false;
          codeLibrary->setFileName(QDir::currentPath()+"/build/libsdcode.so");
        }
    }
}

void SimDSP::actionRun()
{
    ui->appOutput->clear();
    if(sdproject->buildProject()){

        ui->appOutput->clear();

        initFunction dsp_init = (initFunction) codeLibrary->resolve("dsp_init");
        if(dsp_init){
            dsp_init();
        }else{
            ui->appOutput->append(codeLibrary->errorString());
            if( codeLibrary->isLoaded() ) codeLibrary->unload();
            return;
        }

        getSDCoreFunction getSDCore = (getSDCoreFunction) codeLibrary->resolve("getSDCore");
        if(getSDCore){            
            sdcore = reinterpret_cast<SimDSPCore*>(getSDCore());
            ui->runLayout->addWidget(sdcore);
            sdcore->show();
            sdcore->setTextOutput(ui->appOutput);
        }else{
            ui->appOutput->append(codeLibrary->errorString());
            if( codeLibrary->isLoaded() ) codeLibrary->unload();
            return;
        }

        dsp_loop = (loopFunction) codeLibrary->resolve("dsp_loop");
        if(dsp_loop){
            SimDSPTimer->start();
            sdcore->start();
        }else{
            ui->appOutput->append(codeLibrary->errorString());
            if( codeLibrary->isLoaded() ) codeLibrary->unload();
            return;
        }

        ui->statusBar->showMessage(tr("SimDSP Running..."));

        ui->tabWidget->setTabEnabled(ui->tabWidget->indexOf(ui->runTab), true);
        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->runTab));

        ui->actionRun->setDisabled(true);
        ui->actionBuild->setDisabled(true);
        ui->actionStop->setEnabled(true);
        ui->actionLoad->setEnabled(true);
        ui->actionSave->setEnabled(true);
    }
}

void SimDSP::actionStop()
{
    ui->tabWidget->setTabEnabled(ui->tabWidget->indexOf(ui->runTab), false);
    ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->codeTab));

    SimDSPTimer->stop();
    sdcore->stop();
    if( codeLibrary->isLoaded() ) codeLibrary->unload();

    ui->statusBar->showMessage(tr("Ready"));

    ui->actionRun->setEnabled(true);
    ui->actionBuild->setEnabled(true);
    ui->actionStop->setDisabled(true);
    ui->actionLoad->setDisabled(true);
    ui->actionSave->setDisabled(true);
}

void SimDSP::actionHelpContents()
{
    /*SDHelp* help = new SDHelp(this);

    help->show();*/
}

void SimDSP::loop()
{
    dsp_loop();
}


void SimDSP::actionIncreaseFontSize()
{
    ui->widgetEditor->increaseFontSize();
}

void SimDSP::actionDecreaseFontSize()
{
    ui->widgetEditor->decreaseFontSize();
}

void SimDSP::actionResetFontSize()
{
    ui->widgetEditor->resetFontSize();
}

void SimDSP::actionFont()
{
    ui->widgetEditor->selectFont();
}
