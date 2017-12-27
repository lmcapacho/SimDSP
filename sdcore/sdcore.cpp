/*
 * SimDSP Core.
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

#include "sdcore.h"
#include "ui_sdcore.h"

SimDSPCore::SimDSPCore() : QWidget()
{
    ui = new Ui::SimDSPCore;
    ui->setupUi(this);

    output = NULL;
    sdKeyboardISRFnc = NULL;

    init();
}

SimDSPCore::~SimDSPCore()
{
    delete ui;
}

void SimDSPCore::attachInterrupt(void (*callback)())
{
    sdKeyboardISRFnc = callback;
}

void SimDSPCore::println(string text)
{
    if(output)
        output->append(QString::fromStdString(text));
}

void SimDSPCore::println(short number)
{
    if(output)
        output->append(QString::number(number));
}

void SimDSPCore::println(double number)
{
    if(output)
        output->append(QString::number(number));
}

int SimDSPCore::readKeyboard()
{
    return ui->widgetKeyboard->keyboardStatus();
}

void SimDSPCore::setfs(double fs)
{
    sd->setfs(fs);
    ui->frequencySpinBox->setSingleStep(fs/200.0);
    ui->frequencySpinBox->setMaximum(fs/2.0);
    ui->fsValue->setText(QString::number(fs, 'f', 1) + " Hz");
    ui->PlotA->setfs(fs);
    ui->PlotB->setfs(fs);
    ui->frequencySpinBox->setValue(sd->getSignalFrequency());
}

short SimDSPCore::readADC()
{
    return sd->readADC();
}

void SimDSPCore::writeDAC(short value)
{
    sd->writeDAC(value);
}

void SimDSPCore::captureBlock( short* pBuffer, int length, void (*callback)() )
{
    sd->captureBlock( pBuffer, length, callback );
}

void SimDSPCore::playBlock( short* pBuffer, int length, void (*callback)() )
{
    sd->playBlock( pBuffer, length, callback );
}

void SimDSPCore::enableMic(int length)
{
    sd->enableMic(length);
    ui->inputComboBox->setCurrentIndex(sd->SIGNAL_MIC);
}

void SimDSPCore::setTextOutput(QTextEdit *textOutput)
{
    output = textOutput;
}


void SimDSPCore::loadFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString(), tr("Octave/Matlab Files (*.mat)"));

    if (!fileName.isEmpty()){
        QFile file(fileName);
        if (!file.open(QFile::ReadOnly | QFile::Text)) {
            QMessageBox::warning(this, tr("SimDSP"),
                                tr("Cannot open file %1:\n%2.")
                                .arg(QDir::toNativeSeparators(fileName), file.errorString()));
           return;
        }

        sd->loadFile(&file);

        file.close();
    }
}

void SimDSPCore::saveFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QString(),tr("Octave/Matlab Files (*.mat)"));

    if(fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("SimDSP"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    sd->saveFile(&file);
}

void SimDSPCore::init()
{
    sd = new SDSignal;

    connect(ui->inputComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SimDSPCore::changeInput );
    connect(ui->frequencySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SimDSPCore::changeFrequency);
    connect(ui->amplitudeDial, QOverload<int>::of(&QDial::valueChanged), this, &SimDSPCore::changeAmplitude);
    connect(ui->timeBaseDial, QOverload<int>::of(&QDial::valueChanged), this, &SimDSPCore::changeBaseTime);
    connect(ui->timeFreqGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),this, &SimDSPCore::changeInOutSelect);

    connect(sd, &SDSignal::newData, this, &SimDSPCore::newData);

    connect(ui->widgetKeyboard, &SDKeyboard::keyboardClicked, this, &SimDSPCore::keyboardClicked);

    sd->setSignalFrequency(100);
    ui->frequencySpinBox->setSingleStep(50);
}


/******************************************
 * Public slots
 ******************************************/

void SimDSPCore::keyboardClicked()
{
    if( sdKeyboardISRFnc ){
        sdKeyboardISRFnc();
    }
}

void SimDSPCore::changeInput(int inputIndex)
{
    sd->setSignalType(inputIndex);
}

void SimDSPCore::changeFrequency(int freq)
{
    sd->setSignalFrequency(freq);
}

void SimDSPCore::changeAmplitude(int amp)
{
    QString value = "Amplitude: " + QString::number(amp/10.0) +"V";
    ui->amplitudeLabel->setText(value);

    sd->setSignalAmplitude(amp/10.0);
}

void SimDSPCore::changeBaseTime(int bt)
{
    QString value = "Time base (x" + QString::number(11-bt) +")";
    ui->timeBaseLabel->setText(value);

    ui->PlotA->setSizeWindow(ui->timeBaseDial->value());
    ui->PlotB->setSizeWindow(ui->timeBaseDial->value());

    sd->setBaseTime(bt);
}

void SimDSPCore::changeInOutSelect(QAbstractButton *button)
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
}

void SimDSPCore::newData(const QVector<double> *inTime, const QVector<double> *inFreq,
                         const QVector<double> *outTime, const QVector<double> *outFreq)
{
    if( ui->inOutTime->isChecked() ){
        ui->PlotA->updatePlotTime(inTime);
        ui->PlotB->updatePlotTime(outTime);
    }else if( ui->inOutFreq->isChecked() ){
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

void SimDSPCore::stop()
{
    sd->stop();
    ui->PlotA->clearPlot();
    ui->PlotB->clearPlot();
}

void SimDSPCore::start()
{
    sd->start();
}
