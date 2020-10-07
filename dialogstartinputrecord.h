#ifndef DIALOGSTARTINPUTRECORD_H
#define DIALOGSTARTINPUTRECORD_H

#include <QDialog>

namespace Ui {
class DialogStartInputRecord;
}

class DialogStartInputRecord : public QDialog
{
    Q_OBJECT

public:
    explicit DialogStartInputRecord(QWidget *parent = nullptr);
    ~DialogStartInputRecord();

private:
    Ui::DialogStartInputRecord *ui;
};

#endif // DIALOGSTARTINPUTRECORD_H
