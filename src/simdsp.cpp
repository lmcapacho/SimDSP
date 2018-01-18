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

    SimDSPTimer = new QTimer();

    int widthProject = (int)(width()*0.2);
    int widthCode = (int)(width()*0.8);
    ui->codeSplitter->setSizes(QList<int>({widthProject, widthCode}));

    int heightTabs = (int)(height()*0.7);
    int heightOutput = (int)(height()*0.3);
    ui->mainSplitter->setSizes(QList<int>({heightTabs, heightOutput}));

    ui->widgetEditor->addAction(ui->actionIncreaseFontSize);
    ui->widgetEditor->addAction(ui->actionDecreaseFontSize);
    ui->widgetEditor->addAction(ui->actionResetFontSize);

    sdproject = new SDProject(ui->widgetProject, ui->widgetEditor,
                              ui->appOutput, ui->issuesOutput);

    ui->tabWidget->setCurrentIndex(0);
    ui->tabWidget->setTabEnabled(ui->tabWidget->indexOf(ui->runTab), false);

    ui->outputTab->setCurrentIndex(0);
    ui->appOutput->setFont(QFont("SansSerif"));
    ui->issuesOutput->setFont(QFont("SansSerif"));

    codeLibrary =  new QLibrary();

    initActionsConnections();
    loadExamples();

    ui->statusBar->showMessage(tr("Ready"));
}

SimDSP::~SimDSP()
{
    delete ui;
}


void SimDSP::initActionsConnections()
{
    connect(SimDSPTimer, &QTimer::timeout, this, &SimDSP::loop);

    connect(ui->actionNewFile, &QAction::triggered, this, &SimDSP::actionNewFile);
    connect(ui->widgetProject, &SDProjectexplorer::newFile, this, &SimDSP::actionNewFile);
    connect(ui->actionSaveFile, &QAction::triggered, this, &SimDSP::actionSaveFile);
    connect(ui->actionCloseTab, &QAction::triggered, this, &SimDSP::actionCloseTab);
    connect(ui->widgetEditor, &SDEditortab::tabCloseRequested, this, &SimDSP::actionCloseTab);
    connect(ui->actionCloseAll, &QAction::triggered, this, &SimDSP::actionCloseAll);

    connect(ui->actionRun, &QAction::triggered, this, &SimDSP::actionRun);
    connect(ui->widgetProject, &SDProjectexplorer::runProject, this, &SimDSP::actionRun);
    connect(ui->actionStop, &QAction::triggered, this, &SimDSP::actionStop);
    connect(ui->actionLoad, &QAction::triggered, this, &SimDSP::actionLoad);
    connect(ui->actionSave, &QAction::triggered, this, &SimDSP::actionSave);

    connect(ui->actionNewProject, &QAction::triggered, this, &SimDSP::actionNewProject);
    connect(ui->actionOpenProject, &QAction::triggered, this, &SimDSP::actionOpenProject);
    connect(ui->actionBuildProject, &QAction::triggered, this, &SimDSP::actionBuildProject);
    connect(ui->widgetProject, &SDProjectexplorer::buildProject, this, &SimDSP::actionBuildProject);
    connect(ui->actionCleanProject, &QAction::triggered, this, &SimDSP::actionCleanProject);
    connect(ui->widgetProject, &SDProjectexplorer::cleanProject, this, &SimDSP::actionCleanProject);
    connect(ui->actionCloseProject, &QAction::triggered, this, &SimDSP::actionCloseProject);
    connect(ui->widgetProject, &SDProjectexplorer::closeProject, this, &SimDSP::actionCloseProject);
    connect(ui->actionCloseAllProjectsEditors, &QAction::triggered, this, &SimDSP::actionCloseAllProjectsEditors);

    connect(ui->actionSelectAll, &QAction::triggered, ui->widgetEditor, &SDEditortab::selectAll );
    connect(ui->actionIncreaseFontSize, &QAction::triggered, this, &SimDSP::actionIncreaseFontSize);
    connect(ui->actionDecreaseFontSize, &QAction::triggered, this, &SimDSP::actionDecreaseFontSize);
    connect(ui->actionResetFontSize, &QAction::triggered, this, &SimDSP::actionResetFontSize);
    connect(ui->actionResetFont, &QAction::triggered, this, &SimDSP::actionResetFont);
    connect(ui->actionFont, &QAction::triggered, this, &SimDSP::actionFont);

    //connect(ui->actionHelpContents, &QAction::triggered, this, &SimDSP::actionHelpContents);
    connect(ui->actionAbout, &QAction::triggered, this, &SimDSP::actionAbout);
    connect(ui->actionAboutQt, &QAction::triggered, this, &SimDSP::actionAboutQt);

    connect(ui->actionQuit, &QAction::triggered, this, &SimDSP::close);

    connect(sdproject, &SDProject::tabOpen, this, &SimDSP::tabOpen);
    connect(sdproject, QOverload<int>::of(&SDProject::buildIssues), this, &SimDSP::issues);

    connect(ui->widgetProject, &SDProjectexplorer::changePath, this, &SimDSP::actionChangePath);
}

