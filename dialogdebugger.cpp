#include "dialogdebugger.h"
#include "ui_dialogdebugger.h"

int dStartAddress = 0x8000;
QPlainTextEdit * dPTEMap;
QTimer * dLoopTimer;

const char * opcodeStr[256] = {"brk","ora ($%02X,x)","nop","nop","tsb $%02X","ora $%02X","asl $%02X","rmb0 $%02X",
                               "php","ora #$%02X","asl a","nop","tsb $%04X","ora $%04X","asl $%04X","bbr0 $%02X,$%04X",
                               "bpl $%04X","ora ($%02X),y","ora ($%02X)","nop","trb $%02X","ora $%02X,x","asl $%02X,x","rmb1 $%02X",
                               "clc","ora $%04X,y","inc a","nop","trb $%04X","ora $%04X,x","asl $%04X,x","bbr1 $%02X,$%04X",
                               "jsr $%04X","and ($%02X,x)","nop","nop","bit $%02X","and $%02X","rol $%02X","rmb2 $%02X",
                               "plp","and #$%02X","rol a","nop","bit $%04X","and $%04X","rol $%04X","bbr2 $%02X,$%04X",
                               "bmi $%04X","and ($%02X),y","and ($%02X)","nop","bit $%02X,x","and $%02X,x","rol $%02X,x","rmb3 $%02X",
                               "sec","and $%04X,y","dec a","nop","bit $%04X,x","and $%04X,x","rol $%04X,x","bbr3 $%02X,$%04X",
                               "rti","eor ($%02X,x)","nop","nop","nop","eor $%02X","lsr $%02X","rmb4 $%02X",
                               "pha","eor #$%02X","lsr a","nop","jmp $%04X","eor $%04X","lsr $%04X","bbr4 $%02X,$%04X",
                               "bvc $%04X","eor ($%02X),y","eor ($%02X)","nop","nop","eor $%02X,x","lsr $%02X,x","rmb5 $%02X",
                               "cli","eor $%04X,y","phy","nop","nop","eor $%04X,x","lsr $%04X,x","bbr5 $%02X,$%04X",
                               "rts","adc ($%02X,x)","nop","nop","stz $%02X","adc $%02X","ror $%02X","rmb6 $%02X",
                               "pla","adc #$%02X","ror a","nop","jmp ($%04X)","adc $%04X","ror $%04X","bbr6 $%02X,$%04X",
                               "bvs $%04X","adc ($%02X),y","adc ($%02X)","nop","stz $%02X,x","adc $%02X,x","ror $%02X,x","rmb7 $%02X",
                               "sei","adc $%04X,y","ply","nop","jmp ($%04X,x)","adc $%04X,x","ror $%04X,x","bbr7 $%02X,$%04X",
                               "bra $%04X","sta ($%02X,x)","nop","nop","sty $%02X","sta $%02X","stx $%02X","smb0 $%02X",
                               "dey","bit #$%02X","txa","nop","sty $%04X","sta $%04X","stx $%04X","bbs0 $%02X,$%04X",
                               "bcc $%04X","sta ($%02X),y","sta ($%02X)","nop","sty $%02X,x","sta $%02X,x","stx $%02X,y","smb1 $%02X",
                               "tya","sta $%04X,y","txs","nop","stz $%04X","sta $%04X,x","stz $%04X,x","bbs1 $%02X,$%04X",
                               "ldy #$%02X","lda ($%02X,x)","ldx #$%02X","nop","ldy $%02X","lda $%02X","ldx $%02X","smb2 $%02X",
                               "tay","lda #$%02X","tax","nop","ldy $%04X","lda $%04X","ldx $%04X","bbs2 $%02X,$%04X",
                               "bcs $%04X","lda ($%02X),y","lda ($%02X)","nop","ldy $%02X,x","lda $%02X,x","ldx $%02X,y","smb3 $%02X",
                               "clv","lda $%04X,y","tsx","nop","ldy $%04X,x","lda $%04X,x","ldx $%04X,y","bbs3 $%02X,$%04X",
                               "cpy #$%02X","cmp ($%02X,x)","nop","nop","cpy $%02X","cmp $%02X","dec $%02X","smb4 $%02X",
                               "iny","cmp #$%02X","dex","wai","cpy $%04X","cmp $%04X","dec $%04X","bbs4 $%02X,$%04X",
                               "bne $%04X","cmp ($%02X),y","cmp ($%02X)","nop","nop","cmp $%02X,x","dec $%02X,x","smb5 $%02X",
                               "cld","cmp $%04X,y","phx","stp","nop","cmp $%04X,x","dec $%04X,x","bbs5 $%02X,$%04X",
                               "cpx #$%02X","sbc ($%02X,x)","nop","nop","cpx $%02X","sbc $%02X","inc $%02X","smb6 $%02X",
                               "inx","sbc #$%02X","nop","nop","cpx $%04X","sbc $%04X","inc $%04X","bbs6 $%02X,$%04X",
                               "beq $%04X","sbc ($%02X),y","sbc ($%02X)","nop","nop","sbc $%02X,x","inc $%02X,x","smb7 $%02X",
                               "sed","sbc $%04X,y","plx","nop","nop","sbc $%04X,x","inc $%04X,x","bbs7 $%02X,$%04X"};

