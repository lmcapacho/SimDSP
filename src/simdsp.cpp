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

    int widthProject = static_cast<int>(width()*0.2);
    int widthCode = static_cast<int>(width()*0.8);
    ui->codeSplitter->setSizes(QList<int>({widthProject, widthCode}));

    int heightTabs = static_cast<int>(height()*0.7);
    int heightOutput = static_cast<int>(height()*0.3);
    ui->mainSplitter->setSizes(QList<int>({heightTabs, heightOutput}));

    ui->widgetEditor->addAction(ui->actionIncreaseFontSize);
    ui->widgetEditor->addAction(ui->actionDecreaseFontSize);
    ui->widgetEditor->addAction(ui->actionResetFontSize);

    sdproject = new SDProject(ui->widgetProject, ui->widgetEditor,
                              ui->compileOutput, ui->issuesOutput);

    ui->tabWidget->setCurrentIndex(0);
    ui->tabWidget->setTabEnabled(ui->tabWidget->indexOf(ui->runTab), false);

    ui->outputTab->setCurrentIndex(0);
    ui->compileOutput->setFont(QFont("SansSerif", 10));
    ui->issuesOutput->setFont(QFont("SansSerif", 10));

    sdapp = new QProcess(this);

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString libraryPath = QCoreApplication::applicationDirPath();
#ifdef Q_OS_LINUX
    env.insert("LD_LIBRARY_PATH", libraryPath);
#elif defined(Q_OS_WIN32)
    env.insert("PATH", libraryPath);
#endif
    sdapp->setProcessEnvironment(env);

    initActionsConnections();
    loadRecentProjects();
    loadExamples();

    isRun = false;
    ui->statusBar->showMessage(tr("Ready"));
}

SimDSP::~SimDSP()
{
    if(isRun) actionStop();
    delete ui;
}


void SimDSP::initActionsConnections()
{
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
    connect(ui->actionAutoscale, &QAction::triggered, this, &SimDSP::actionAutoScale);
    connect(ui->actionResetZoom, &QAction::triggered, this, &SimDSP::actionResetZoom);

    connect(ui->actionNewProject, &QAction::triggered, this, &SimDSP::actionNewProject);
    connect(ui->actionOpenProject, &QAction::triggered, this, &SimDSP::actionOpenProject);
    connect(ui->actionBuildProject, &QAction::triggered, this, &SimDSP::actionBuildProject);
    connect(ui->widgetProject, &SDProjectexplorer::buildProject, this, &SimDSP::actionBuildProject);
    connect(ui->actionCleanProject, &QAction::triggered, this, &SimDSP::actionCleanProject);
    connect(ui->widgetProject, &SDProjectexplorer::cleanProject, this, &SimDSP::actionCleanProject);
    connect(ui->actionCloseProject, &QAction::triggered, this, &SimDSP::actionCloseActiveProject);
    connect(ui->widgetProject, &SDProjectexplorer::closeActiveProject, this, &SimDSP::actionCloseProject);
    connect(ui->actionCloseAllProjectsEditors, &QAction::triggered, this, &SimDSP::actionCloseAllProjectsEditors);
    connect(ui->actionClearRecentProjects, &QAction::triggered, this, &SimDSP::actionClearRecentProjects);

    connect(ui->actionSelectAll, &QAction::triggered, ui->widgetEditor, &SDEditortab::selectAll );
    connect(ui->actionUndo, &QAction::triggered, ui->widgetEditor, &SDEditortab::undo );
    connect(ui->actionRedo, &QAction::triggered, ui->widgetEditor, &SDEditortab::redo );
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
    connect(ui->sdview, QOverload<QByteArray>::of(&SDView::changeView), this, &SimDSP::changeView);
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

    QString examplesFolder = QCoreApplication::applicationDirPath() + QDir::separator() + ".." + QDir::separator() + "examples";

    QFileInfoList folderList = QDir(examplesFolder).entryInfoList(QDir::NoDotAndDotDot|QDir::AllDirs);

    if(folderList.isEmpty()){
        examplesMenu->setDisabled(true);
    }else{
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
}

void SimDSP::loadRecentProjects()
{
    QAction* recentProjectsAction;

    foreach (QAction *action, ui->menuRecentProjects->actions()) {
        ui->menuRecentProjects->removeAction(action);
        if(action->isSeparator()){
            break;
        }
    }

    QStringList recentProjects = settings.value("Projects/recent").toStringList();
    QFile path;

    if(recentProjects.isEmpty()){
        ui->menuRecentProjects->setDisabled(true);
    }else{
        ui->menuRecentProjects->setEnabled(true);
        std::reverse(recentProjects.begin(), recentProjects.end());

        foreach (QString recentProject, recentProjects) {
            if(path.exists(recentProject)) {
                recentProjectsAction = new QAction(this);
                recentProjectsAction->setData(recentProject);
                recentProjectsAction->setText(recentProject);
                connect(recentProjectsAction, &QAction::triggered, this, &SimDSP::actionOpenRecentProject);
                ui->menuRecentProjects->insertAction(ui->actionClearRecentProjects, recentProjectsAction);
            } else {
                recentProjects.removeAt(recentProjects.indexOf(recentProject));
            }
        }

        settings.setValue("Projects/recent", recentProjects);
    }

    ui->menuRecentProjects->insertSeparator(ui->actionClearRecentProjects);
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
    ui->sdview->loadMatFile();
}

void SimDSP::actionSave()
{
    ui->sdview->saveMatFile();
}

void SimDSP::actionAutoScale()
{
    ui->sdview->autoScale();
}

void SimDSP::actionResetZoom()
{
    ui->sdview->resetZoom();
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
        QMessageBox::information(nullptr, "SimDSP", tr("This is an example. Examples are read-only\n"));
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
        QMessageBox::information(nullptr, "SimDSP", tr("This is an example. Examples are read-only\n"));
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
        menuOpenProject();
        loadRecentProjects();
    }
}

