#include "sdnewfile.h"
#include "ui_sdnewfile.h"

SDNewFile::SDNewFile(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SDNewFile)
{
    ui->setupUi(this);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    loadTemplate(SOURCE);
    connect(ui->templateSelect, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SDNewFile::changeTemplateSelect);
    connect(ui->fileName, QOverload<const QString&>::of(&QLineEdit::textChanged), this, &SDNewFile::changeFileName);
    connect(ui->buttonBox,&QDialogButtonBox::accepted, this, &SDNewFile::accepted);
}

SDNewFile::~SDNewFile()
{
    delete ui;
}

void SDNewFile::loadTemplate(int index)
{
    QFile file;

    if( index == SOURCE )
        file.setFileName(":/files/sdSourceTPL.txt");
    else
        file.setFileName(":/files/sdHeaderTPL.txt");

    file.open(QFile::ReadOnly | QFile::Text);

    QTextStream in(&file);
    ui->preview->setPlainText(in.readAll());
}

void SDNewFile::changeTemplateSelect(int index)
{
    loadTemplate(index);
    changeFileName(ui->fileName->text());
}

void SDNewFile::changeFileName(const QString &fileName)
{
    QRegExp rx;

    if( ui->templateSelect->currentIndex() == SOURCE )
        rx.setPattern("\\w+\\.cpp");
    else
        rx.setPattern("\\w+\\.h");


    QValidator *validator = new QRegExpValidator(rx, this);

    int pos = 0;

    if( validator->validate((QString&)fileName,pos) == QValidator::Acceptable )
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    else
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    ui->path->setText(QDir::currentPath()+'/'+fileName);
}

void SDNewFile::accepted()
{
    QString fileName = ui->fileName->text();

    if( ui->templateSelect->currentIndex() == SOURCE )
        QFile::copy(":/files/sdSourceTPL.txt", fileName);
    else
        QFile::copy(":/files/sdHeaderTPL.txt", fileName);

    QFile code(fileName);
    code.setPermissions(QFile::WriteUser | QFile::ReadUser);
}

QString SDNewFile::getFileName()
{
   return ui->fileName->text();
}