void SimDSP::loadExamples()
{
    QAction* exampleAction;
    QMenu* examplesMenu = ui->menuFile->addMenu(tr("Examples"));

    foreach (QAction *action, ui->menuFile->actions()) {
        if(action->isSeparator()){
            ui->menuFile->insertMenu(action, examplesMenu);
            break;
        }
    }

    QString examplesFolder = QCoreApplication::applicationDirPath() + QDir::separator() + "examples";

    QFileInfoList folderList = QDir(examplesFolder).entryInfoList(QDir::NoDotAndDotDot|QDir::AllDirs);

    foreach (QFileInfo folderInfo, folderList) {

        QDirIterator it(folderInfo.absoluteFilePath(), QDir::NoDotAndDotDot|QDir::AllDirs);
        QMenu* examplesFolder = examplesMenu->addMenu(folderInfo.fileName());

        while (it.hasNext()) {
            QString folder = it.next();
            exampleAction = new QAction(this);
            exampleAction->setData(folder);
            exampleAction->setText(QFileInfo(folder).fileName());
            connect(exampleAction, &QAction::triggered, this, &SimDSP::actionOpenExample);
            examplesFolder->addAction(exampleAction);
        }
    }
}

void SimDSP::closeEvent(QCloseEvent *event)
{
    sdproject->closeAllTabs();
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

void SimDSP::actionCloseTab()
{
    if( sdproject->closeTab() == 0 ){
        ui->actionSaveFile->setDisabled(true);
        ui->actionCloseTab->setDisabled(true);
        ui->actionCloseAll->setDisabled(true);
    }
}

void SimDSP::actionCloseAll()
{
    sdproject->closeAllTabs();
    ui->actionSaveFile->setDisabled(true);
    ui->actionCloseTab->setDisabled(true);
    ui->actionCloseAll->setDisabled(true);
}

void SimDSP::actionNewFile()
{
    if(ui->widgetProject->isExample()){
        QMessageBox::information(0, "SimDSP", tr("This is an example. Examples are read-only\n"));
    }else{
        SDNewFile* newFile =  new SDNewFile(this);

        if(newFile->exec()){
            sdproject->newFile(newFile->getFileName());
            ui->actionSaveFile->setEnabled(true);
            ui->actionCloseTab->setEnabled(true);
            ui->actionCloseAll->setEnabled(true);
        }
    }
}

void SimDSP::actionSaveFile()
{
    if(ui->widgetProject->isExample()){
        QMessageBox::information(0, "SimDSP", tr("This is an example. Examples are read-only\n"));
    }else{
        sdproject->saveFile();
    }
}

void SimDSP::actionNewProject()
{
    SDNewProject* newProject = new SDNewProject(this);

    if(newProject->exec()){
        QString projectPath, projectName;
        newProject->getProjectInfo(projectName, projectPath);
        sdproject->newProject(projectName, projectPath);
    #ifdef Q_OS_LINUX
        codeLibrary->setFileName(QDir::currentPath()+"/build/libsdcode.so");
    #elif defined(Q_OS_WIN32)
        codeLibrary->setFileName(QDir::currentPath()+"/build/libsdcode.dll");
    #endif
        menuOpenProject();
    }
}

void SimDSP::actionOpenProject()
{
    QString lastOpen = settings.value("simdsp/lastopen", QDir::homePath()).toString();
    QString path = QFileDialog::getExistingDirectory(this,
            tr("Open SimDSP Project"), lastOpen,
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(!path.isEmpty()){
        if( sdproject->openProject(path) ){
        #ifdef Q_OS_LINUX
            codeLibrary->setFileName(QDir::currentPath()+"/build/libsdcode.so");
        #elif defined(Q_OS_WIN32)
            codeLibrary->setFileName(QDir::currentPath()+"/build/libsdcode.dll");
        #endif
            menuOpenProject();
            settings.setValue("simdsp/lastopen", path);
        }
    }
}

void SimDSP::actionBuildProject()
{
    ui->outputTab->setTabText(1, "Issues");
    if(sdproject->buildProject()){
        ui->outputTab->setCurrentIndex(0);
    }
    else{
        ui->outputTab->setCurrentIndex(1);
    }
}

void SimDSP::actionCleanProject()
{
    sdproject->cleanProject();
    ui->outputTab->setCurrentIndex(0);
    ui->outputTab->setTabText(1, "Issues");
}

void SimDSP::actionCloseProject()
{
    ui->appOutput->clear();

    if( sdproject->closeProject() < 0 ){
        menuCloseAllProjects();
        return;
    }

#ifdef Q_OS_LINUX
    codeLibrary->setFileName(QDir::currentPath()+"/build/libsdcode.so");
#elif defined(Q_OS_WIN32)
    codeLibrary->setFileName(QDir::currentPath()+"/build/libsdcode.dll");
#endif
}

void SimDSP::actionCloseAllProjectsEditors()
{
    sdproject->closeAll();
    ui->appOutput->clear();
    menuCloseAllProjects();
    ui->actionCloseTab->setDisabled(true);
    ui->actionCloseAll->setDisabled(true);
}

void SimDSP::actionChangePath()
{
#ifdef Q_OS_LINUX
    codeLibrary->setFileName(QDir::currentPath()+"/build/libsdcode.so");
#elif defined(Q_OS_WIN32)
    codeLibrary->setFileName(QDir::currentPath()+"/build/libsdcode.dll");
#endif
}

void SimDSP::actionRun()
{
    if(sdproject->buildProject()){

        ui->appOutput->clear();
        ui->issuesOutput->clear();
        ui->outputTab->setTabText(1, "Issues");

        setupFunction dsp_setup = (setupFunction) codeLibrary->resolve("dsp_setup");
        if(!dsp_setup){
            ui->appOutput->append(tr("<b>Running SimDSP Project...</b><br>"));
            ui->appOutput->append(codeLibrary->errorString());
            if( codeLibrary->isLoaded() ) codeLibrary->unload();
            ui->appOutput->append(tr("<span style='color:#AE0000;'><br><b>Failed running SimDSP</b></span>"));
            return;
        }

        initFunction dsp_init = (initFunction) codeLibrary->resolve("dsp_init");
        if(dsp_init){
            dsp_init();
        }else{
            ui->appOutput->append(tr("<b>Running SimDSP Project...</b><br>"));
            ui->appOutput->append(codeLibrary->errorString());
            if( codeLibrary->isLoaded() ) codeLibrary->unload();
            ui->appOutput->append(tr("<span style='color:#AE0000;'><b><br>Failed running SimDSP</b></span>"));
            return;
        }

        getSDCoreFunction getSDCore = (getSDCoreFunction) codeLibrary->resolve("getSDCore");
        if(getSDCore){            
            sdcore = reinterpret_cast<SimDSPCore*>(getSDCore());
            ui->runLayout->addWidget(sdcore);
            sdcore->show();
            sdcore->setTextOutput(ui->appOutput);
        }else{
            ui->appOutput->append(tr("<b>Running SimDSP Project...</b><br>"));
            ui->appOutput->append(codeLibrary->errorString());
            if( codeLibrary->isLoaded() ) codeLibrary->unload();
            ui->appOutput->append(tr("<span style='color:#AE0000;'><b><br>Failed running SimDSP</b></span>"));
            return;
        }

        dsp_loop = (loopFunction) codeLibrary->resolve("dsp_loop");
        if(dsp_loop){
            SimDSPTimer->start();
            sdcore->start();
        }else{
            ui->appOutput->append(tr("<b>Running SimDSP Project...</b><br>"));
            ui->appOutput->append(codeLibrary->errorString());
            if( codeLibrary->isLoaded() ) codeLibrary->unload();
            ui->appOutput->append(tr("<span style='color:#AE0000;'><b><br>Failed running SimDSP</b></span>"));
            return;
        }

        ui->outputTab->setTabText(0, "Application Output");
        ui->outputTab->setCurrentIndex(0);
        ui->statusBar->showMessage(tr("SimDSP Running..."));

        ui->tabWidget->setTabEnabled(ui->tabWidget->indexOf(ui->runTab), true);
        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->runTab));

        ui->actionRun->setDisabled(true);
        ui->actionBuildProject->setDisabled(true);
        ui->actionCleanProject->setDisabled(true);
        ui->actionStop->setEnabled(true);
        ui->actionLoad->setEnabled(true);
        ui->actionSave->setEnabled(true);
    }else{
        ui->outputTab->setCurrentIndex(1);
    }
}

