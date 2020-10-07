#include "dialogstartinputrecord.h"
#include "ui_dialogstartinputrecord.h"

DialogStartInputRecord::DialogStartInputRecord(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogStartInputRecord)
{
    ui->setupUi(this);
}

DialogStartInputRecord::~DialogStartInputRecord()
{
    delete ui;
}
