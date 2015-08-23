#ifndef HLSSERVER_H
#define HLSSERVER_H

#include <QDialog>
#include "keyboard/keyboard.h"

namespace Ui {
class CHlsServer;
}

class CHlsServer : public QDialog
{
    Q_OBJECT

public:
    explicit CHlsServer(QWidget *parent = 0);
    ~CHlsServer();

private slots:
    void on_buttonBox_accepted();
    void open_vkbd();

private:
    Ui::CHlsServer *ui;
    Keyboard        *m_pVkbd;
};

#endif // HLSSERVER_H
