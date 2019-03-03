#ifndef SDMAT_H
#define SDMAT_H

#include <QDebug>
#include <QDialog>
#include <QFileDialog>
#include <QStringList>

#include <matio.h>

namespace Ui {
class SDMat;
}

class SDMat : public QDialog
{
    Q_OBJECT

public:
    explicit SDMat(QWidget *parent = nullptr);
    ~SDMat();

signals:
    void loadVariable(QString path, QString varName);

public slots:
    void accepted();
    void browseFile();
    void changeVariable(int index);

private:
    Ui::SDMat *ui;

    QStringList mat_types;
    QStringList types;
};

#endif // SDMAT_H
