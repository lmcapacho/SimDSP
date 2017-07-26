#include "sdplot.h"
#include "ui_sdplot.h"

SDPlot::SDPlot(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SDPlot)
{
    ui->setupUi(this);

    ui->customPlot->addGraph(); // blue line
    ui->customPlot->graph()->setPen(QPen(QColor(229, 217, 11)));
    ui->customPlot->yAxis->setRange(-1.2, 1.2);
    ui->customPlot->xAxis->setVisible(false);
    ui->customPlot->setBackground(QBrush(QColor(3, 3, 13)));
    ui->customPlot->xAxis->grid()->setPen(QPen(QColor(14, 85, 89), 1, Qt::DotLine));
    ui->customPlot->yAxis->grid()->setPen(QPen(QColor(14, 85, 89), 1, Qt::DotLine));
    ui->customPlot->yAxis->grid()->setZeroLinePen(QPen(QColor(Qt::gray), 1, Qt::DotLine));
    ui->customPlot->yAxis->setTickPen(QPen(Qt::white, 1));
    ui->customPlot->yAxis->setTickLabelColor(Qt::white);

    cursor = new QCPItemLine(ui->customPlot);
    cursor->setPen(QPen(QColor(Qt::gray), 2, Qt::DotLine));
    cursorLabel = new QCPItemText(ui->customPlot);
    cursorLabel->setClipToAxisRect(false);
    cursorLabel->setPositionAlignment(Qt::AlignTop | Qt::AlignLeft);
    cursorLabel->setText("Cursor: ");
    cursorLabel->setFont(QFont(font().family(), 9));
    cursorLabel->setColor(QColor(Qt::white));
    cursorLabel->position->setCoords(0, ui->customPlot->yAxis->range().lower);

    connect(ui->customPlot, QOverload<QMouseEvent*>::of(&QCustomPlot::mousePress), this, &SDPlot::plotMouseMove);
    connect(ui->customPlot, QOverload<QMouseEvent*>::of(&QCustomPlot::mouseMove), this, &SDPlot::plotMouseMove);
    connect(ui->customPlot, QOverload<QMouseEvent*>::of(&QCustomPlot::mouseRelease), this, &SDPlot::plotMouseRelease);

    sizeWindow = 850;

    keysTime = new QVector<double>(sizeWindow);

    for (int i = 0; i < keysTime->size(); i++) {
        keysTime->replace(i, i);
    }

    keysFreq = new QVector<double>(1024);

    for (int i = 0; i < keysFreq->size(); i++) {
        keysFreq->replace(i, i);
    }

    cursorMove = false;
    x = y = 0;
    fs = 8000.0;
}

SDPlot::~SDPlot()
{
    delete ui;
}


void SDPlot::setSizeWindow(int size)
{
    sizeWindow = (850/size)-1;
}

void SDPlot::setfs(double fsValue)
{
    fs = fsValue;
}

/************************************************************
 * Public slots
 * **********************************************************/

void SDPlot::plotMouseMove(QMouseEvent *event)
{
    if( event->button() == Qt::LeftButton )
        cursorMove = true;

    if( cursorMove ){
        y = ui->customPlot->yAxis->pixelToCoord(event->y());
        x = ui->customPlot->xAxis->pixelToCoord(event->x());
    }
}

void SDPlot::plotMouseRelease(QMouseEvent *event)
{
    if( event->button() == Qt::LeftButton )
        cursorMove = false;
}

void SDPlot::updatePlotTime(const QVector<double> *data)
{
    ui->customPlot->graph()->setPen(QPen(QColor(229, 217, 11)));

    ui->customPlot->graph()->setData(*keysTime, *data);
    ui->customPlot->xAxis->setRange(0,sizeWindow);
    ui->customPlot->yAxis->setRange(-1.2, 1.2);

    cursor->start->setCoords(QCPRange::minRange, y);
    cursor->end->setCoords(QCPRange::maxRange, y);

    QString value = "Cursor: " + QString::number(y, 'f', 2) + " V";
    cursorLabel->setText(value);
    cursorLabel->position->setCoords(0, ui->customPlot->yAxis->range().lower);

    ui->customPlot->replot();
}

void SDPlot::updatePlotFreq(const QVector<double> *data)
{
    ui->customPlot->graph()->setPen(QPen(QColor(64, 186, 225)));

    ui->customPlot->graph()->setData(*keysFreq, *data);
    ui->customPlot->xAxis->setRange(0,512);
    ui->customPlot->yAxis->setRange(0, 1.2);

    cursor->start->setCoords(x, QCPRange::minRange);
    cursor->end->setCoords(x, QCPRange::maxRange);

    double f = x*(fs/2) / 512.0;

    QString value = "Cursor: " + QString::number(f, 'f', 2) + " Hz";
    cursorLabel->setText(value);
    cursorLabel->position->setCoords(0, ui->customPlot->yAxis->range().lower);

    ui->customPlot->replot();
}
