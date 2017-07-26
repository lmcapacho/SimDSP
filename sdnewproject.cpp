#include "sdnewproject.h"
#include "ui_sdnewproject.h"

SDNewProject::SDNewProject(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SDNewProject)
{
    ui->setupUi(this);

    ui->projectPath->setText(QDir::homePath());

    connect(ui->browseButton, &QPushButton::clicked, this, &SDNewProject::browsePath);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SDNewProject::accepted);
}

SDNewProject::~SDNewProject()
{
    delete ui;
}

void SDNewProject::getProjectInfo(QString &name, QString &path)
{
    path = ui->projectPath->text();
    name = ui->projectName->text();
}

void SDNewProject::browsePath()
{
    QString path = QFileDialog::getExistingDirectory(this,
            tr("Save SimDSP Project"),QDir::homePath(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    ui->projectPath->setText(path);
}


void SDNewProject::accepted()
{
    QDir dir(ui->projectPath->text());

    if(dir.exists()){
        dir.mkpath(ui->projectPath->text()+"/"+ui->projectName->text());
    }
}
