/*
 * SimDSP project explorer.
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

#ifndef SDPROJECTEXPLORER_H
#define SDPROJECTEXPLORER_H

#include <QWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QDesktopServices>
#include <QMenu>
#include <QMessageBox>
#include <QDir>
#include <QDebug>
#include <QUrl>

namespace Ui {
class SDProjectexplorer;
}

class SDProjectexplorer : public QWidget
{
    Q_OBJECT

public:
    explicit SDProjectexplorer(QWidget *parent = 0);
    ~SDProjectexplorer();
    void addProject(QString path, QString name);
    void addExample(QString path, QString name);
    void addFile(QString name);
    void setExpanded(bool expanded);
    void setProjectName(QString name);
    void sortProject();
    int  removeProject();
    void removeAllProjects();
    bool isExample();

signals:
    void buildProject();
    void cleanProject();
    void runProject();
    void closeProject();
    void changePath();

    void newFile();
    void deleteFile(QString fileName);
    void itemActivated(QString fileName, bool isExample);

public slots:
    void itemActivatedSD(QTreeWidgetItem* item, int column);
    void showContextMenu(const QPoint &pos);
    void activateProject();
    void showContainingFolder();
    void openFile();
    void removeFile();

private:

    Ui::SDProjectexplorer *ui;
    QTreeWidgetItem* currentProject;
    QTreeWidgetItem* selectItem;
};
#endif // SDPROJECTEXPLORER_H
