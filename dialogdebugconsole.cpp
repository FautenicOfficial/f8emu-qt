#include "dialogdebugconsole.h"
#include "ui_dialogdebugconsole.h"

QPlainTextEdit * dcPTEMap;
QTimer * dcLoopTimer;
//char textBuffer[(33*24)+1];
//int textPtr = 0;

DialogDebugConsole::DialogDebugConsole(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogDebugConsole)
{
    ui->setupUi(this);
    ui->plainTextEdit->installEventFilter(this);
    dcPTEMap = ui->plainTextEdit;

    dcLoopTimer = new QTimer;
    connect(dcLoopTimer,SIGNAL(timeout()),this,SLOT(dcDrawLoop()));
    dcLoopTimer->start(1);
}
DialogDebugConsole::~DialogDebugConsole()
{
    delete ui;
}

void DialogDebugConsole::dcDrawLoop() {
    if(dbgNewChar) {
        dbgMutex = true;

        /*if(dbgChar==9) {
            textBuffer[textPtr++] = ' ';
            int tpx = textPtr%33;
            while(tpx&3) {
                textBuffer[textPtr++] = ' ';
                tpx++;
            }
            if(tpx==32) {textPtr++;}
            if(textPtr==(33*24)) {
            }
        } else if(dbgChar==10) {

        } else if(dbgChar==11) {
        } else if(dbgChar==12) {
            for(int i=0; i<24; i++) {
                for(int j=0; j<33; j++) {
                    textBuffer[(i*33)+j] = (j==32)?'\n':0;
                }
            }
            textPtr = 0;
        } else {

        }*/
        if(dbgChar==12) {
            dcPTEMap->clear();
        } else {
            dcPTEMap->moveCursor(QTextCursor::End);
            dcPTEMap->insertPlainText(QString(dbgChar));
        }

        dbgMutex = false;
        dbgNewChar = false;
    }
    if(dbgHaltInit) {

        int errCode = gameImage[0x742A];
        if(gameImage[0x7428]==2) {
            int errAddr = errCode|(gameImage[0x742B]<<8);
            errCode = gameImage[errAddr];
        }
        QString errMsg = "Unspecified error";
        if(errCode==0) {errMsg = "Main exited";}
        else if(errCode==0xFF) {errMsg = "General error";}
        dcPTEMap->moveCursor(QTextCursor::End);
        dcPTEMap->insertPlainText(QString("EXECUTION HALTED\n================\nError code %1: %2.\n\n").arg(QString("%1").arg(errCode,2,16,QChar('0')),errMsg));

        int errPc = (gameImage[0x102+s_reg]|(gameImage[0x103+s_reg]<<8))-1;
        dcPTEMap->moveCursor(QTextCursor::End);
        dcPTEMap->insertPlainText(QString("REGISTERS\n=========\nA:  %1\nX:  %2\nY:  %3\nP:  %4\nS:  %5\nPC: %6\n\nZERO PAGE\n=========\n").arg(
            QString("%1").arg(a_reg,2,16,QChar('0')),
            QString("%1").arg(x_reg,2,16,QChar('0')),
            QString("%1").arg(y_reg,2,16,QChar('0')),
            QString("%1").arg(flag_reg,2,16,QChar('0')),
            QString("%1").arg(s_reg,4,16,QChar('0')),
            QString("%1").arg(errPc,4,16,QChar('0'))));

        for(int i=0; i<256; i++) {
            dcPTEMap->moveCursor(QTextCursor::End);
            dcPTEMap->insertPlainText(QString("%1 ").arg(gameImage[i],2,16,QChar('0')));
            if((i&15)==15) {
                dcPTEMap->moveCursor(QTextCursor::End);
                dcPTEMap->insertPlainText("\n");
            }
        }

        dcPTEMap->moveCursor(QTextCursor::End);
        dcPTEMap->insertPlainText("\nSTACK\n=====\n");
        for(int i=0; i<256; i++) {
            dcPTEMap->moveCursor(QTextCursor::End);
            dcPTEMap->insertPlainText(QString("%1 ").arg(gameImage[i+256],2,16,QChar('0')));
            if((i&15)==15) {
                dcPTEMap->moveCursor(QTextCursor::End);
                dcPTEMap->insertPlainText("\n");
            }
        }

        dbgHaltInit = false;
        dbgHalt = true;
    }
}