void SimDSP::actionOpenProject()
{
    QString lastOpen = settings.value("Projects/lastopen", QDir::homePath()).toString();
    QString path = QFileDialog::getExistingDirectory(this,
            tr("Open SimDSP Project"), lastOpen,
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(!path.isEmpty()){
        if( sdproject->openProject(path) ){
            menuOpenProject();
            loadRecentProjects();
            settings.setValue("Projects/lastopen", path);
        }
    }
}

void SimDSP::actionBuildProject()
{
    ui->outputTab->setTabText(1, "Issues");
    if(sdproject->buildProject()){
        ui->outputTab->setCurrentIndex(0);
    }else{
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
    ui->compileOutput->clear();
    if(isRun) actionStop();

    if( sdproject->closeProject() < 0 ){
        menuCloseAllProjects();
        return;
    }
}

void SimDSP::actionCloseActiveProject()
{
    ui->compileOutput->clear();
    if(isRun) actionStop();

    if( sdproject->closeActiveProject() < 0 ){
        menuCloseAllProjects();
        return;
    }
}

void SimDSP::actionCloseAllProjectsEditors()
{
    if(isRun) actionStop();
    sdproject->closeAll();
    ui->compileOutput->clear();
    menuCloseAllProjects();
    ui->actionCloseTab->setDisabled(true);
    ui->actionCloseAll->setDisabled(true);
}

void SimDSP::actionOpenRecentProject()
{
    QAction *recentProjectAction = static_cast<QAction*>(sender());
    QString path = recentProjectAction->data().toString();

    if(!path.isEmpty()){
        if( sdproject->openProject(path) ){
            menuOpenProject();
            loadRecentProjects();
            settings.setValue("Projects/lastopen", path);
        }
    }
}

void SimDSP::actionClearRecentProjects()
{
    settings.remove("Projects/recent");
    loadRecentProjects();
}

void SimDSP::actionRun()
{
    if(sdproject->buildProject()){

        ui->compileOutput->clear();
        ui->issuesOutput->clear();
        ui->outputTab->setTabText(1, "Issues");

        ui->compileOutput->append(tr("<b>Running SimDSP Project...</b><br>"));

        connect(sdapp, &QProcess::readyRead, this, &SimDSP::sdappReadyRead);
        connect(sdapp, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &SimDSP::sdappFinished);

    #ifdef Q_OS_LINUX
        sdapp->setProgram(QDir::currentPath()+"/build/sdapp");
        sdapp->start();
    #elif defined(Q_OS_WIN32)
        sdapp->setProgram(QDir::currentPath()+"/build/sdapp.exe");
        sdapp->start();
    #endif
        if(!sdapp->waitForStarted(5000)){
          ui->compileOutput->append(tr("<b>SDapp failed to start</b><br>"));
          ui->compileOutput->append(tr("<span style='color:#AE0000;'><br><b>Failed running SimDSP</b></span>"));
          ui->sdview->stop();
          return;
        }

        ui->sdview->start();

        ui->compileOutput->append(tr("<b>SDapp started</b><br>"));

        ui->outputTab->setCurrentIndex(0);
        ui->statusBar->showMessage(tr("SimDSP Simulation Running..."));

        ui->tabWidget->setTabEnabled(ui->tabWidget->indexOf(ui->runTab), true);
        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->runTab));

        ui->actionRun->setDisabled(true);
        ui->actionBuildProject->setDisabled(true);
        ui->actionCleanProject->setDisabled(true);
        ui->actionStop->setEnabled(true);
        ui->actionLoad->setEnabled(true);
        ui->actionSave->setEnabled(true);
        ui->actionAutoscale->setEnabled(true);
        ui->actionResetZoom->setEnabled(true);

        isRun = true;
    }else{
        ui->outputTab->setCurrentIndex(1);
    }
}

