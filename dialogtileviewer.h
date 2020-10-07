#ifndef DIALOGTILEVIEWER_H
#define DIALOGTILEVIEWER_H

#include "6502.h"
#include <QDialog>
#include <QLabel>
#include <QImage>
#include <QPixmap>
#include <QTimer>

extern unsigned char tvPixelsB[128*128*4];
extern unsigned char tvPixelsS[128*128*4];
extern unsigned char tvPixelsP[16*2*4];
extern QLabel * tvLabelMapB;
extern QLabel * tvLabelMapS;
extern QLabel * tvLabelMapP;
extern QTimer * tvLoopTimer;
extern int curPalB, curPalS;

namespace Ui {
class DialogTileViewer;
}

class DialogTileViewer : public QDialog
{
    Q_OBJECT
public:
    explicit DialogTileViewer(QWidget *parent = nullptr);
    ~DialogTileViewer();
private slots:
    void tvPutPixel(int color,int xd,int yd,unsigned char * pixels);
    void tvPutPixelFromTile(int tile,int pal,int xs,int ys,int xd,int yd,unsigned char * pixels);
    void tvDrawLoop();
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
private:
    Ui::DialogTileViewer *ui;
};

#endif // DIALOGTILEVIEWER_H
