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

#ifndef SDBUILDER_H
#define SDBUILDER_H

#include <QObject>
#include <QDebug>
#include <QProcess>
#include <QDirIterator>

class SDBuilder : public QObject
{
    Q_OBJECT
public:
    explicit SDBuilder();
    bool build();
    void clean();
    void create();

signals:
    void errorOutput(QByteArray error);
    void msgOutput(QByteArray msg);

public slots:
    void readProcess();

private:
    QProcess *process;
};

#endif // SDBUILDER_H
