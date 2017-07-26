#ifndef SDPROJECT_H
#define SDPROJECT_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QTextEdit>

#include "sdprojectexplorer.h"
#include "sdbuilder.h"
#include "sdeditortab.h"

class SDProject : public QObject
{
    Q_OBJECT
public:
    explicit SDProject(SDProjectexplorer *explorer,
                       SDEditortab *editorTab,
                       QTextEdit *appOutput);
    void newProject(QString projectName, QString projectPath);
    bool openProject(QString projectPath);
    void closeProject();
    bool saveProject(QString projectName, QString projectPath);
    bool buildProject();

    void newFile(QString fileName);
    void saveFile();

    SDEditortab *editor;

public slots:
    void builderOutput(QByteArray error);
    void doubleClickedFile(QString fileName);

private:
    SDProjectexplorer *projectExplorer;
    SDBuilder *builder;

    QTextEdit *output;
    QString path;
};

#endif // SDPROJECT_H
