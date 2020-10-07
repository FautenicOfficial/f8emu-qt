#include "dialoggameinfo.h"
#include "ui_dialoggameinfo.h"

unsigned char giPixels[128*128*4];

DialogGameInfo::DialogGameInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogGameInfo)
{
    ui->setupUi(this);

    QString str = "Short Name: ";
    for(int i=4; i<8; i++) {
        str.append(gameHeader[i]);
    }
    ui->label_2->setText(str);
    str.clear();
    str.append("Long Name: ");
    int offset=64;
    while(true) {
        char x = static_cast<char>(gameHeader[offset++]);
        if(x==0) {break;}
        str.append(x);
    }
    ui->label_3->setText(str);

    int bgColor = gameHeader[0x20];
    for(int x=0; x<128; x++) {
        for(int y=0; y<128; y++) {
            giPutPixel(bgColor,x,y);
        }
    }
    for(int x=0; x<128; x++) {
        for(int y=0; y<128; y++) {
            int etile = ((x>>4)&15)+(((y>>4)&15)<<3);
            int estile = ((x>>3)&1)+(((y>>3)&1)<<1);
            int mask = 0xC0>>(estile<<1);
            int shift = 6-(estile<<1);
            int xs = x&7;
            int ys = y&7;
            giPutPixelFromTile((x>>3)|((y>>3)<<4),
                    (gameHeader[0xFC0+etile]&mask)>>shift,
                    xs,ys,x,y);
        }
    }
    QImage imageMap(giPixels,128,128,QImage::Format_RGB32);
    QPixmap pixelMap = QPixmap::fromImage(imageMap);
    pixelMap = pixelMap.scaled(ui->label->width(),ui->label->height());
    ui->label->setPixmap(pixelMap);
}
void giPutPixel(int color,int xd,int yd)
{
    int pix = ((xd+(yd<<7))<<2);
    giPixels[pix+2] = 85*((color&0x30)>>4);
    giPixels[pix+1] = 85*((color&0x0C)>>2);
    giPixels[pix] = 85*(color&0x03);
}
void giPutPixelFromTile(int tile,int pal,int xs,int ys,int xd,int yd)
{
    int addr = 0x1000+(tile<<4)+ys;
    int entry0 = (gameHeader[addr]>>(7-xs))&1;
    int entry1 = (gameHeader[addr+8]>>(7-xs))&1;
    int entry = entry0+(entry1<<1);
    if(entry!=0)
    {
        int color = gameHeader[0x20+(pal<<2)+entry];
        giPutPixel(color,xd,yd);
    }
}
DialogGameInfo::~DialogGameInfo()
{
    delete ui;
}
