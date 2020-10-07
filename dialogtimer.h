#ifndef DIALOGTIMER_H
#define DIALOGTIMER_H

#include "6502.h"
#include <QDialog>
#include <QTimer>

extern QTimer * tLoopTimer;

namespace Ui {
class DialogTimer;
}

class DialogTimer : public QDialog
{
    Q_OBJECT
public:
    explicit DialogTimer(QWidget *parent = nullptr);
    ~DialogTimer();
private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void tDrawLoop();
private:
    Ui::DialogTimer *ui;
};

#endif // DIALOGTIMER_H
