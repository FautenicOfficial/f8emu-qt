#include "dialogeditbreakpoint.h"
#include "ui_dialogeditbreakpoint.h"

int dBreakpointAddr[256];
bool dBreakpointE[256];
bool dBreakpointX[256];
bool dBreakpointR[256];
bool dBreakpointW[256];
int numBreakpoints=0;
int thisIdx=0;
int thisAddr;
bool thisE;
bool thisX;
bool thisR;
bool thisW;

DialogEditBreakpoint::DialogEditBreakpoint(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEditBreakpoint)
{
    ui->setupUi(this);

    thisAddr = dBreakpointAddr[thisIdx];
    QString str = "";
    str.append(QString::asprintf("%04X",thisAddr));
    ui->lineEdit->setText(str);
    thisE = dBreakpointE[thisIdx];
    thisX = dBreakpointX[thisIdx];
    thisR = dBreakpointR[thisIdx];
    thisW = dBreakpointW[thisIdx];
    if(thisE) {
        ui->checkBox->setCheckState(Qt::CheckState::Checked);
    }
    if(thisX) {
        ui->checkBox_2->setCheckState(Qt::CheckState::Checked);
    }
    if(thisR) {
        ui->checkBox_3->setCheckState(Qt::CheckState::Checked);
    }
    if(thisW) {
        ui->checkBox_4->setCheckState(Qt::CheckState::Checked);
    }
}
DialogEditBreakpoint::~DialogEditBreakpoint()
{
    delete ui;
}
void DialogEditBreakpoint::on_checkBox_toggled(bool checked)
{
    thisE = checked;
}
void DialogEditBreakpoint::on_checkBox_2_toggled(bool checked)
{
    thisX = checked;
}
void DialogEditBreakpoint::on_checkBox_3_toggled(bool checked)
{
    thisR = checked;
}
void DialogEditBreakpoint::on_checkBox_4_toggled(bool checked)
{
    thisW = checked;
}
void DialogEditBreakpoint::on_lineEdit_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    int addr = ui->lineEdit->text().toInt(nullptr,16);
    if(addr>=0 && addr<=0xFFFF) {
        thisAddr = addr;
    }
}
