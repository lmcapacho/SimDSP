/*
 * SimDSP View.
 *
 * Copyright (c) 2019 lmcapacho
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

#include "sdview.h"
#include "ui_sdview.h"

SDView::SDView(QWidget *parent) :
    QWidget(parent)
{
    ui = new Ui::SDView;
    ui->setupUi(this);

    output = nullptr;

    init();
}

SDView::~SDView()
{
    delete ui;
}

void SDView::println(QString text)
{
    ui->appOutput->append(text);
}

int SDView::readKeyboard()
{
    return ui->widgetKeyboard->keyboardStatus();
}

void SDView::setfs(double fs)
{
    ui->frequencySpinBox->setSingleStep(static_cast<int>(fs/200.0));
    ui->frequencySpinBox->setMaximum(static_cast<int>(fs/2.0));
    ui->fsValue->setText(QString::number(fs, 'f', 1) + " Hz");
    ui->PlotA->setfs(fs);
    ui->PlotB->setfs(fs);
    ui->frequencySpinBox->setValue(static_cast<int>(fs/100.0));
}

void SDView::enableMic()
{
    ui->inputComboBox->setItemData(SIGNAL_MIC, QVariant(Qt::ItemIsSelectable|Qt::ItemIsEnabled), Qt::UserRole-1);
    ui->inputComboBox->setCurrentIndex(SIGNAL_MIC);
    ui->inputComboBox->setDisabled(true);
}

void SDView::clearOutput()
{
    ui->appOutput->clear();
}

void SDView::init()
{
    sdmat = new SDMat;
    refresh = new QTimer;

#if QT_VERSION >= 0x050700
    connect(ui->inputComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SDView::changeInput );
    connect(ui->frequencySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SDView::changeFrequency);
    connect(ui->amplitudeDial, QOverload<int>::of(&QDial::valueChanged), this, &SDView::changeAmplitude);
    connect(ui->timeBaseDial, QOverload<int>::of(&QDial::valueChanged), this, &SDView::changeBaseTime);
    connect(ui->timeFreqGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),this, &SDView::changeInOutSelect);

    connect(ui->awgnCheckBox, QOverload<bool>::of(&QCheckBox::toggled), this, &SDView::changeAWGN);
    connect(ui->snrSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SDView::changeSNR);

    connect(sdmat, QOverload<QString, QString>::of(&SDMat::loadVariable), this, &SDView::loadFile);
#else
    connect(ui->inputComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeInput(int)));
    connect(ui->frequencySpinBox, SIGNAL(valueChanged(int)), this, SLOT(changeFrequency(int)));
    connect(ui->amplitudeDial, SIGNAL(valueChanged(int)), this, SLOT(changeAmplitude(int)));
    connect(ui->timeBaseDial, SIGNAL(valueChanged(int)), this, SLOT(changeBaseTime(int)));
    connect(ui->timeFreqGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(changeInOutSelect(QAbstractButton*)));

    connect(ui->awgnCheckBox, SIGNAL(toggled(bool)), this, SLOT(changeAWGN(bool)));
    //connect(ui->snrSpinBox, SIGNAL(valueChanged(int)), sd, SLOT(changeSNR(int)));

    connect(sdmat, SIGNAL(loadVariable(QString,QString)), this, SLOT(loadFile(QString,QString)));
#endif

    connect(ui->widgetKeyboard, &SDKeyboard::keyboardClicked, this, &SDView::keyboardClicked);
    connect(refresh, &QTimer::timeout, this, &SDView::newData);

    changeFrequency(100);
    ui->frequencySpinBox->setSingleStep(50);

    ui->inputComboBox->setItemData(SIGNAL_MIC, 0, Qt::UserRole-1);
}


/******************************************
 * Public slots
 ******************************************/
void SDView::autoScale()
{
    ui->PlotA->setAutoScale();
    ui->PlotB->setAutoScale();
}

void SDView::resetZoom()
{
    ui->PlotA->resetZoom();
    ui->PlotB->resetZoom();
}

void SDView::loadMatFile()
{
    sdmat->show();
}

void SDView::loadFile(QString path, QString varName)
{
    mat_t *matfp;
    matvar_t *matvar;
    matfp = Mat_Open(path.toLatin1().data(), MAT_ACC_RDONLY);

    matvar = Mat_VarRead(matfp, varName.toLatin1().data());

    if( matvar->dims[0] > 1 )
        fileLength = static_cast<size_t>(matvar->dims[0]);
    else
        fileLength = static_cast<size_t>(matvar->dims[1]);

    if( fileLength > MAXFILESIZE ) fileLength = MAXFILESIZE;

    SDFileBufferIn.lock();
    memcpy(SDFileBufferIn.data(), reinterpret_cast<void*>(matvar->data), fileLength*sizeof(double));
    SDFileBufferIn.unlock();

    QByteArray data = "6," + QByteArray::number(static_cast<int>(fileLength)) + "\n";
    emit changeView(data);
}

