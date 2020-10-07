#ifndef DIALOGBACKGROUNDVIEWER_H
#define DIALOGBACKGROUNDVIEWER_H

#include "6502.h"
#include <QDialog>
#include <QLabel>
#include <QImage>
#include <QPixmap>
#include <QTimer>

extern unsigned char bvPixels[512*512*4];
extern QLabel * bvLabelMap;
extern QTimer * bvLoopTimer;

namespace Ui {
class DialogBackgroundViewer;
}

class DialogBackgroundViewer : public QDialog
{
    Q_OBJECT
public:
    explicit DialogBackgroundViewer(QWidget *parent = nullptr);
    ~DialogBackgroundViewer();
private slots:
    void bvDrawLoop();
private:
    Ui::DialogBackgroundViewer *ui;
};

#endif // DIALOGBACKGROUNDVIEWER_H
