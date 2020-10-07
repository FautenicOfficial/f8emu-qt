#include "dialogeditaddress.h"
#include "ui_dialogeditaddress.h"

int mwMemWatchAddr[256];
int numAddrs=0;
int thisIdx2=0;
int thisAddr2;

DialogEditAddress::DialogEditAddress(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEditAddress)
{
    ui->setupUi(this);

    thisAddr2 = mwMemWatchAddr[thisIdx2];
    QString str = "";
    str.append(QString::asprintf("%04X",thisAddr2));
    ui->lineEdit->setText(str);
}
DialogEditAddress::~DialogEditAddress()
{
    delete ui;
}
void DialogEditAddress::on_lineEdit_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    int addr = ui->lineEdit->text().toInt(nullptr,16);
    if(addr>=0 && addr<=0xFFFF) {
        thisAddr2 = addr;
    }
}
