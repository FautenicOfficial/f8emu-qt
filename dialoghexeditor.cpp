#include "dialoghexeditor.h"
#include "ui_dialoghexeditor.h"

QPlainTextEdit * hePTEMap;
QTimer * heLoopTimer;
int heStartAddress = 0;

DialogHexEditor::DialogHexEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogHexEditor)
{
    ui->setupUi(this);
    ui->plainTextEdit->installEventFilter(this);
    hePTEMap = ui->plainTextEdit;

    heLoopTimer = new QTimer;
    connect(heLoopTimer,SIGNAL(timeout()),this,SLOT(heDrawLoop()));
    heLoopTimer->start(16);
}
DialogHexEditor::~DialogHexEditor()
{
    heLoopTimer->stop();
    delete ui;
}
void DialogHexEditor::heDrawLoop() {
    QString str = "";
    int startAddr = (heStartAddress<0xFE00)?heStartAddress:0xFE00;
    for(int l=0; l<32; l++) {
        int laddr = startAddr+(l<<4);
        str.append(QString::asprintf("%04X",laddr));
        str.append(":   ");
        for(int b=0; b<16; b++) {
            str.append(' ');
            str.append(QString::asprintf("%02X",gameImage[laddr+b]));
        }
        str.append("\r\n");
    }
    hePTEMap->setPlainText(str);
}
void DialogHexEditor::heSetNybble(WORD addr,char data,bool hi) {
    if(hi) {
        gameImage[addr]&=0x0F;
        gameImage[addr]|=(data<<4);
    } else {
        gameImage[addr]&=0xF0;
        gameImage[addr]|=data;
    }
}
void DialogHexEditor::on_pushButton_clicked()
{
    int addr = ui->lineEdit->text().toInt(nullptr,16);
    if(addr>=0 && addr<=0xFFFF) {
        heStartAddress = addr;
    }
}
void DialogHexEditor::on_pushButton_2_clicked()
{
    int addr = ui->lineEdit->text().toInt(nullptr,16);
    int data = ui->lineEdit_2->text().toInt(nullptr,16);
    if(addr>=0 && addr<=0xFFFF && data>=0 && data<=0xFF) {
        put6502memory(static_cast<WORD>(addr),static_cast<BYTE>(data));
    }
}
void DialogHexEditor::on_pushButton_3_clicked()
{
    int taddr = heStartAddress, taddr2 = heStartAddress;
    bool matchFound = false;
    int searchVal = ui->lineEdit_2->text().toInt(nullptr,16);
    int comboIndex = ui->comboBox->currentIndex();
    bool isSigned = (ui->checkBox->checkState()==Qt::CheckState::Checked)?true:false;
    bool searchBackwards = (ui->checkBox_2->checkState()==Qt::CheckState::Checked)?true:false;
    if(searchBackwards==false && taddr!=0) {taddr++;}
    if(searchBackwards && taddr!=0xFFFF) {taddr--;}
    while(true) {
        if(comboIndex==0) {
            if(gameImage[taddr]==searchVal) {
                matchFound = true;
            }
        } else if(comboIndex==1) {
            if(gameImage[taddr]!=searchVal) {
                matchFound = true;
            }
        } else {
            int newM = gameImage[taddr];
            int newS = searchVal;
            if(isSigned) {
                if(newM&0x80) {newM-=0x100;}
                if(newS&0x80) {newS-=0x100;}
            }
            switch(comboIndex) {
            case 2: {
                if(newM<newS) {
                    matchFound = true;
                }
                break;
            }
            case 3: {
                if(newM<=newS) {
                    matchFound = true;
                }
                break;
            }
            case 4: {
                if(newM>newS) {
                    matchFound = true;
                }
                break;
            }
            case 5: {
                if(newM>=newS) {
                    matchFound = true;
                }
                break;
            }
            }
        }

        if(matchFound) {heStartAddress=taddr; break;}
        if(searchBackwards) {
            taddr--;
            if(taddr<0) {
                heStartAddress = taddr2;
                break;
            }
        } else {
            taddr++;
            if(taddr>0xFFFF) {
                heStartAddress = taddr2;
                break;
            }
        }
    }
}
