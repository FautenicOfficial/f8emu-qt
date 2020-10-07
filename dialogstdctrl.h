#ifndef DIALOGSTDCTRL_H
#define DIALOGSTDCTRL_H

#include <QDialog>
#include "dialogpresskey.h"
#include "ui_dialogpresskey.h"

namespace Ui {
class DialogStdCtrl;
}

class DialogStdCtrl : public QDialog
{
    Q_OBJECT
public:
    DialogPressKey * dpk;
    explicit DialogStdCtrl(QWidget *parent = nullptr);
    ~DialogStdCtrl();
    void openKeyDialog(QString dst,QPushButton * target);
private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_7_clicked();
    void on_pushButton_8_clicked();
private:
    Ui::DialogStdCtrl *ui;
};

#endif // DIALOGSTDCTRL_H
