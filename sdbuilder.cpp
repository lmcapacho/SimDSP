/*
 * SimDSP build support.
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

#include "sdbuilder.h"

SDBuilder::SDBuilder()
{
    process = new QProcess(this);

    connect(process, &QProcess::readyReadStandardError, this, &SDBuilder::readProcess);
    connect(process, &QProcess::readyReadStandardOutput, this, &SDBuilder::readProcess);
}

bool SDBuilder::build()
{
    QDir build(QDir::currentPath()+"/build");
    if( ! build.exists() )
        this->create();

    QFile::copy(":/base/simdsp.cpp", QDir::currentPath()+"/simdsp.cpp");
    QFile::setPermissions(QDir::currentPath()+"/build/simdsp.cpp", QFile::WriteUser | QFile::ReadUser);


    QFile::remove(QDir::currentPath()+"/build/libsdcode.so");

    process->start("make", QStringList() << "-C" << "build");
    process->waitForStarted();
    if(!process->waitForFinished())
        return false;

    //if(!QFileInfo::exists(QDir::currentPath()+"simdsp.dsp"))
    QFile::remove(QDir::currentPath()+"/simdsp.cpp");

    QFileInfo output(QDir::currentPath()+"/build/libsdcode.so");

    return output.exists();
}

void SDBuilder::create()
{
    QDir build(QDir::currentPath());
    build.mkpath("build");

    QFile::copy(":/files/Makefile", "build/Makefile");    
    QFile makefile("build/Makefile");
    makefile.setPermissions(QFile::WriteUser | QFile::ReadUser);
}

void SDBuilder::readProcess()
{
    QByteArray standardError = process->readAllStandardError();
    QByteArray standardOutput = process->readAllStandardOutput();

    if( !standardError.isEmpty() ){
        emit errorOutput(standardError);
    }

    if( !standardOutput.isEmpty() ){
        emit errorOutput(standardOutput);
    }
}
