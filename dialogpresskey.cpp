#include "dialogpresskey.h"
#include "ui_dialogpresskey.h"
#include <QtGui>

DialogPressKey::DialogPressKey(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPressKey)
{
    ui->setupUi(this);
    ui->label->installEventFilter(this);
}
DialogPressKey::~DialogPressKey()
{
    delete ui;
}
void DialogPressKey::keyPressEvent(QKeyEvent *event)
{
    int k = event->key();
    done(k);
}