void SDView::saveMatFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QString(),tr("Matlab/Octave Files (*.mat)"));

    if(fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("SimDSP"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }


    mat_t *matfp;
    matvar_t *matvar;

    size_t dims[2] = {1, fileLength};

    matfp = Mat_CreateVer(file.fileName().toLatin1().data(), nullptr, MAT_FT_DEFAULT);

    SDFileBufferOut.lock();
    matvar = Mat_VarCreate("out", MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dims, SDFileBufferOut.data(), 0);
    SDFileBufferOut.unlock();
    if ( nullptr != matvar ) {
        Mat_VarWrite(matfp, matvar, MAT_COMPRESSION_NONE);
        Mat_VarFree(matvar);
    }

    Mat_Close(matfp);

}

void SDView::keyboardClicked()
{
    QByteArray data = "5," + QByteArray::number(readKeyboard()) + "\n";
    emit changeView(data);
}

void SDView::changeInput(int inputIndex)
{
    SDView::SignalTypes signalType = static_cast<SDView::SignalTypes>(inputIndex);

    if( signalType >= SDView::SIGNAL_NOISE ){
        ui->snrSpinBox->setValue(0);
        ui->frequencySpinBox->setDisabled(true);
        ui->awgnCheckBox->setDisabled(true);
        ui->awgnCheckBox->setChecked(false);
    }
    else{
        ui->snrSpinBox->setValue(40);
        ui->awgnCheckBox->setEnabled(true);
        ui->frequencySpinBox->setEnabled(true);
    }

    QByteArray data = "0," +QByteArray::number(inputIndex)+ "\n";
    emit changeView(data);
}

void SDView::changeFrequency(int freq)
{
    QByteArray data = "1," + QByteArray::number(freq)+ "\n";
    emit changeView(data);
}

void SDView::changeAmplitude(int amp)
{
    QString value = "Amplitude: " + QString::number(amp/10.0) +"V";
    ui->amplitudeLabel->setText(value);

    QByteArray data = "2," + QByteArray::number(amp)+ "\n";
    emit changeView(data);
}

void SDView::changeBaseTime(int bt)
{
    QString value = "Time base (x" + QString::number(11-bt) +")";
    ui->timeBaseLabel->setText(value);

    ui->PlotA->setSizeWindow(ui->timeBaseDial->value());
    ui->PlotB->setSizeWindow(ui->timeBaseDial->value());
}

void SDView::changeInOutSelect(QAbstractButton *button)
{
    Q_UNUSED(button);

    if( ui->inOutTime->isChecked() ){
        ui->inputBox->setTitle("Input (ADC) - Time");
        ui->outputBox->setTitle("Output (DAC) - Time");
    }else if( ui->inOutFreq->isChecked() ){
        ui->inputBox->setTitle("Input (ADC) - Frequency");
        ui->outputBox->setTitle("Output (DAC) - Frequency");
    }else if( ui->inTimeFreq->isChecked() ){
        ui->inputBox->setTitle("Input (ADC) - Time");
        ui->outputBox->setTitle("Input (ADC) - Frequency");
    }else if( ui->outTimeFreq->isChecked() ){
        ui->inputBox->setTitle("Output (DAC) - Time");
        ui->outputBox->setTitle("Output (DAC) - Frequency");
    }

    ui->PlotA->resetZoom();
    ui->PlotB->resetZoom();
}

void SDView::changeAWGN(bool checked)
{
    QByteArray data = "3," + QByteArray::number(checked) + "\n";
    emit changeView(data);

    if(checked){
        ui->snrLabel->setEnabled(true);
        ui->snrSpinBox->setEnabled(true);
    }else{
        ui->snrLabel->setDisabled(true);
        ui->snrSpinBox->setDisabled(true);
    }
}

void SDView::changeSNR(int value)
{
    QByteArray data = "4," + QByteArray::number(value) + "\n";
    emit changeView(data);
}

void SDView::changeSizeWindow(int size)
{
    ui->PlotA->setMaxSizeWindow(size);
    ui->PlotB->setMaxSizeWindow(size);
    ui->PlotA->setSizeWindow(ui->timeBaseDial->value());
    ui->PlotB->setSizeWindow(ui->timeBaseDial->value());
}

