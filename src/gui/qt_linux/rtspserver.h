#ifndef RTSPSERVER_H
#define RTSPSERVER_H

#include <QDialog>
#include "keyboard/keyboard.h"

namespace Ui {
class CRtspServer;
}

class CRtspServer : public QDialog
{
    Q_OBJECT
    
public:
    explicit CRtspServer(QWidget *parent = 0);
    ~CRtspServer();


private slots:
    void on_buttonBox_accepted();
    void open_vkbd();

private:
    Ui::CRtspServer *ui;
    Keyboard        *m_pVkbd;
};

#endif // RTSPSERVER_H
