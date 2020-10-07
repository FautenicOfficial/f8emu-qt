#ifndef DIALOGEXTCTRL_H
#define DIALOGEXTCTRL_H

#include <QDialog>
#include "dialogpresskey.h"
#include "ui_dialogpresskey.h"

namespace Ui {
class DialogExtCtrl;
}

class DialogExtCtrl : public QDialog
{
    Q_OBJECT
public:
    DialogPressKey * dpk;
    explicit DialogExtCtrl(QWidget *parent = nullptr);
    ~DialogExtCtrl();
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
    void on_pushButton_9_clicked();
    void on_pushButton_10_clicked();
    void on_pushButton_11_clicked();
    void on_pushButton_12_clicked();
    void on_pushButton_13_clicked();
private:
    Ui::DialogExtCtrl *ui;
};

#endif // DIALOGEXTCTRL_H