void SDView::newData()
{
    SDPlotBufferIn.lock();
    SDPlotBufferOut.lock();
    memcpy(reinterpret_cast<char*>(inTime->data()), SDPlotBufferIn.constData(), 1024*sizeof(double));
    memcpy(reinterpret_cast<char*>(outTime->data()), SDPlotBufferOut.constData(), 1024*sizeof(double));
    SDPlotBufferIn.unlock();
    SDPlotBufferOut.unlock();

    if( ui->inOutTime->isChecked() ){
        ui->PlotA->updatePlotTime(inTime);
        ui->PlotB->updatePlotTime(outTime);
    }else{
        updateFFT();
        if( ui->inOutFreq->isChecked() ){
            ui->PlotA->updatePlotFreq(inFreq);
            ui->PlotB->updatePlotFreq(outFreq);
        }else if( ui->inTimeFreq->isChecked() ){
            ui->PlotA->updatePlotTime(inTime);
            ui->PlotB->updatePlotFreq(inFreq);
        }else if( ui->outTimeFreq->isChecked() ){
            ui->PlotA->updatePlotTime(outTime);
            ui->PlotB->updatePlotFreq(outFreq);
        }
    }
}

void SDView::updateFFT()
{
    double real, img;

    for( int i=0; i<fftWidth; i++){
        fftInADC[i]=inTime->value(i);
        fftInDAC[i]=outTime->value(i);
    }

    fftw_execute (planADC);
    fftw_execute (planDAC);

    for( int i=0; i<fftWidth/2; i++)
    {
        real = fftOutADC[i][0];
        img = fftOutADC[i][1];
        inFreq->replace(i, qSqrt(qPow(real,2.0) + qPow(img,2.0))/511.0);

        real = fftOutDAC[i][0];
        img = fftOutDAC[i][1];
        outFreq->replace(i, qSqrt(qPow(real,2.0) + qPow(img,2.0))/511.0);
    }

}

void SDView::stop()
{
    ui->PlotA->clearPlot();
    ui->PlotB->clearPlot();
    ui->inputComboBox->setItemData(SIGNAL_MIC, 0, Qt::UserRole-1);
    ui->inputComboBox->setCurrentIndex(SIGNAL_SIN);
    ui->inputComboBox->setEnabled(true);

    SDPlotBufferIn.detach();
    SDPlotBufferOut.detach();
    SDFileBufferIn.detach();
    SDFileBufferOut.detach();

    free(inTime);
    free(outTime);
    free(inFreq);
    free(outFreq);

    fftw_free(fftInADC);
    fftw_free(fftOutADC);
    fftw_free(fftInDAC);
    fftw_free(fftOutDAC);

    fftw_destroy_plan (planADC);
    fftw_destroy_plan (planDAC);

    refresh->stop();
}

void SDView::start()
{
    if (SDPlotBufferIn.isAttached()) SDPlotBufferIn.detach();
    if (SDPlotBufferOut.isAttached()) SDPlotBufferOut.detach();
    if (SDFileBufferIn.isAttached()) SDFileBufferIn.detach();
    if (SDFileBufferOut.isAttached()) SDFileBufferOut.detach();

    SDPlotBufferIn.setKey("SDPlotBufferIn");
    SDPlotBufferIn.create(1024*sizeof(double));
    SDPlotBufferOut.setKey("SDPlotBufferOut");
    SDPlotBufferOut.create(1024*sizeof(double));

    SDFileBufferIn.setKey("SDFileBufferIn");
    SDFileBufferIn.create(MAXFILESIZE*sizeof(double));
    SDFileBufferOut.setKey("SDFileBufferOut");
    SDFileBufferOut.create(MAXFILESIZE*sizeof(double));

    inTime = new QVector<double>(1024);
    outTime = new QVector<double>(1024);

    inFreq = new QVector<double>(1024);
    outFreq = new QVector<double>(1024);

    fftWidth =  1024;

    fftInADC = fftw_alloc_real(static_cast<size_t>(fftWidth));
    fftOutADC = fftw_alloc_complex(static_cast<size_t>((fftWidth/2)+1));
    fftInDAC = fftw_alloc_real(static_cast<size_t>(fftWidth));
    fftOutDAC = fftw_alloc_complex(static_cast<size_t>((fftWidth/2)+1));

    planADC = fftw_plan_dft_r2c_1d ( fftWidth, fftInADC, fftOutADC, FFTW_ESTIMATE );
    planDAC = fftw_plan_dft_r2c_1d ( fftWidth, fftInDAC, fftOutDAC, FFTW_ESTIMATE );

    refresh->start(30);
}
