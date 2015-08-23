/*
 * {module name} MatrixControl
 *
 * {module description} Create an object for top level matrix control 
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

#ifndef ONYXCONTROL_H
#define ONYXCONTROL_H

#include <QList>
#include <QtWebKit>
#include <QtGui>

#include <QDialog>
#include <QList>
#include <QPair>
#include <QString>
#include <QComboBox>
#include <QProgressBar>
#include <stdio.h>
#include <stdlib.h>

#include "../onyx_mw/uimsg.h"
#ifdef WIN32
#define DISP_WIDTH    1600
#define DISP_HEIGHT   1080
#else
#define DISP_WIDTH    1920
#define DISP_HEIGHT   1080
#endif
 class QCheckBox;
 class QDialogButtonBox;
 class QLabel;
 class QLineEdit;
 class QTableWidget;
 class QTextEdit;
 class QWidget;

class MatrixControl : public QWidget
{
    Q_OBJECT

public:
    MatrixControl(QString DescModeOn);

public slots:
    void SetDescMode(void);
    QString getDescMode(void);

signals:
        void SetMenuAutoHide(int fMenuAutoHide);

private:
    QString            m_pDescModeOn;
};

//===========================================================

#define MAX_URL_LEN	(256)
#define MAX_AVSRC    16

typedef struct _AVSRC_URL {
    char url[MAX_URL_LEN];
} AVSRC_URL;

typedef struct _AVSRC_LIST{
    int no_avsrc;
    AVSRC_URL avsrcList[MAX_AVSRC];
} AVSRC_LIST;

// Wrapper for ONVIF DEvice Discovery
class CDetectCamera : public QObject
 {
     Q_OBJECT

public:
    CDetectCamera();
public slots:
     void doDetection();
     void doRefresh();

signals:
     void updateProgress(int nPercent);
     void detectComplete(int nError);

public:
     static void onvifCallback(void *pClientCtx, int nProgress);

public:
    void   *m_pOnvifCtx;
    int getAvsrcList(AVSRC_LIST *pAvsrcList);
    static AVSRC_LIST    m_AvSrcList;
 };


class CSelectAvSrc : public QWidget
{
    Q_OBJECT

public:
    CSelectAvSrc(QWidget * parent );
    void SetAvSrcOption(QString Option);
public slots:
    void setAvSrc(void);

public:
    QString            m_InputSessionId;
};


class COnyxSetup : public QWidget
{
    Q_OBJECT

public:
    COnyxSetup(QWidget *pParent);
    void SetOption(QString Option);
public slots:
    void setup(void);

public:
    QString            m_pOption;
};

class CLaunchOnyx : public QWidget
{
    Q_OBJECT

public:
    CLaunchOnyx();
    void SetOption(QString Option);
public slots:
    void launch(void);

public:
    QString            m_pOption;
};

class COnyxCmd : public QObject
{
    Q_OBJECT

public:
    COnyxCmd();
    ~COnyxCmd();
    int SendHello();
    int SendSelectLayout(int nLayout);

    int SendCmd(int nCmdId, char *pCmd, int nCmdSize, char *pReply, int nReplySize);
    int ReadReply(QTcpSocket *Socket, char *pReply, int nReplySize);

public slots:
    void doGetHlsPublishStats();
    void doGetSwitchesStats();

signals:
    void doneGetHlsPublishStats();
    void doneGetSwitchesStats();

public:
    hlspublisgStats_t m_hlspublisgStats;
    switchesStats_t    m_switchesStats;
    QTcpSocket        *m_pSocket;
    int               m_fCmdPending;
};

class CSystemSettings
{
public:
    static void Reboot();
};

class CHlsStatusForm : public QFormLayout
{
    Q_OBJECT
public:
    CHlsStatusForm();
    QLabel *m_pStatus;
    QLabel *m_pRecorded;
    QLabel *m_pDropped;
    QLabel *m_pUploaded;
    QLabel *m_pPublished;

public slots:
    void doUpdate(int nStaus, int nRecTime, int nLostBufferTime, int nUploadTime, int nPublishTime);
};


class CHlsStatusUpdater : public QTimer
{
    Q_OBJECT
    long m_timePassed;

public:
    explicit CHlsStatusUpdater(QObject *parent = 0);

public slots:
     void update();

signals:
     void sendUpdate(int nStaus, int nRecTime, int nLostTime, int nUploadTime, int nPublishTime);

public:
     COnyxCmd *m_pOnyxCmd;
     QThread  *m_pThread;
};

class CSwitchesStatusForm : public QFormLayout
{
    Q_OBJECT
public:
    CSwitchesStatusForm();
    QLabel *m_pSwitchInput[SWITCH_STAT_MAX_NUM];
    QLabel *m_pSwitchOutput[SWITCH_STAT_MAX_NUM];
    QLabel *m_pStartTime;

public slots:
    void doUpdate(void */*switchesStats_t *pswitchesStats*/);
};

class CSwitchesStatusUpdater : public QTimer
{
    Q_OBJECT
    long m_timePassed;

public:
    explicit CSwitchesStatusUpdater(QObject *parent = 0);

public slots:
     void update();

signals:
     void sendUpdate(void *pStats);

public:
     COnyxCmd *m_pOnyxCmd;
     QThread  *m_pThread;
};

QString GetConfigParam(const char *szConfFile, const char *szSection, const char *szKey);
void SetConfigParam(const char *szConfFile, const char *szSection, const char *szKey, QString value);
QString GetAvSrcForInputSession(QString InputSession);
void SetAvSrcForInputSession(QString InputSession, QString input);


extern AVSRC_LIST *gpAvSrcList;

#endif // ONYXCONTROL_H

