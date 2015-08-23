/*
 * {module name} OSDManager
 *
 * {module description} OSD manager module
 *
 * Copyright (C) {YEAR} Texas Instruments Incorporated - http://www.ti.com/ 
 * 
 * 
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the   
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/
#ifndef OSDMANAGER_H
#define OSDMANAGER_H

#include <QtGui>
#define MENU_AUDO_HIDE_TIMEOUT      10000

class OSDManager : public QObject
{
    Q_OBJECT

public:
    typedef enum {
        MAX_TRANSPARENCY = 0,  //Very transparent 
        INC_TRANSPARENCY = 0x11, 
        NORMAL_TRANSPARENCY = 0x55, 
        MIN_TRANSPARENCY = 0x77,  //Very opaque
    } Transparency;

    typedef enum {
        MIN_ALPHA = 0,
        INC_ALPHA = 0x11,
        NORMAL_ALPHA = 0x55,
        MAX_ALPHA = 0xFF
    } Alpha;

    OSDManager();
    static void Show(int nShow);
	static void Init();
    
private:
    int             m_fdDisplay;
    int             m_fdAttr;
    char *          m_virtPtrDisplay;
    char *          m_virtPtrAttr;
    bool            m_valid;
    int             m_sizeDisplay;
    int             m_sizeAttr;
};

class CMenuVisibility : public QTimer
{
    long m_timePassed;
    int  m_fVisible;
    int  m_fAutoHide;
    Q_OBJECT

 public:
    explicit CMenuVisibility(QObject *parent = 0) : QTimer(parent)
    {
        m_timePassed = 0;
        m_fVisible = 1;
        m_fAutoHide = 0;
        connect(this, SIGNAL(timeout()), this, SLOT(update()));
    }
    private slots:
    //this slot will be connected with the QTimer timeout() signal.
    //after you start the timer, the timeout signal will be fired every time,
    //when the amount interval() time passed.
    void update()
    {
        if(m_fAutoHide) {
            m_timePassed += interval(); //we increase the time passed
            qDebug()<<m_timePassed; //and debug our collected time
            if(m_timePassed > MENU_AUDO_HIDE_TIMEOUT){
                if(m_fVisible) {
                    m_fVisible = 0;
                    OSDManager::Show(m_fVisible);
                }
            }
        }
    }
public:
    void SetNormal()
    {
        m_timePassed = 0;
        start(1000);
        if(!m_fVisible) {
            m_fVisible = 1;
            OSDManager::Show(m_fVisible);
        }
    }
public slots:
    void SetMenuAutoHide (int fEnable)
    {
        m_fAutoHide = fEnable;
    }
};

class OnyxEventFilter : public QObject
{
Q_OBJECT
protected:
    bool eventFilter(QObject *obj, QEvent *ev)
    {
        if(ev->type() == QEvent::KeyPress ||
           ev->type() == QEvent::MouseMove){
            if(m_pMenuVisibility) {
                m_pMenuVisibility->SetNormal();
            }
        }
        return QObject::eventFilter(obj, ev);
    }
public:
    CMenuVisibility *m_pMenuVisibility;
};

extern CMenuVisibility *gpMenuVisibility;

#endif // OSDMANAGER_H
