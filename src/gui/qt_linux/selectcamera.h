#ifndef SELECTCAMERA_H
#define SELECTCAMERA_H

#include <QDialog>
#include "onyxcontrol.h"
#include "keyboard/keyboard.h"

namespace Ui {
class CSelectCamera;
}


class CSelectCamera : public QDialog
{
    Q_OBJECT
    
public:
    explicit CSelectCamera(QWidget *parent, QString szIpAddr, QString szPort,QString szStream, QString  szAudCodec);
    ~CSelectCamera();

public slots:
    void verify();
    void updateOnvifCameralist(int nPercent);
    void open_vkbd();

signals:
    void startOnvifDetection();

private:
    CDetectCamera   *m_pDetectCamera;
    QThread       *m_pCamDetectThread;

private:
    Ui::CSelectCamera *ui;
    Keyboard          *m_pVkbd;
public:
    QString          m_IpAddress;
    QString          m_Port;
    QString          m_Stream;
    QString          m_AudioCodec;

};

#endif // SELECTCAMERA_H
