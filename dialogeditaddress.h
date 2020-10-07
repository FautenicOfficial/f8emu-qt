#ifndef DIALOGEDITADDRESS_H
#define DIALOGEDITADDRESS_H

#include <QDialog>

extern int mwMemWatchAddr[256];
extern int numAddrs;
extern int thisIdx2;
extern int thisAddr2;

namespace Ui {
class DialogEditAddress;
}

class DialogEditAddress : public QDialog
{
    Q_OBJECT
public:
    explicit DialogEditAddress(QWidget *parent = nullptr);
    ~DialogEditAddress();
private slots:
    void on_lineEdit_textChanged(const QString &arg1);
private:
    Ui::DialogEditAddress *ui;
};

#endif // DIALOGEDITADDRESS_H
