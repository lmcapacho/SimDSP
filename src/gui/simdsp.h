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

#ifndef SIMDSP_H
#define SIMDSP_H

#include <QMainWindow>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QProcess>
#include <QFontDatabase>
#include <QCloseEvent>

#include "view/sdview.h"
#include "editor/sdeditortab.h"
#include "project/sdproject.h"
#include "project/sdnewproject.h"
#include "actions/sdbuilder.h"
#include "actions/sdnewfile.h"

#define SDAPP_NOTFOUND  127
#ifdef Q_OS_LINUX
    #define SDAPP_KILL      9
#elif defined(Q_OS_WIN32)
    #define SDAPP_KILL      62097
#endif

namespace Ui {
class SimDSP;
}

class SimDSP : public QMainWindow
{
    Q_OBJECT

public:
    explicit SimDSP(QWidget *parent = nullptr);
    ~SimDSP();

public slots:
    // Files slots
    void actionNewFile();    
    void actionSaveFile();
    void actionCloseTab();
    void actionCloseAll();

    // SDCore slots
    void actionRun();
    void actionStop();
    void actionLoad();
    void actionSave();
    void actionAutoScale();
    void actionResetZoom();

    // Project slots
    void actionNewProject();
    void actionOpenProject();
    void actionBuildProject();
    void actionCleanProject();
    void actionCloseProject();
    void actionCloseActiveProject();
    void actionCloseAllProjectsEditors();
    void actionOpenRecentProject();
    void actionClearRecentProjects();

    // Edit slots
    void actionIncreaseFontSize();
    void actionDecreaseFontSize();
    void actionResetFontSize();
    void actionResetFont();
    void actionFont();

    // Help slots
    void actionAbout();
    void actionAboutQt();
    void actionHelpContents();

    // Example slots
    void actionOpenExample();

    // SDApp slots
    void sdappReadyRead();

    // View slots
    void changeView(QByteArray data);

    void tabOpen();
    void issues(int total);

    void sdappFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    Ui::SimDSP *ui;

    void initActionsConnections();
    void loadExamples();
    void loadRecentProjects();

    void menuCloseAllProjects();
    void menuOpenProject();

    void closeEvent(QCloseEvent *event);

    QSettings settings;

    QList<QAction*> examplesActionList;

    SDProject *sdproject;
    SDBuilder *sdbuilder;

    QProcess *sdapp;

    bool isRun;
};

#endif // SIMDSP_H
