#ifndef DIALOGPLAYINPUTRECORD_H
#define DIALOGPLAYINPUTRECORD_H

#include <QDialog>

namespace Ui {
class DialogPlayInputRecord;
}

class DialogPlayInputRecord : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPlayInputRecord(QWidget *parent = nullptr);
    ~DialogPlayInputRecord();

private:
    Ui::DialogPlayInputRecord *ui;
};

#endif // DIALOGPLAYINPUTRECORD_H
