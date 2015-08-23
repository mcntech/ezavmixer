#ifndef RTSPPUBLISH_H
#define RTSPPUBLISH_H

#include <QDialog>
#include "keyboard/keyboard.h"

namespace Ui {
class rtsppublish;
}

class rtsppublish : public QDialog
{
    Q_OBJECT

public:
    explicit rtsppublish(QWidget *parent = 0);
    ~rtsppublish();
private slots:
    void open_vkbd_server_address();

public slots:
    void verify();

private:
    Ui::rtsppublish *ui;
    Keyboard        *m_pVkbd;
};

#endif // RTSPPUBLISH_H