void SimDSP::actionStop()
{
    ui->outputTab->setTabText(0, "Compile Output");
    ui->outputTab->setCurrentIndex(0);
    ui->appOutput->clear();

    ui->tabWidget->setTabEnabled(ui->tabWidget->indexOf(ui->runTab), false);
    ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->codeTab));

    SimDSPTimer->stop();
    sdcore->stop();
    if( codeLibrary->isLoaded() ) codeLibrary->unload();

    ui->statusBar->showMessage(tr("Ready"));

    ui->actionRun->setEnabled(true);
    ui->actionBuildProject->setEnabled(true);
    ui->actionCleanProject->setEnabled(true);
    ui->actionStop->setDisabled(true);
    ui->actionLoad->setDisabled(true);
    ui->actionSave->setDisabled(true);
}

void SimDSP::actionHelpContents()
{
    /*SDHelp* help = new SDHelp(this);

    help->show();*/
}

void SimDSP::actionAbout()
{
    QString text = QString("<h3>SimDSP - DSP Simulator</h3>"

                   "<p>Copyright (C) 2017-%1 Luis Miguel Capacho V., Alexander López Parrado "
                   "and Jorge Iván Marín H.</p>"

                   "<p>SimDSP based on Qt created and designed by Luis Miguel Capacho V.</p>"

                   "<p>Based on an original idea by Jorge Iván Marín H. (SimDSP based on C++ Builder)</p>"

                   "<p>The base code is a derivative work of the code from the GNUSimDSP "
                   "project based on WxWidgets by Alexander López Parrado.</p>"

                   "<p>SimDSP is free software: you can redistribute it and/or modify it under the terms "
                   "of the GNU General Public License as published by the Free Software Foundation, either"
                   "version 3 of the License, or (at your option) any later version.</p>"

                   "<p>SimDSP is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; "
                   "without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. "
                   "See the GNU General Public License for more details.</p>"

                   "<p>You should have received a copy of the GNU General Public License along with SimDSP.  "
                   "If not, see <a href=\"http://%2/\">%2</a>.</p>")
                    .arg(QStringLiteral("2018"))
                    .arg(QStringLiteral("www.gnu.org/licenses"));

    QMessageBox::about(this, "About SimDSP", text);
}

