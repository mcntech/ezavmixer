#ifndef RTMPPUBLISH_H
#define RTMPPUBLISH_H

#include <QDialog>
#include "keyboard/keyboard.h"

namespace Ui {
class CRtmpPublish;
}

class CRtmpPublish : public QDialog
{
    Q_OBJECT
    
public:
    explicit CRtmpPublish(QWidget *parent = 0);
    ~CRtmpPublish();
    
private slots:
    void on_buttonBox_accepted();

private:
    Ui::CRtmpPublish *ui;
    Keyboard        *m_pVkbd;
};

#endif // RTMPPUBLISH_H
