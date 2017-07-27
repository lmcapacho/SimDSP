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

#include "sdprojectexplorer.h"
#include "ui_sdprojectexplorer.h"

SDProjectexplorer::SDProjectexplorer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SDProjectexplorer)
{
    ui->setupUi(this);

    connect(ui->projectTreeWidget, QOverload<QTreeWidgetItem*, int>::of(&QTreeWidget::itemDoubleClicked),
            this, &SDProjectexplorer::doubleClickedFile);
}

SDProjectexplorer::~SDProjectexplorer()
{
    delete ui;
}


void SDProjectexplorer::addTreeRoot(QString name)
{
    QFont font;
    parent = new QTreeWidgetItem(ui->projectTreeWidget);

    font.setBold(true);
    parent->setText(0, name);
    parent->setFont(0, font);
}

void SDProjectexplorer::addTreeChild(QString name)
{
    QTreeWidgetItem *treeItem = new QTreeWidgetItem();

    treeItem->setText(0, name);

    parent->addChild(treeItem);
}

void SDProjectexplorer::setTreeRootName(QString name)
{
    parent->setText(0, name);
}

void SDProjectexplorer::setExpanded(bool expanded)
{
    parent->setExpanded(expanded);
}

void SDProjectexplorer::removeTreeRoot()
{
    if(parent)
        delete parent;
}

void SDProjectexplorer::doubleClickedFile(QTreeWidgetItem *item, int column)
{
    emit DoubleClickedFile(item->text(column));
}
