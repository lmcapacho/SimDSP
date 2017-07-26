#ifndef SDEditortab_H
#define SDEditortab_H

#include <QObject>
#include <QMainWindow>
#include <QTextEdit>
#include <QDebug>
#include <QFontDialog>

#include <sdeditor.h>
#include <highlighter.h>

namespace Ui {
class SDEditortab;
}

class SDEditor;
class Highlighter;

class SDEditortab : public QWidget
{
    Q_OBJECT

public:
    explicit SDEditortab(QWidget *parent = 0);
    ~SDEditortab();
    void newFile();
    void newFile(QString fileName);
    bool openFile(const QString &fileName);
    SDEditor *createEditor();
    SDEditor *activeEditor();
    void asterisk();
    void tabClose();
    void saveFile();
    void saveAll();
    void closeAll();
    void setClosable(bool closable);
    void increaseFontSize();
    void decreaseFontSize();
    void resetFontSize();
    void selectFont();

private slots:
    void tabCloseRequested();

private:
    bool findFile(const QString &fileName);
    bool loadFile(const QString &fileName);
    void setFont(SDEditor *editor);
    void setFontAll();

    QFont font;

    Ui::SDEditortab *ui;

    Highlighter *highlighter;
    QTextEdit *textEdit;
    int findTab;    
};

#endif // SDEditortab_H