void SimDSP::actionAboutQt()
{
    QMessageBox::aboutQt(this, "About Qt used on SimDSP");
}

void SimDSP::actionOpenExample()
{
    QAction* example = qobject_cast<QAction*>(sender());
    if( sdproject->openExample(example->data().toString()) ){
    #ifdef Q_OS_LINUX
        codeLibrary->setFileName(QDir::currentPath()+"/build/libsdcode.so");
    #elif defined(Q_OS_WIN32)
        codeLibrary->setFileName(QDir::currentPath()+"/build/libsdcode.dll");
    #endif
        menuOpenProject();
    }
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

void SimDSP::actionResetFont()
{
    ui->widgetEditor->resetFont();
}

void SimDSP::actionFont()
{
    ui->widgetEditor->selectFont();
}

void SimDSP::tabOpen()
{
    ui->actionSaveFile->setEnabled(true);
    ui->actionCloseTab->setEnabled(true);
    ui->actionCloseAll->setEnabled(true);
}

void SimDSP::issues(int total)
{
    ui->outputTab->setTabText(1, "Issues ("+QString::number(total)+")");
}

/******************************************
 * Enabled and disabled menu options
 ******************************************/

void SimDSP::menuCloseAllProjects()
{
    ui->actionCloseProject->setDisabled(true);
    ui->actionCloseAllProjectsEditors->setDisabled(true);
    ui->actionBuildProject->setDisabled(true);
    ui->actionCleanProject->setDisabled(true);
    ui->actionRun->setDisabled(true);
    ui->actionNewFile->setDisabled(true);
    ui->actionSaveFile->setDisabled(true);
    ui->actionSelectAll->setDisabled(true);
    ui->actionIncreaseFontSize->setDisabled(true);
    ui->actionDecreaseFontSize->setDisabled(true);
    ui->actionResetFontSize->setDisabled(true);
    ui->actionResetFont->setDisabled(true);
    ui->actionFont->setDisabled(true);

    if( ui->widgetEditor->getOpenedTabs() == 0 ){
        ui->actionCloseTab->setDisabled(true);
        ui->actionCloseAll->setDisabled(true);
    }
}

void SimDSP::menuOpenProject()
{
    ui->actionCloseProject->setEnabled(true);
    ui->actionCloseAllProjectsEditors->setEnabled(true);
    ui->actionBuildProject->setEnabled(true);
    ui->actionCleanProject->setEnabled(true);
    ui->actionRun->setEnabled(true);
    ui->actionNewFile->setEnabled(true);
    ui->actionSaveFile->setEnabled(true);
    ui->actionCloseTab->setEnabled(true);
    ui->actionCloseAll->setEnabled(true);
    ui->actionSelectAll->setEnabled(true);
    ui->actionIncreaseFontSize->setEnabled(true);
    ui->actionDecreaseFontSize->setEnabled(true);
    ui->actionResetFontSize->setEnabled(true);
    ui->actionResetFont->setEnabled(true);
    ui->actionFont->setEnabled(true);
}
