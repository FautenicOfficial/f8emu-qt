#ifndef DIALOGDEBUGGER_H
#define DIALOGDEBUGGER_H

#include "6502.h"
#include <QDialog>
#include <QPlainTextEdit>
#include <QTimer>

extern int dStartAddress;
extern QPlainTextEdit * dPTEMap;
extern QTimer * dLoopTimer;

namespace Ui {
class DialogDebugger;
}

class DialogDebugger : public QDialog
{
    Q_OBJECT
public:
    explicit DialogDebugger(QWidget *parent = nullptr);
    ~DialogDebugger();
private slots:
    void dDrawInfo();
    void dClearInfo();
    void dUpdateBreakpoints();
    void dAddBreakpoint(int addr);
    void dDeleteBreakpoint(int idx);
    void dDrawLoop();
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_5_clicked();
private:
    Ui::DialogDebugger *ui;
};

#endif // DIALOGDEBUGGER_H
