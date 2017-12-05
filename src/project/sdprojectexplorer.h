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
#include <QDebug>

namespace Ui {
class SDProjectexplorer;
}

class SDProjectexplorer : public QWidget
{
    Q_OBJECT

public:
    explicit SDProjectexplorer(QWidget *parent = 0);
    ~SDProjectexplorer();
    int addTreeRoot(QString name);
    void addTreeChild(QString name);
    void setExpanded(bool expanded);
    void setTreeRootName(QString name);
    void removeTreeRoot();

signals:
    void DoubleClickedFile(QString fileName, int column);

public slots:
    void doubleClickedFile(QTreeWidgetItem* item, int column);

private:
    Ui::SDProjectexplorer *ui;

    QTreeWidgetItem *parent;
};
#endif // SDPROJECTEXPLORER_H
