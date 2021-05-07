#include "sdmat.h"
#include "ui_sdmat.h"

SDMat::SDMat(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SDMat)
{
    ui->setupUi(this);

    mat_types << "Empty" << "Cell" << "Struct" << "Object" << "Char" <<
                "Sparse" << "Double" << "Single" << "Int8" << "Uint8" <<
                "Int16" << "Uint16" << "Int32" << "Uint32" << "Int64" <<
                "Uint64" << "Function" << "Opaque";

    ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);

    connect(ui->browseButton, &QPushButton::pressed, this, &SDMat::browseFile);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SDMat::accepted);
    connect(ui->variables, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SDMat::changeVariable);
}

SDMat::~SDMat()
{
    delete ui;
}

void SDMat::browseFile()
{
    QSettings settings;
    QString lastopen = settings.value("MatFiles/lastopen").toString();

    QString filePath = QFileDialog::getOpenFileName(this, tr("Open File"), lastopen, tr("Matlab/Octave Files (*.mat)"));

    if( !filePath.isEmpty() ){
        mat_t *matfp;
        matvar_t *matvar;
        matfp = Mat_Open(filePath.toLatin1().data(), MAT_ACC_RDONLY);

        ui->filePath->setText(filePath);
        ui->error->setText("");
        ui->varType->setText("");
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        types.clear();
        ui->variables->clear();

        if ( nullptr == matfp ) {
            ui->error->setText("Error opening MAT file");
            ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
            return;
        }

        while ( (matvar = Mat_VarReadNextInfo(matfp)) != nullptr ) {
            types.append(mat_types[static_cast<int>(matvar->class_type)] + (matvar->isComplex ? " - Complex" : "" ) );
            ui->variables->addItem(matvar->name);
            Mat_VarFree(matvar);
            matvar = nullptr;
        }

        if( !types.isEmpty() ){
            ui->varType->setText(types[0]);
            ui->variables->setCurrentIndex(0);
        }

        Mat_Close(matfp);
    }
}

void SDMat::changeVariable(int index)
{
    if( !types.isEmpty() )
        ui->varType->setText(types[index]);
}

void SDMat::accepted()
{
    QSettings settings;
    settings.setValue("MatFiles/lastopen",QFileInfo(ui->filePath->text()).canonicalPath());

    emit loadVariable(ui->filePath->text(), ui->variables->currentText());
}
