#include "dialogplayinputrecord.h"
#include "ui_dialogplayinputrecord.h"

DialogPlayInputRecord::DialogPlayInputRecord(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPlayInputRecord)
{
    ui->setupUi(this);
}

DialogPlayInputRecord::~DialogPlayInputRecord()
{
    delete ui;
}
