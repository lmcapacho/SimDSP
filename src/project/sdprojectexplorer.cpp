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

    ui->projectTreeWidget->hideColumn(1);
    ui->projectTreeWidget->hideColumn(2);
    ui->projectTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

#if QT_VERSION >= 0x050700
    connect(ui->projectTreeWidget, QOverload<QTreeWidgetItem*, int>::of(&QTreeWidget::itemActivated),
            this, &SDProjectexplorer::itemActivatedSD);
    connect(ui->projectTreeWidget, QOverload<const QPoint &>::of(&QTreeWidget::customContextMenuRequested),
            this, &SDProjectexplorer::showContextMenu);
#else
    connect(ui->projectTreeWidget, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this, SLOT(itemActivatedSD(QTreeWidgetItem*,int)));
    connect(ui->projectTreeWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));
#endif
}

SDProjectexplorer::~SDProjectexplorer()
{
    delete ui;
}


void SDProjectexplorer::addProject(QString path, QString name)
{    
    QFont font;
    currentProject = new QTreeWidgetItem();
    ui->projectTreeWidget->addTopLevelItem(currentProject);

    font.setBold(false);
    QTreeWidgetItemIterator it(ui->projectTreeWidget);
    while (*it) {
        (*it)->setFont(0, font);
        (*it)->setTextColor(0, QColor("#888888"));
        ++it;
    }

    font.setBold(true);
    currentProject->setText(0, name);
    currentProject->setFont(0, font);
    currentProject->setTextColor(0, QColor("#FFFFFF"));
    currentProject->setToolTip(0, path);

    currentProject->setText(1, path);
}

void SDProjectexplorer::addExample(QString path, QString name)
{
    addProject(path, name);
    currentProject->setText(2, "example");
}

void SDProjectexplorer::addFile(QString name)
{
    QTreeWidgetItem *treeItem = new QTreeWidgetItem();

    treeItem->setText(0, name);

    currentProject->addChild(treeItem);
    sortProject();
}

void SDProjectexplorer::setProjectName(QString name)
{
    currentProject->setText(0, name);
}

void SDProjectexplorer::setExpanded(bool expanded)
{
    currentProject->setExpanded(expanded);
}

void SDProjectexplorer::sortProject()
{
    currentProject->sortChildren(0, Qt::AscendingOrder);
    QString mainFile = currentProject->text(0);

    QTreeWidgetItemIterator it(currentProject);
    while (*it) {
        if ((*it)->text(0) == mainFile && (*it)->parent()){
            break;
        }
        ++it;
    }

    int index = currentProject->indexOfChild(*it);
    QTreeWidgetItem* file = currentProject->takeChild(index);
    currentProject->insertChild(0, file);
}

void SDProjectexplorer::activateProject()
{
    currentProject->setTextColor(0, QColor("#888888"));
    for( int i = 0; i < currentProject->childCount(); ++i )
        currentProject->child(i)->setTextColor(0, QColor("#888888"));

    currentProject = selectItem;
    currentProject->setTextColor(0, QColor("#FFFFFF"));
    for( int i = 0; i < currentProject->childCount(); ++i )
        currentProject->child(i)->setTextColor(0, QColor("#FFFFFF"));
    QDir::setCurrent(currentProject->text(1));

    emit changePath();
}

int SDProjectexplorer::removeProject()
{
    int index = -1;

    if(selectItem){
        delete selectItem;
        index = ui->projectTreeWidget->topLevelItemCount()-1;
        if( index >= 0 ){
            currentProject = ui->projectTreeWidget->topLevelItem( index );
            currentProject->setTextColor(0, QColor("#FFFFFF"));
            for( int i = 0; i < currentProject->childCount(); ++i )
                currentProject->child(i)->setTextColor(0, QColor("#FFFFFF"));
            QDir::setCurrent(currentProject->text(1));
        }
    }

    return index;
}

int SDProjectexplorer::removeActiveProject()
{
    int index = -1;

    if(currentProject){
        delete currentProject;
        index = ui->projectTreeWidget->topLevelItemCount()-1;
        if( index >= 0 ){
            currentProject = ui->projectTreeWidget->topLevelItem( index );
            currentProject->setTextColor(0, QColor("#FFFFFF"));
            for( int i = 0; i < currentProject->childCount(); ++i )
                currentProject->child(i)->setTextColor(0, QColor("#FFFFFF"));
            QDir::setCurrent(currentProject->text(1));
        }
    }

    return index;
}

