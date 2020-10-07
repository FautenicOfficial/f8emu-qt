#ifndef DIALOGPRESSKEY_H
#define DIALOGPRESSKEY_H

#include <QDialog>

namespace Ui {
class DialogPressKey;
}

class DialogPressKey : public QDialog
{
    Q_OBJECT
public:
    explicit DialogPressKey(QWidget *parent = nullptr);
    ~DialogPressKey();
protected:
    void keyPressEvent(QKeyEvent *event);
private:
    Ui::DialogPressKey *ui;
};

#endif // DIALOGPRESSKEY_H
