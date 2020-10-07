#include "mainwindow.h"
#include "dialogextctrl.h"
#include "ui_dialogextctrl.h"

DialogExtCtrl::DialogExtCtrl(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogExtCtrl)
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
    dst = "Port%1L";
    dst = dst.arg(curPortToCfg);
    if(ctrlMap[dst].isValid()) {
        ui->pushButton_9->setText(QKeySequence(ctrlMap[dst].toInt()).toString());
    }
    dst = "Port%1R";
    dst = dst.arg(curPortToCfg);
    if(ctrlMap[dst].isValid()) {
        ui->pushButton_10->setText(QKeySequence(ctrlMap[dst].toInt()).toString());
    }
    dst = "Port%1X";
    dst = dst.arg(curPortToCfg);
    if(ctrlMap[dst].isValid()) {
        ui->pushButton_11->setText(QKeySequence(ctrlMap[dst].toInt()).toString());
    }
    dst = "Port%1Y";
    dst = dst.arg(curPortToCfg);
    if(ctrlMap[dst].isValid()) {
        ui->pushButton_12->setText(QKeySequence(ctrlMap[dst].toInt()).toString());
    }
    dst = "Port%1Z";
    dst = dst.arg(curPortToCfg);
    if(ctrlMap[dst].isValid()) {
        ui->pushButton_13->setText(QKeySequence(ctrlMap[dst].toInt()).toString());
    }
}
DialogExtCtrl::~DialogExtCtrl()
{
    delete ui;
}

void DialogExtCtrl::openKeyDialog(QString dst,QPushButton * target)
{
    dst = dst.arg(curPortToCfg);
    dpk = new DialogPressKey();
    int y = dpk->exec();
    ctrlMap.insert(dst,y);
    target->setText(QKeySequence(y).toString());
}

void DialogExtCtrl::on_pushButton_clicked()
{
    this->openKeyDialog("Port%1Up",ui->pushButton);
}
void DialogExtCtrl::on_pushButton_2_clicked()
{
    this->openKeyDialog("Port%1Down",ui->pushButton_2);
}
void DialogExtCtrl::on_pushButton_3_clicked()
{
    this->openKeyDialog("Port%1Left",ui->pushButton_3);
}
void DialogExtCtrl::on_pushButton_4_clicked()
{
    this->openKeyDialog("Port%1Right",ui->pushButton_4);
}
void DialogExtCtrl::on_pushButton_5_clicked()
{
    this->openKeyDialog("Port%1Start",ui->pushButton_5);
}
void DialogExtCtrl::on_pushButton_6_clicked()
{
    this->openKeyDialog("Port%1A",ui->pushButton_6);
}
void DialogExtCtrl::on_pushButton_7_clicked()
{
    this->openKeyDialog("Port%1B",ui->pushButton_7);
}
void DialogExtCtrl::on_pushButton_8_clicked()
{
    this->openKeyDialog("Port%1C",ui->pushButton_8);
}
void DialogExtCtrl::on_pushButton_9_clicked()
{
    this->openKeyDialog("Port%1L",ui->pushButton_9);
}
void DialogExtCtrl::on_pushButton_10_clicked()
{
    this->openKeyDialog("Port%1R",ui->pushButton_10);
}
void DialogExtCtrl::on_pushButton_11_clicked()
{
    this->openKeyDialog("Port%1X",ui->pushButton_11);
}
void DialogExtCtrl::on_pushButton_12_clicked()
{
    this->openKeyDialog("Port%1Y",ui->pushButton_12);
}
void DialogExtCtrl::on_pushButton_13_clicked()
{
    this->openKeyDialog("Port%1Z",ui->pushButton_13);
}
