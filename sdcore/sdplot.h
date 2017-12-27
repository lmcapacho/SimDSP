/*
 * SimDSP Plot.
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

#ifndef SDPLOT_H
#define SDPLOT_H

#include <QWidget>
#include <QVector>
#include "qcustomplot.h"

namespace Ui {
class SDPlot;
}

class SDPlot : public QWidget
{
    Q_OBJECT

public:
    explicit SDPlot(QWidget *parent = 0);
    ~SDPlot();
    void setSizeWindow(int size);
    void updatePlotTime(const QVector<double> *data);
    void updatePlotFreq(const QVector<double> *data);
    void clearPlot();
    void setfs(double fsValue);

public slots:
    void plotMouseRelease(QMouseEvent *event);
    void plotMouseMove(QMouseEvent *event);

private:
    Ui::SDPlot *ui;

    QCPItemLine *cursor;
    QCPItemText *cursorLabel;

    QVector<double> *keysTime;
    QVector<double> *keysFreq;

    double y;
    double x;
    double fs;
    bool cursorMove;

    int sizeWindow;
};

#endif // SDPLOT_H
