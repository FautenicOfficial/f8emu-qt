#include "dialogmemorywatch.h"
#include "ui_dialogmemorywatch.h"

QListWidget * mwLWMap;
QTimer * mwLoopTimer;

DialogMemoryWatch::DialogMemoryWatch(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogMemoryWatch)
{
    ui->setupUi(this);
    ui->listWidget->installEventFilter(this);
    mwLWMap = ui->listWidget;

    mwLoopTimer = new QTimer;
    connect(mwLoopTimer,SIGNAL(timeout()),this,SLOT(mwDrawLoop()));
    mwLoopTimer->start(16);
}
DialogMemoryWatch::~DialogMemoryWatch()
{
    delete ui;
}
void DialogMemoryWatch::mwAddAddress(int addr)
{
    mwMemWatchAddr[numAddrs] = addr;
    numAddrs++;
}
void DialogMemoryWatch::mwDeleteAddress(int idx)
{
    if(idx<numAddrs) {
        numAddrs--;
        for(int i=idx; i<numAddrs; i++) {
            mwMemWatchAddr[i] = mwMemWatchAddr[i+1];
        }
    }
}
void DialogMemoryWatch::mwDrawLoop()
{
    mwLWMap->setUpdatesEnabled(false);
    thisIdx2 = mwLWMap->currentRow();
    mwLWMap->clear();
    for(int i=0; i<numAddrs; i++) {
        mwLWMap->addItem(QString::asprintf("%04X:   %02X",mwMemWatchAddr[i],gameImage[mwMemWatchAddr[i]]));
    }
    mwLWMap->setCurrentRow(thisIdx2);
    mwLWMap->setUpdatesEnabled(true);
}
void DialogMemoryWatch::on_pushButton_clicked()
{
    mwAddAddress(ui->lineEdit->text().toInt(nullptr,16));
}
void DialogMemoryWatch::on_pushButton_2_clicked()
{
    thisIdx2 = mwLWMap->currentRow();
    DialogEditAddress dea(this);
    if(dea.exec()==QDialog::Accepted) {
        mwMemWatchAddr[thisIdx2] = thisAddr2;
    }
}
void DialogMemoryWatch::on_pushButton_3_clicked()
{
    mwDeleteAddress(mwLWMap->currentRow());
}