void SDProjectexplorer::removeAllProjects()
{
    ui->projectTreeWidget->clear();
    currentProject = NULL;
}

void SDProjectexplorer::showContainingFolder()
{
    QDesktopServices::openUrl( QUrl::fromLocalFile( selectItem->parent()->text(1) ) );
}

void SDProjectexplorer::openFile()
{
    itemActivatedSD(selectItem, 0);
}

void SDProjectexplorer::removeFile()
{
    QMessageBox::StandardButton removeBtn = QMessageBox::question( this, "SimDSP",
                                                                 tr("Are you sure that you want to permanently delete this file?\n"),
                                                                 QMessageBox::No | QMessageBox::Yes);
    if (removeBtn == QMessageBox::Yes) {
        QString fileName = selectItem->parent()->text(1) + QDir::separator() + selectItem->text(0);
        emit deleteFile(fileName);
        QFile file(fileName);
        file.remove();
        selectItem->parent()->removeChild(selectItem);
    }
}

bool SDProjectexplorer::isExample()
{
    return !currentProject->text(2).isEmpty();
}

void SDProjectexplorer::itemActivatedSD(QTreeWidgetItem *item, int column)
{
    if(item->parent()){
        QString fileName = item->parent()->text(1) + QDir::separator() + item->text(column);
        emit itemActivated(fileName, !item->parent()->text(2).isEmpty());
    }
}

void SDProjectexplorer::showContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = ui->projectTreeWidget->itemAt(pos);

    if( !item )
        return;

    selectItem = item;

    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    if( !item->parent() ){

        if( ui->projectTreeWidget->indexOfTopLevelItem(currentProject) !=
                ui->projectTreeWidget->indexOfTopLevelItem(item) ) {

            QAction* m_active = new QAction(tr("Set as Active Project"), this);
            connect(m_active, &QAction::triggered, this, &SDProjectexplorer::activateProject);
            menu->addAction(m_active);
        }else{

            QAction* m_build = new QAction(QIcon(":/resources/images/icons/builder.png"), tr("Build"), this);
            connect(m_build, &QAction::triggered, this, &SDProjectexplorer::buildProject);
            menu->addAction(m_build);

            QAction* m_clean = new QAction(tr("Clean"), this);
            connect(m_clean, &QAction::triggered, this, &SDProjectexplorer::cleanProject);
            menu->addAction(m_clean);

            QAction* m_run = new QAction(QIcon(":/resources/images/icons/play.png"), tr("Run"), this);
            connect(m_run, &QAction::triggered, this, &SDProjectexplorer::runProject);
            menu->addAction(m_run);

            menu->addSeparator();

            if( item->text(2).isEmpty() ){
                QAction* m_new_file = new QAction(QIcon(":/resources/images/icons/new_file.png"), tr("New File"), this);
                connect(m_new_file, &QAction::triggered, this, &SDProjectexplorer::newFile);
                menu->addAction(m_new_file);
            }
        }

        QString m_closeName = tr("Close Project") + " '" + item->text(0) + "'";
        QAction* m_close = new QAction(m_closeName, this);
        connect(m_close, &QAction::triggered, this, &SDProjectexplorer::closeActiveProject);
        menu->addAction(m_close);

        menu->exec( ui->projectTreeWidget->mapToGlobal(pos) );
    }else{

        QAction* m_open = new QAction(tr("Open File"), this);
        connect(m_open, &QAction::triggered, this, &SDProjectexplorer::openFile);
        menu->addAction(m_open);

        QAction* m_show_folder = new QAction(tr("Show Containing Folder"), this);
        connect(m_show_folder, &QAction::triggered, this, &SDProjectexplorer::showContainingFolder);
        menu->addAction(m_show_folder);

        if( item->parent()->text(2).isNull() ){
            if( item->parent()->text(0) != item->text(0) ){
                QAction* m_remove = new QAction(tr("Remove File"), this);
                connect(m_remove, &QAction::triggered, this, &SDProjectexplorer::removeFile);
                menu->addAction(m_remove);
            }
        }

        menu->exec( ui->projectTreeWidget->mapToGlobal(pos) );
    }
}
