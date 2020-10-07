#include "dialogbackgroundviewer.h"
#include "ui_dialogbackgroundviewer.h"

unsigned char bvPixels[512*512*4];
QLabel * bvLabelMap;
QTimer * bvLoopTimer;

DialogBackgroundViewer::DialogBackgroundViewer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogBackgroundViewer)
{
    ui->setupUi(this);
    ui->label->installEventFilter(this);
    bvLabelMap = ui->label;

    bvLoopTimer = new QTimer;
    connect(bvLoopTimer,SIGNAL(timeout()),this,SLOT(bvDrawLoop()));
    bvLoopTimer->start(16);
}
void bvPutPixel(int color,int xd,int yd)
{
    xd &= 511;
    yd &= 511;
    int pix = ((xd+(yd<<9))<<2);
    bvPixels[pix+2] = 85*((color&0x30)>>4);
    bvPixels[pix+1] = 85*((color&0x0C)>>2);
    bvPixels[pix] = 85*(color&0x03);
}
void bvPutPixelFromTile(int tile,int pal,int xs,int ys,int xd,int yd)
{
    int addr = 0x4000+(tile<<4)+ys;
    int entry0 = (gameImage[addr]>>(7-xs))&1;
    int entry1 = (gameImage[addr+8]>>(7-xs))&1;
    int entry = entry0+(entry1<<1);
    if(entry!=0)
    {
        int color = gameImage[0x7500+(pal<<2)+entry];
        bvPutPixel(color,xd,yd);
    }
}
void DialogBackgroundViewer::bvDrawLoop() {
    int bgColor = gameImage[0x7500];
    for(int i=0; i<512; i++) {
        for(int j=0; j<512; j++) {
            bvPutPixel(bgColor,i,j);
        }
    }
    for(int x=0; x<512; x++) {
        for(int y=0; y<512; y++) {
            int etile = ((x>>4)&15)+(((y>>4)&15)<<4)+(x&0x100)+((y&0x100)<<1);
            int estile = ((x>>3)&1)+(((y>>3)&1)<<1);
            int mask = 0xC0>>(estile<<1);
            int shift = 6-(estile<<1);
            int xs = x&7;
            int ys = y&7;
            int px = ((x&0xFF)>>3)+((x&0x100)<<2);
            int py = ((y&0xF8)<<2)+((y&0x100)<<3);
            bvPutPixelFromTile(gameImage[0x6000+px+py],
                    (gameImage[0x7000+etile]&mask)>>shift,
                    xs,ys,x,y);
        }
    }
    int splx = gameImage[0x7400];
    int sply = (gameImage[0x7401]<192)?gameImage[0x7401]:192;
    if(gameImage[0x7424]&0x08)
    {
        int scrlx = gameImage[0x7408]+((gameImage[0x7404]&0x80)<<1);
        int scrly = gameImage[0x7409]+((gameImage[0x7404]&0x40)<<2);
        int scrlw = scrlx+splx;
        int scrlh = scrly+sply;
        for(int i=scrlx; i<scrlw; i++) {
            bvPutPixel(0x20,i,scrly);
            bvPutPixel(0x20,i,scrlh);
        }
        for(int i=scrly; i<scrlh; i++) {
            bvPutPixel(0x20,scrlx,i);
            bvPutPixel(0x20,scrlw,i);
        }
    }
    if(gameImage[0x7424]&0x04)
    {
        int scrlx = gameImage[0x740A]+((gameImage[0x7404]&0x20)<<3);
        int scrly = gameImage[0x740B]+((gameImage[0x7404]&0x10)<<4);
        int scrlw = scrlx+256;
        scrlx += splx;
        int scrlh = scrly+sply;
        for(int i=scrlx; i<scrlw; i++) {
            bvPutPixel(0x22,i,scrly);
            bvPutPixel(0x22,i,scrlh);
        }
        for(int i=scrly; i<scrlh; i++) {
            bvPutPixel(0x22,scrlx,i);
            bvPutPixel(0x22,scrlw,i);
        }
    }
    if(gameImage[0x7424]&0x02)
    {
        int scrlx = gameImage[0x740C]+((gameImage[0x7404]&0x08)<<5);
        int scrly = gameImage[0x740D]+((gameImage[0x7404]&0x04)<<6);
        int scrlw = scrlx+splx;
        int scrlh = scrly+192;
        scrly += sply;
        for(int i=scrlx; i<scrlw; i++) {
            bvPutPixel(0x08,i,scrly);
            bvPutPixel(0x08,i,scrlh);
        }
        for(int i=scrly; i<scrlh; i++) {
            bvPutPixel(0x08,scrlx,i);
            bvPutPixel(0x08,scrlw,i);
        }
    }
    if(gameImage[0x7424]&0x01)
    {
        int scrlx = gameImage[0x740E]+((gameImage[0x7404]&0x02)<<7);
        int scrly = gameImage[0x740F]+((gameImage[0x7404]&0x01)<<8);
        int scrlw = scrlx+256;
        scrlx += splx;
        int scrlh = scrly+192;
        scrly += sply;
        for(int i=scrlx; i<scrlw; i++) {
            bvPutPixel(0x02,i,scrly);
            bvPutPixel(0x02,i,scrlh);
        }
        for(int i=scrly; i<scrlh; i++) {
            bvPutPixel(0x02,scrlx,i);
            bvPutPixel(0x02,scrlw,i);
        }
    }

    QImage imageMap(bvPixels,512,512,QImage::Format_RGB32);
    QPixmap pixelMap = QPixmap::fromImage(imageMap);
    pixelMap = pixelMap.scaled(bvLabelMap->width(),bvLabelMap->height());
    bvLabelMap->setPixmap(pixelMap);
}
DialogBackgroundViewer::~DialogBackgroundViewer()
{
    bvLoopTimer->stop();
    delete ui;
}
