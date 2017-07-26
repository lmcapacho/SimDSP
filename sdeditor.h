#ifndef SDEditor_H
#define SDEditor_H

#include <QTextEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QApplication>

class SDEditor : public QTextEdit
{
    Q_OBJECT

public:
    SDEditor();

    void newFile();
    void newFile(QString fileName);
    void loadTemplate(QString fileName);
    bool loadFile(const QString &fileName);
    bool save();
    bool saveAs();
    bool saveFile(const QString &fileName);
    QString userFriendlyCurrentFile();
    QString currentFile() { return curFile; }
    bool isFileLoad();
    void isFileLoad(bool load);
    bool closeEditor();

protected:

private slots:
    void documentWasModified();

private:
    bool maybeSave();
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);

    QString curFile;
    bool isUntitled;
    bool isLoad;
};
#endif // SDEditor_H
