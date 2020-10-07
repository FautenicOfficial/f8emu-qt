#include "dialogsetsaveslot.h"
#include "ui_dialogsetsaveslot.h"
#include <QtGui>

DialogSetSaveSlot::DialogSetSaveSlot(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSetSaveSlot)
{
    ui->setupUi(this);
    ui->label->installEventFilter(this);
}
DialogSetSaveSlot::~DialogSetSaveSlot()
{
    delete ui;
}
void DialogSetSaveSlot::keyPressEvent(QKeyEvent *event)
{
    int k = event->key();
    if(k<'0' || k>'9') {k=-1;}
    done(k);
}

