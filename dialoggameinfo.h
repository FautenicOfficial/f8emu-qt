#ifndef DIALOGGAMEINFO_H
#define DIALOGGAMEINFO_H

#include "6502.h"
#include <QDialog>
#include <QLabel>

extern unsigned char giPixels[128*128*4];

void giPutPixel(int color,int xd,int yd);
void giPutPixelFromTile(int tile,int pal,int xs,int ys,int xd,int yd);

namespace Ui {
class DialogGameInfo;
}

class DialogGameInfo : public QDialog
{
    Q_OBJECT
public:
    explicit DialogGameInfo(QWidget *parent = nullptr);
    ~DialogGameInfo();
private:
    Ui::DialogGameInfo *ui;
};

#endif // DIALOGGAMEINFO_H
