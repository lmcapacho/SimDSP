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


SimDSPCore::SimDSPCore()
{
    sdKeyboardISRFnc = nullptr;

    init();
}

SimDSPCore::~SimDSPCore()
{

}

void SimDSPCore::attachInterrupt(void (*callback)())
{
    sdKeyboardISRFnc = callback;
}

void SimDSPCore::println(string text)
{
    cout<<"2,"<<QString::fromStdString(text).toStdString()<<endl;

}

void SimDSPCore::println(short number)
{
    cout<<"2,"<<QString::number(number).toStdString()<<endl;
}

void SimDSPCore::println(double number)
{
    cout<<"2,"<<QString::number(number).toStdString()<<endl;
}

int SimDSPCore::readKeyboard()
{
    return key;
}

void SimDSPCore::setfs(double fs)
{
    sd->setfs(fs);
    cout<<"0,"<<QString::number(fs).toStdString()<<endl;

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
    cout<<"1"<<endl;

}

void SimDSPCore::init()
{
    //Crea los objetos asociados a los buffer compartidos
    //creados por simDSP
    plotBufferIn= new QSharedMemory("SDPlotBufferIn");
    plotBufferOut=new QSharedMemory("SDPlotBufferOut");
    fileBufferIn=new QSharedMemory("SDFileBufferIn");
    fileBufferOut=new QSharedMemory("SDFileBufferOut");

    //Se conectan los objetos a la memoria compartida
    plotBufferIn->attach();
    plotBufferOut->attach();
    fileBufferIn->attach();
    fileBufferOut->attach();

    //Crea un vector con cada uno de los punteros
    sharedMemoryArray=new QVector<QSharedMemory*>();
    //Añade cada puntero al vector
    sharedMemoryArray->append(plotBufferIn);
    sharedMemoryArray->append(plotBufferOut);
    sharedMemoryArray->append(fileBufferIn);
    sharedMemoryArray->append(fileBufferOut);

    sd = new SDSignal(sharedMemoryArray);
}


void SimDSPCore::changeFileSize(int length)
{
    sd->changeFileSize(length);
}

void SimDSPCore::keyboardClicked(int iKey)
{
    key=iKey;
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

    sd->setSignalAmplitude(amp/10.0);
}

void SimDSPCore::changeSNR(int snr){
    sd->changeSNR(snr);
}

void SimDSPCore::changeAWGN(bool checked)
{
    sd->changeAWGN(checked);

}

void SimDSPCore::stop()
{
    sd->stop();

}

void SimDSPCore::start()
{
    sd->start();
    standardInputThread= new std::thread(&SimDSPCore::standardInputTask,this);
}


//Tarea que espera los comandos de SimDSP desde entrada estándar
void SimDSPCore::standardInputTask()
{
    string line;

    int cmd,value;
    while(true){
        //Espera por línea de comando
        getline(cin, line);
        //Identifica comando y valor
        sscanf(line.data(),"%d,%d",&cmd,&value);

        //Procesa el comando recibido
        switch(cmd){
            case 0:
            changeInput(value);
            break;
            case 1:
            changeFrequency(value);
            break;
            case 2:
            changeAmplitude(value);
            break;
            case 3:
            changeAWGN(value);
            break;
            case 4:
            changeSNR(value);
            break;
            case 5:
            keyboardClicked(value);
            break;
            case 6:
            changeFileSize(value);
            break;

            default:

            break;
        }
    }

}
