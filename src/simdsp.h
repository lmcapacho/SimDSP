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

#include "sdcore.h"
#include "sdsignal.h"
#include "qcustomplot.h"
#include "editor/sdeditortab.h"
#include "project/sdproject.h"
#include "project/sdnewproject.h"
#include "actions/sdbuilder.h"
#include "actions/sdnewfile.h"

typedef void (*initFunction)(void);
typedef void (*loopFunction)(void);
typedef void (*setupFunction)(void);
typedef void *(*getSDCoreFunction)(void);
typedef void *(*closeSDCoreFunction)(void);

namespace Ui {
class SimDSP;
}

class SimDSP : public QMainWindow
{
    Q_OBJECT

public:
    explicit SimDSP(QWidget *parent = 0);
    ~SimDSP();

public slots:
    // Files slots
    void actionNewFile();    
    void actionSaveFile();
    void actionCloseTab();
    void actionCloseAll();

    // Build slots
    void actionRun();
    void actionStop();
    void actionLoad();
    void actionSave();

    // Project slots
    void actionNewProject();
    void actionOpenProject();
    void actionBuildProject();
    void actionCleanProject();
    void actionCloseProject();
    void actionCloseAllProjectsEditors();
    void actionChangePath();

    // Edit slots
    void actionIncreaseFontSize();
    void actionDecreaseFontSize();
    void actionResetFontSize();
    void actionResetFont();
    void actionFont();

    // Help slots
    void actionHelpContents();

    void loop();

    void tabOpen();
    void issues(int total);

private:
    Ui::SimDSP *ui;

    void initActionsConnections();

    void menuCloseAllProjects();
    void menuOpenProject();

    void closeEvent(QCloseEvent *event);

    QSettings settings;

    SDProject *sdproject;
    SDBuilder *sdbuilder;

    QLibrary *codeLibrary;

    QTimer *SimDSPTimer;
    loopFunction dsp_loop;

    SimDSPCore* sdcore;
};

#endif // SIMDSP_H
