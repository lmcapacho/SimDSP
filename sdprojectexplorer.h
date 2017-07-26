#ifndef SDPROJECTEXPLORER_H
#define SDPROJECTEXPLORER_H

#include <QWidget>
#include <QTreeWidgetItem>
#include <QDebug>

namespace Ui {
class SDProjectexplorer;
}

class SDProjectexplorer : public QWidget
{
    Q_OBJECT

public:
    explicit SDProjectexplorer(QWidget *parent = 0);
    ~SDProjectexplorer();
    void addTreeRoot(QString name);
    void addTreeChild(QString name);
    void setExpanded(bool expanded);
    void setTreeRootName(QString name);
    void removeTreeRoot();

signals:
    void DoubleClickedFile(QString fileName);

public slots:
    void doubleClickedFile(QTreeWidgetItem* item, int column);

private:
    Ui::SDProjectexplorer *ui;

    QTreeWidgetItem *parent;
};
#endif // SDPROJECTEXPLORER_H
