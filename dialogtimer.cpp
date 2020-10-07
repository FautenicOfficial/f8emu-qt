#include "dialogtimer.h"
#include "ui_dialogtimer.h"

QTimer * tLoopTimer;
const int msecLut[3] = {0,17,33};

DialogTimer::DialogTimer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogTimer)
{
    ui->setupUi(this);

    tLoopTimer = new QTimer;
    connect(tLoopTimer,SIGNAL(timeout()),this,SLOT(tDrawLoop()));
    tLoopTimer->start(16);
}
DialogTimer::~DialogTimer()
{
    delete ui;
}

void DialogTimer::tDrawLoop()
{
    int hours = timerFrameVal/216000;
    int mins = (timerFrameVal/3600)%60;
    int secs = (timerFrameVal/60)%60;
    int msecs = timerFrameVal%60;
    msecs = ((msecs/3)*50)+msecLut[msecs%3];
    ui->label->setText(QString("Emulated Time: %1:%2:%3.%4").arg(
                           QString("%1").arg(hours,2,10,QChar('0')),
                           QString("%1").arg(mins,2,10,QChar('0')),
                           QString("%1").arg(secs,2,10,QChar('0')),
                           QString("%1").arg(msecs,3,10,QChar('0'))));
}

void DialogTimer::on_pushButton_clicked()
{
    isTimerRunning = true;
}
void DialogTimer::on_pushButton_2_clicked()
{
    isTimerRunning = false;
}
void DialogTimer::on_pushButton_3_clicked()
{
    timerFrameVal = 0;
}
