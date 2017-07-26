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
