#ifndef DIALOGEDITBREAKPOINT_H
#define DIALOGEDITBREAKPOINT_H

#include <QDialog>

extern int dBreakpointAddr[256];
extern bool dBreakpointE[256];
extern bool dBreakpointX[256];
extern bool dBreakpointR[256];
extern bool dBreakpointW[256];
extern int numBreakpoints;
extern int thisIdx;
extern int thisAddr;
extern bool thisE;
extern bool thisX;
extern bool thisR;
extern bool thisW;

namespace Ui {
class DialogEditBreakpoint;
}

class DialogEditBreakpoint : public QDialog
{
    Q_OBJECT
public:
    explicit DialogEditBreakpoint(QWidget *parent = nullptr);
    ~DialogEditBreakpoint();
private slots:
    void on_checkBox_toggled(bool checked);
    void on_checkBox_2_toggled(bool checked);
    void on_checkBox_3_toggled(bool checked);
    void on_checkBox_4_toggled(bool checked);
    void on_lineEdit_textChanged(const QString &arg1);
private:
    Ui::DialogEditBreakpoint *ui;
};

#endif // DIALOGEDITBREAKPOINT_H
