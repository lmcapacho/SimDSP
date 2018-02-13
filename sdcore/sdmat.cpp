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
#if QT_VERSION >= 0x050700
    connect(ui->variables, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SDMat::changeVariable);
#else
    connect(ui->variables, SIGNAL(currentIndexChanged(int)), this, SLOT(changeVariable(int)));
#endif
}

SDMat::~SDMat()
{
    delete ui;
}

void SDMat::browseFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open File"), QString(), tr("Matlab/Octave Files (*.mat)"));

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

        if ( NULL == matfp ) {
            ui->error->setText("Error opening MAT file");
            ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
            return;
        }

        while ( (matvar = Mat_VarReadNextInfo(matfp)) != NULL ) {
            types.append(mat_types[(int)matvar->class_type] + (matvar->isComplex ? " - Complex" : "" ) );
            ui->variables->addItem(matvar->name);
            Mat_VarFree(matvar);
            matvar = NULL;
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
    emit loadVariable(ui->filePath->text(), ui->variables->currentText());
}
