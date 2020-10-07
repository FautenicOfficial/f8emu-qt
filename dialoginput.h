#ifndef DIALOGINPUT_H
#define DIALOGINPUT_H

#include <QDialog>
#include "dialogstdctrl.h"
#include "ui_dialogstdctrl.h"
#include "dialogextctrl.h"
#include "ui_dialogextctrl.h"

namespace Ui {
class DialogInput;
}

class DialogInput : public QDialog
{
    Q_OBJECT
public:
    DialogStdCtrl * dsc;
    DialogExtCtrl * dec;
    explicit DialogInput(QWidget *parent = nullptr);
    ~DialogInput();
    void openCtrlDialog(int port);
private slots:
    void on_comboBox_currentIndexChanged(int index);
    void on_comboBox_2_currentIndexChanged(int index);
    void on_comboBox_3_currentIndexChanged(int index);
    void on_comboBox_4_currentIndexChanged(int index);
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
private:
    Ui::DialogInput *ui;
};

#endif // DIALOGINPUT_H