const int opcodeLen[256] = {1,2,-2,-1, 2,2,2,2,1,2,1,-1, 3,3,3,0,
                            0,2, 2,-1, 2,2,2,2,1,3,1,-1, 3,3,3,0,
                            3,2,-2,-1, 2,2,2,2,1,2,1,-1, 3,3,3,0,
                            0,2, 2,-1, 2,2,2,2,1,3,1,-1, 3,3,3,0,
                            1,2,-2,-1,-2,2,2,2,1,2,1,-1, 3,3,3,0,
                            0,2, 2,-1,-2,2,2,2,1,3,1,-1,-3,3,3,0,
                            1,2,-2,-1, 2,2,2,2,1,2,1,-1, 3,3,3,0,
                            0,2, 2,-1, 2,2,2,2,1,3,1,-1, 3,3,3,0,
                            0,2,-2,-1, 2,2,2,2,1,2,1,-1, 3,3,3,0,
                            0,2, 2,-1, 2,2,2,2,1,3,1,-1, 3,3,3,0,
                            2,2, 2,-1, 2,2,2,2,1,2,1,-1, 3,3,3,0,
                            0,2, 2,-1, 2,2,2,2,1,3,1,-1, 3,3,3,0,
                            2,2,-2,-1, 2,2,2,2,1,2,1,-1, 3,3,3,0,
                            0,2, 2,-1,-2,2,2,2,1,3,1,-1,-3,3,3,0,
                            2,2,-2,-1, 2,2,2,2,1,2,1,-1, 3,3,3,0,
                            0,2, 2,-1,-2,2,2,2,1,3,1,-1,-3,3,3,0};

DialogDebugger::DialogDebugger(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogDebugger)
{
    ui->setupUi(this);
    ui->plainTextEdit->installEventFilter(this);
    dPTEMap = ui->plainTextEdit;

    ui->checkBox->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->checkBox_2->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->checkBox_3->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->checkBox_4->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->checkBox_5->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->checkBox_6->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->checkBox_7->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->checkBox_8->setAttribute(Qt::WA_TransparentForMouseEvents);

    dLoopTimer = new QTimer;
    connect(dLoopTimer,SIGNAL(timeout()),this,SLOT(dDrawLoop()));
    dLoopTimer->start(16);
}
DialogDebugger::~DialogDebugger()
{
    dLoopTimer->stop();
    delete ui;
}