void SimDSP::actionStop()
{
    sdapp->close();
    disconnect(sdapp, nullptr, nullptr ,nullptr);

    ui->outputTab->setCurrentIndex(0);
    ui->compileOutput->clear();

    ui->tabWidget->setTabEnabled(ui->tabWidget->indexOf(ui->runTab), false);
    ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->codeTab));

    ui->sdview->clearOutput();
    ui->sdview->stop();

    ui->statusBar->showMessage(tr("Ready"));

    ui->actionRun->setEnabled(true);
    ui->actionBuildProject->setEnabled(true);
    ui->actionCleanProject->setEnabled(true);
    ui->actionStop->setDisabled(true);
    ui->actionLoad->setDisabled(true);
    ui->actionSave->setDisabled(true);
    ui->actionAutoscale->setDisabled(true);
    ui->actionResetZoom->setDisabled(true);
    isRun = false;
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
                    .arg(QStringLiteral("2021"),
                         QStringLiteral("www.gnu.org/licenses"));

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
        menuOpenProject();
    }
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

void SimDSP::sdappFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus);

    switch (exitCode) {
        case SDAPP_KILL:
            break;
        case SDAPP_NOTFOUND:
            if(isRun) actionStop();
            QMessageBox::critical(this, "SimDSP", tr("SDapp failed to start"));
            break;
        default:
            if(isRun) actionStop();
            QMessageBox::critical(this, "SimDSP", tr("SDapp has closed unexpectedly"));
            break;
    }
}

void SimDSP::sdappReadyRead()
{
    while(sdapp->canReadLine()){
        QByteArray data = sdapp->readLine().trimmed();
        QList<QByteArray> request = data.split(',');

        if(request.size()>0){
            bool ok;
            int cmd = request.at(0).toInt(&ok);
            if(ok){
                switch (cmd) {
                    case 0:
                        ui->sdview->setfs(static_cast<double>(request.at(1).toFloat()));
                        break;
                    case 1:
                        ui->sdview->enableMic();
                        break;
                    case 2:
                        ui->sdview->println(request.at(1));
                        break;
                    case 3:
                        ui->sdview->setSizeWindow(request.at(1).toInt());
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

void SimDSP::changeView(QByteArray data)
{
    if(sdapp->isOpen())
        sdapp->write(data);
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
    ui->actionUndo->setEnabled(true);
    ui->actionRedo->setEnabled(true);
    ui->actionIncreaseFontSize->setEnabled(true);
    ui->actionDecreaseFontSize->setEnabled(true);
    ui->actionResetFontSize->setEnabled(true);
    ui->actionResetFont->setEnabled(true);
    ui->actionFont->setEnabled(true);
}
