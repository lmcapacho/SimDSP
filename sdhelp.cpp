#include "sdhelp.h"
#include "ui_sdhelp.h"

SDHelp::SDHelp(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SDHelp)
{
    ui->setupUi(this);

    helpEngine = new QHelpEngine(":/help/simdsphelp.qhc");
    helpEngine->setupData();

    ui->tabWidget->addTab(reinterpret_cast<QTabWidget*>(helpEngine->contentWidget()), "Contents");
    ui->tabWidget->addTab(reinterpret_cast<QTabWidget*>(helpEngine->indexWidget()), "Index");
}

SDHelp::~SDHelp()
{
    delete ui;
}
