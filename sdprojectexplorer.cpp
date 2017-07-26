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
