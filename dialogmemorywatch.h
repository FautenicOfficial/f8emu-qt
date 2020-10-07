#ifndef DIALOGMEMORYWATCH_H
#define DIALOGMEMORYWATCH_H

#include "6502.h"
#include "dialogeditaddress.h"
#include "ui_dialogeditaddress.h"
#include <QDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTimer>

extern QListWidget * mwLWMap;
extern QTimer * mwLoopTimer;

namespace Ui {
class DialogMemoryWatch;
}

class DialogMemoryWatch : public QDialog
{
    Q_OBJECT

public:
    explicit DialogMemoryWatch(QWidget *parent = nullptr);
    ~DialogMemoryWatch();
private slots:
    void mwAddAddress(int addr);
    void mwDeleteAddress(int idx);
    void mwDrawLoop();
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
private:
    Ui::DialogMemoryWatch *ui;
};

#endif // DIALOGMEMORYWATCH_H
