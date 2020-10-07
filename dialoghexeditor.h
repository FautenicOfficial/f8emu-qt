#ifndef DIALOGHEXEDITOR_H
#define DIALOGHEXEDITOR_H

#include "6502.h"
#include <QDialog>
#include <QPlainTextEdit>
#include <QTimer>

extern int heStartAddress;
extern QPlainTextEdit * hePTEMap;
extern QTimer * heLoopTimer;

namespace Ui {
class DialogHexEditor;
}

class DialogHexEditor : public QDialog
{
    Q_OBJECT
public:
    explicit DialogHexEditor(QWidget *parent = nullptr);
    ~DialogHexEditor();
private slots:
    void heDrawLoop();
    void heSetNybble(WORD addr,char data,bool hi);
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
private:
    Ui::DialogHexEditor *ui;
};

#endif // DIALOGHEXEDITOR_H
