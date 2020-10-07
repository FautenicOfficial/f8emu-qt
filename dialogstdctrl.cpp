#include "mainwindow.h"
#include "dialogstdctrl.h"
#include "ui_dialogstdctrl.h"

DialogStdCtrl::DialogStdCtrl(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogStdCtrl)
{
    ui->setupUi(this);

    QString dst = "Port%1Up";
    dst = dst.arg(curPortToCfg);
    if(ctrlMap[dst].isValid()) {
        ui->pushButton->setText(QKeySequence(ctrlMap[dst].toInt()).toString());
    }
    dst = "Port%1Down";
    dst = dst.arg(curPortToCfg);
    if(ctrlMap[dst].isValid()) {
        ui->pushButton_2->setText(QKeySequence(ctrlMap[dst].toInt()).toString());
    }
    dst = "Port%1Left";
    dst = dst.arg(curPortToCfg);
    if(ctrlMap[dst].isValid()) {
        ui->pushButton_3->setText(QKeySequence(ctrlMap[dst].toInt()).toString());
    }
    dst = "Port%1Right";
    dst = dst.arg(curPortToCfg);
    if(ctrlMap[dst].isValid()) {
        ui->pushButton_4->setText(QKeySequence(ctrlMap[dst].toInt()).toString());
    }
    dst = "Port%1Start";
    dst = dst.arg(curPortToCfg);
    if(ctrlMap[dst].isValid()) {
        ui->pushButton_5->setText(QKeySequence(ctrlMap[dst].toInt()).toString());
    }
    dst = "Port%1A";
    dst = dst.arg(curPortToCfg);
    if(ctrlMap[dst].isValid()) {
        ui->pushButton_6->setText(QKeySequence(ctrlMap[dst].toInt()).toString());
    }
    dst = "Port%1B";
    dst = dst.arg(curPortToCfg);
    if(ctrlMap[dst].isValid()) {
        ui->pushButton_7->setText(QKeySequence(ctrlMap[dst].toInt()).toString());
    }
    dst = "Port%1C";
    dst = dst.arg(curPortToCfg);
    if(ctrlMap[dst].isValid()) {
        ui->pushButton_8->setText(QKeySequence(ctrlMap[dst].toInt()).toString());
    }
}
DialogStdCtrl::~DialogStdCtrl()
{
    delete ui;
}

void DialogStdCtrl::openKeyDialog(QString dst,QPushButton * target)
{
    dst = dst.arg(curPortToCfg);
    dpk = new DialogPressKey();
    int y = dpk->exec();
    ctrlMap.insert(dst,y);
    target->setText(QKeySequence(y).toString());
}

void DialogStdCtrl::on_pushButton_clicked()
{
    this->openKeyDialog("Port%1Up",ui->pushButton);
}
void DialogStdCtrl::on_pushButton_2_clicked()
{
    this->openKeyDialog("Port%1Down",ui->pushButton_2);
}
void DialogStdCtrl::on_pushButton_3_clicked()
{
    this->openKeyDialog("Port%1Left",ui->pushButton_3);
}
void DialogStdCtrl::on_pushButton_4_clicked()
{
    this->openKeyDialog("Port%1Right",ui->pushButton_4);
}
void DialogStdCtrl::on_pushButton_5_clicked()
{
    this->openKeyDialog("Port%1Start",ui->pushButton_5);
}
void DialogStdCtrl::on_pushButton_6_clicked()
{
    this->openKeyDialog("Port%1A",ui->pushButton_6);
}
void DialogStdCtrl::on_pushButton_7_clicked()
{
    this->openKeyDialog("Port%1B",ui->pushButton_7);
}
void DialogStdCtrl::on_pushButton_8_clicked()
{
    this->openKeyDialog("Port%1C",ui->pushButton_8);
}
