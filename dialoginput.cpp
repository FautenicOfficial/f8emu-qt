#include "mainwindow.h"
#include "dialoginput.h"
#include "ui_dialoginput.h"

DialogInput::DialogInput(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogInput)
{
    ui->setupUi(this);

    if(!ctrlMap["Port1CtrlType"].isValid()) {
        ui->comboBox->setCurrentIndex(1);
    } else {
        ui->comboBox->setCurrentIndex(ctrlMap["Port1CtrlType"].toInt());
    }
    if(!ctrlMap["Port2CtrlType"].isValid()) {
        ui->comboBox_2->setCurrentIndex(0);
    } else {
        ui->comboBox_2->setCurrentIndex(ctrlMap["Port2CtrlType"].toInt());
    }
    if(!ctrlMap["Port3CtrlType"].isValid()) {
        ui->comboBox_3->setCurrentIndex(0);
    } else {
        ui->comboBox_3->setCurrentIndex(ctrlMap["Port3CtrlType"].toInt());
    }
    if(!ctrlMap["Port4CtrlType"].isValid()) {
        ui->comboBox_4->setCurrentIndex(0);
    } else {
        ui->comboBox_4->setCurrentIndex(ctrlMap["Port4CtrlType"].toInt());
    }

    if(ui->comboBox->currentIndex()==0) {
        ui->pushButton->setEnabled(false);
    } else {
        ui->pushButton->setEnabled(true);
    }
    if(ui->comboBox_2->currentIndex()==0) {
        ui->pushButton_2->setEnabled(false);
    } else {
        ui->pushButton_2->setEnabled(true);
    }
    if(ui->comboBox_3->currentIndex()==0) {
        ui->pushButton_3->setEnabled(false);
    } else {
        ui->pushButton_3->setEnabled(true);
    }
    if(ui->comboBox_4->currentIndex()==0) {
        ui->pushButton_4->setEnabled(false);
    } else {
        ui->pushButton_4->setEnabled(true);
    }
}
DialogInput::~DialogInput()
{
    delete ui;
}

void DialogInput::openCtrlDialog(int port)
{
    QMap<QString, QVariant> tempMap = ctrlMap = ctrlSettings->value("InputSettings").toMap();
    curPortToCfg = port;
    int idx = 1;
    switch(port) {
        case 1:
        {
            idx = ui->comboBox->currentIndex();
            break;
        }
        case 2:
        {
            idx = ui->comboBox_2->currentIndex();
            break;
        }
        case 3:
        {
            idx = ui->comboBox_3->currentIndex();
            break;
        }
        case 4:
        {
            idx = ui->comboBox_4->currentIndex();
            break;
        }
    }
    switch(idx) {
        case 1: {
            DialogStdCtrl dsc(this);
            if(dsc.exec()==QDialog::Rejected)
            {
                ctrlSettings->setValue("InputSettings",tempMap);
            } else {
                ctrlSettings->setValue("InputSettings",ctrlMap);
            }
            break;
        }
        case 2: {
            DialogExtCtrl dec(this);
            if(dec.exec()==QDialog::Rejected)
            {
                ctrlSettings->setValue("InputSettings",tempMap);
            } else {
                ctrlSettings->setValue("InputSettings",ctrlMap);
            }
            break;
        }
    }
}

void DialogInput::on_comboBox_currentIndexChanged(int index)
{
    if(index==0) {
        ui->pushButton->setEnabled(false);
    } else {
        ui->pushButton->setEnabled(true);
    }
    ctrlMap.insert("Port1CtrlType",index);
}
void DialogInput::on_comboBox_2_currentIndexChanged(int index)
{
    if(index==0) {
        ui->pushButton_2->setEnabled(false);
    } else {
        ui->pushButton_2->setEnabled(true);
    }
    ctrlMap.insert("Port2CtrlType",index);
}
void DialogInput::on_comboBox_3_currentIndexChanged(int index)
{
    if(index==0) {
        ui->pushButton_3->setEnabled(false);
    } else {
        ui->pushButton_3->setEnabled(true);
    }
    ctrlMap.insert("Port3CtrlType",index);
}
void DialogInput::on_comboBox_4_currentIndexChanged(int index)
{
    if(index==0) {
        ui->pushButton_4->setEnabled(false);
    } else {
        ui->pushButton_4->setEnabled(true);
    }
    ctrlMap.insert("Port4CtrlType",index);
}
void DialogInput::on_pushButton_clicked()
{
    this->openCtrlDialog(1);
}
void DialogInput::on_pushButton_2_clicked()
{
    this->openCtrlDialog(2);
}
void DialogInput::on_pushButton_3_clicked()
{
    this->openCtrlDialog(3);
}
void DialogInput::on_pushButton_4_clicked()
{
    this->openCtrlDialog(4);
}
