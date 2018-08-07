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
    QDir currentPath(QDir::currentPath());
    QString mkName = currentPath.dirName()+".mk";

    if( !build.exists() )
        create();
    else{
        QFile makefile(mkName);
        if( !makefile.exists() )
            createMakefile();
    }

    QFile::copy(":/resources/templates/simdsp.cpp", QDir::currentPath()+"/simdsp.cpp");
    QFile::setPermissions(QDir::currentPath()+"/simdsp.cpp", QFile::WriteUser | QFile::ReadUser);

#ifdef Q_OS_LINUX
    QFile::remove(QDir::currentPath()+"/build/libsdcode.so");
    process->start("make", QStringList() << "-f" << mkName);
#elif defined(Q_OS_WIN32)
    QFile::remove(QDir::currentPath()+"/build/libsdcode.dll");
    process->start("mingw32-make", QStringList() << "-f" << mkName);
#endif

    process->waitForStarted();
    if(!process->waitForFinished())
        return false;

    QFile::remove(QDir::currentPath()+"/simdsp.cpp");

#ifdef Q_OS_LINUX
    QFileInfo output(QDir::currentPath()+"/build/libsdcode.so");
#elif defined(Q_OS_WIN32)
    QFileInfo output(QDir::currentPath()+"/build/libsdcode.dll");
#endif

    return output.exists();
}

void SDBuilder::clean()
{
    QDir build(QDir::currentPath()+"/build");
    QDir currentPath(QDir::currentPath());
    QString mkName = currentPath.dirName()+".mk";

    if( ! build.exists() )
        return;
    else{
        QFile makefile(mkName);

        if( !makefile.exists() )
            createMakefile();
    }

#ifdef Q_OS_LINUX
    process->start("make", QStringList() << "-f" << mkName << "clean");
#elif defined(Q_OS_WIN32)
    process->start("mingw32-make", QStringList() << "-f" << mkName << "clean");
#endif
    process->waitForStarted();
    process->waitForFinished();
}

void SDBuilder::create()
{
    QDir build(QDir::currentPath());
    build.mkpath("build");

    createMakefile();
}

void SDBuilder::createMakefile()
{
    QDir currentPath(QDir::currentPath());
    QString mkName = currentPath.dirName()+".mk";

    QFile::copy(":/resources/templates/Makefile_simdsp", mkName);
    QFile makefile(mkName);
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
