#ifndef DIALOGSETSAVESLOT_H
#define DIALOGSETSAVESLOT_H

#include <QDialog>

namespace Ui {
class DialogSetSaveSlot;
}

class DialogSetSaveSlot : public QDialog
{
    Q_OBJECT
public:
    explicit DialogSetSaveSlot(QWidget *parent = nullptr);
    ~DialogSetSaveSlot();
protected:
    void keyPressEvent(QKeyEvent *event);
private:
    Ui::DialogSetSaveSlot *ui;
};

#endif // DIALOGSETSAVESLOT_H
