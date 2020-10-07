#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "6502.h"
#include "rtaudio-unix/RtAudio.h"
#include <QMainWindow>
#include <QFileDialog>
#include <QTimer>
#include <QObject>
#include <QSettings>
#include <QtMultimedia/QAudio>
#include "dialoginput.h"
#include "ui_dialoginput.h"
#include "dialogtileviewer.h"
#include "ui_dialogtileviewer.h"
#include "dialogbackgroundviewer.h"
#include "ui_dialogbackgroundviewer.h"
#include "dialoghexeditor.h"
#include "ui_dialoghexeditor.h"
#include "dialogdebugger.h"
#include "ui_dialogdebugger.h"
#include "dialoggameinfo.h"
#include "ui_dialoggameinfo.h"
#include "dialogsetsaveslot.h"
#include "ui_dialogsetsaveslot.h"
#include "dialogmemorywatch.h"
#include "ui_dialogmemorywatch.h"

extern bool isRunning;
extern QSettings * ctrlSettings;
extern QMap<QString, QVariant> ctrlMap;
extern int curPortToCfg;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    DialogTileViewer * dtv;
    DialogBackgroundViewer * dbv;
    DialogHexEditor * dhe;
    DialogDebugger * dd;
    DialogSetSaveSlot * dsss;
    DialogMemoryWatch * dmw;
    explicit MainWindow(QWidget *parent = nullptr,int argc = 0, char *argv[] = {});
    ~MainWindow();
    void closePreviousGame();
    void openNewGame(QString path);
protected:
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
private slots:
    void on_actionOpen_triggered();
    void on_actionClose_triggered();
    void emuLoop();
    void on_actionConfigure_Input_triggered();
    void on_actionReset_triggered();
    void on_actionQuit_triggered();
    void on_actionTile_Viewer_triggered();
    void on_actionBackground_Viewer_triggered();
    void on_actionHex_Editor_triggered();
    void on_actionDebugger_triggered();
    void on_actionGame_Info_triggered();
    void on_actionPause_triggered();
    void on_actionFrame_Advance_triggered();
    void on_actionLoad_State_triggered();
    void on_actionSave_State_triggered();
    void on_actionSet_Save_Slot_triggered();
    void on_actionMemory_Watch_triggered();
    //void on_actionDebug_Console_triggered();
    //void on_actionTimer_triggered();
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