QString dGetOpcodeString(int addr) {
    int opcode = gameImage[addr];
    int zp = gameImage[addr+1];
    int abs = zp|(gameImage[addr+2]<<8);
    switch(opcodeLen[opcode])
    {
        case -3:
        case -2:
        case -1:
        case 1:
        {
            return opcodeStr[opcode];
        }
        case 0:
        {
            if((opcode&0xF)==0xF)
            {
                zp = abs>>8;
                if(zp&0x80) zp-=0x100;
                return QString::asprintf(opcodeStr[opcode],abs&0xFF,zp+addr+2);
            }
            else
            {
                if(zp&0x80) zp-=0x100;
                return QString::asprintf(opcodeStr[opcode],zp+addr+2);
            }
        }
        case 2:
        {
            return QString::asprintf(opcodeStr[opcode],zp);
        }
        case 3:
        {
            return QString::asprintf(opcodeStr[opcode],abs);
        }
        default:
        {
            return "";
        }
    }
}
void DialogDebugger::dDrawInfo() {
    ui->label->setText(QString::asprintf("PC: %04X",pc_reg));
    ui->label_2->setText(QString::asprintf("A: %02X",a_reg));
    ui->label_3->setText(QString::asprintf("X: %02X",x_reg));
    ui->label_4->setText(QString::asprintf("Y: %02X",y_reg));
    ui->label_5->setText(QString::asprintf("Stack Pointer: %03X",s_reg|0x100));
    ui->label_6->setText(QString::asprintf("Flags: %02X",flag_reg));

    ui->checkBox->setCheckState(Qt::CheckState::Unchecked);
    ui->checkBox_2->setCheckState(Qt::CheckState::Unchecked);
    ui->checkBox_3->setCheckState(Qt::CheckState::Unchecked);
    ui->checkBox_4->setCheckState(Qt::CheckState::Unchecked);
    ui->checkBox_5->setCheckState(Qt::CheckState::Unchecked);
    ui->checkBox_6->setCheckState(Qt::CheckState::Unchecked);
    ui->checkBox_7->setCheckState(Qt::CheckState::Unchecked);
    ui->checkBox_8->setCheckState(Qt::CheckState::Unchecked);
    if(flag_reg&0x80) {
        ui->checkBox->setCheckState(Qt::CheckState::Checked);
    }
    if(flag_reg&0x40) {
        ui->checkBox_2->setCheckState(Qt::CheckState::Checked);
    }
    if(flag_reg&0x20) {
        ui->checkBox_3->setCheckState(Qt::CheckState::Checked);
    }
    if(flag_reg&0x10) {
        ui->checkBox_4->setCheckState(Qt::CheckState::Checked);
    }
    if(flag_reg&0x08) {
        ui->checkBox_5->setCheckState(Qt::CheckState::Checked);
    }
    if(flag_reg&0x04) {
        ui->checkBox_6->setCheckState(Qt::CheckState::Checked);
    }
    if(flag_reg&0x02) {
        ui->checkBox_7->setCheckState(Qt::CheckState::Checked);
    }
    if(flag_reg&0x01) {
        ui->checkBox_8->setCheckState(Qt::CheckState::Checked);
    }
}
void DialogDebugger::dClearInfo() {
    ui->label->setText("PC:");
    ui->label_2->setText("A:");
    ui->label_3->setText("X:");
    ui->label_4->setText("Y:");
    ui->label_5->setText("Stack Pointer:");
    ui->label_6->setText("Flags:");
    ui->checkBox->setCheckState(Qt::CheckState::Unchecked);
    ui->checkBox_2->setCheckState(Qt::CheckState::Unchecked);
    ui->checkBox_3->setCheckState(Qt::CheckState::Unchecked);
    ui->checkBox_4->setCheckState(Qt::CheckState::Unchecked);
    ui->checkBox_5->setCheckState(Qt::CheckState::Unchecked);
    ui->checkBox_6->setCheckState(Qt::CheckState::Unchecked);
    ui->checkBox_7->setCheckState(Qt::CheckState::Unchecked);
    ui->checkBox_8->setCheckState(Qt::CheckState::Unchecked);
}
void DialogDebugger::dUpdateBreakpoints() {
    ui->listWidget->clear();
    for(int i=0; i<numBreakpoints; i++) {
        ui->listWidget->addItem(QString::asprintf("%04X:   %c%c%c%c\r\n",dBreakpointAddr[i],dBreakpointE[i]?'E':'-',dBreakpointX[i]?'X':'-',dBreakpointR[i]?'R':'-',dBreakpointW[i]?'W':'-'));
    }
}
void DialogDebugger::dAddBreakpoint(int addr) {
    dBreakpointAddr[numBreakpoints] = addr;
    dBreakpointE[numBreakpoints] = true;
    dBreakpointX[numBreakpoints] = true;
    dBreakpointR[numBreakpoints] = true;
    dBreakpointW[numBreakpoints] = true;
    numBreakpoints++;
    dUpdateBreakpoints();
}
void DialogDebugger::dDeleteBreakpoint(int idx) {
    if(idx<numBreakpoints) {
        numBreakpoints--;
        for(int i=idx; i<numBreakpoints; i++) {
            dBreakpointAddr[i] = dBreakpointAddr[i+1];
            dBreakpointE[i] = dBreakpointE[i+1];
            dBreakpointX[i] = dBreakpointX[i+1];
            dBreakpointR[i] = dBreakpointR[i+1];
            dBreakpointW[i] = dBreakpointW[i+1];
        }
    }
    dUpdateBreakpoints();
}
void DialogDebugger::dDrawLoop() {
    QString str = "";
    int laddr = dStartAddress;
    for(int l=0; l<40; l++) {
        int len = abs(opcodeLen[gameImage[laddr]]);
        if(len==0)
        {
            len = 2;
            if((gameImage[laddr]&0xF)==0xF) len++;
        }
        if((laddr+len)>0xFFFF) break;
        str.append(QString::asprintf("%04X:   ",laddr));
        str.append(dGetOpcodeString(laddr));
        laddr+=len;
        str.append("\r\n");
    }
    dPTEMap->setPlainText(str);
    if(!isExec) {dDrawInfo();}
}

void DialogDebugger::on_pushButton_clicked()
{
    int addr = ui->lineEdit->text().toInt(nullptr,16);
    if(addr>=0 && addr<=0xFFFF) {
        dStartAddress = addr;
    }
}
void DialogDebugger::on_pushButton_2_clicked()
{
    dAddBreakpoint(ui->lineEdit->text().toInt(nullptr,16));
}
void DialogDebugger::on_pushButton_3_clicked()
{
    thisIdx = ui->listWidget->currentRow();
    DialogEditBreakpoint deb(this);
    if(deb.exec()==QDialog::Accepted) {
        dBreakpointAddr[thisIdx] = thisAddr;
        dBreakpointE[thisIdx] = thisE;
        dBreakpointX[thisIdx] = thisX;
        dBreakpointR[thisIdx] = thisR;
        dBreakpointW[thisIdx] = thisW;
        dUpdateBreakpoints();
    }
}
void DialogDebugger::on_pushButton_4_clicked()
{
    dDeleteBreakpoint(ui->listWidget->currentRow());
}
void DialogDebugger::on_pushButton_5_clicked()
{
    isExec=true;
    dClearInfo();
}
