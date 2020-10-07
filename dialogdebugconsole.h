#ifndef DIALOGDEBUGCONSOLE_H
#define DIALOGDEBUGCONSOLE_H

#include "6502.h"
#include <QDialog>
#include <QPlainTextEdit>
#include <QTimer>

extern QPlainTextEdit * dcPTEMap;
extern QTimer * dcLoopTimer;
//extern char textBuffer[(33*24)+1];
//extern int textPtr;

namespace Ui {
class DialogDebugConsole;
}

class DialogDebugConsole : public QDialog
{
    Q_OBJECT
public:
    explicit DialogDebugConsole(QWidget *parent = nullptr);
    ~DialogDebugConsole();
private slots:
    void dcDrawLoop();
private:
    Ui::DialogDebugConsole *ui;
};

#endif // DIALOGDEBUGCONSOLE_H
