/*
 * {module name} Matrix Control
 *
 * {module description} Add any top level mode or operation controls to matrix_gui
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
#include <QtGui>
#include "onyxcontrol.h"
#include "minini.h"
#include "Onvif.h"
#include "rtsppublish.h"
#include "rtmppublish.h"
#include "hlspublish.h"
#include "hlsserver.h"
#include "rtspserver.h"
#include "advertisement.h"
#include "selectcamera.h"
#include "activeserverlist.h"
#include "settingsgeneral.h"
#include "wifisetup.h"
#include "osdmanager.h"
#include <time.h>

// Extensions to errors defined in uimsg.h
#define HLS_PUBLISH_CAN_NOT_CONNECT_MW 	0x80001001
//========================================================================================

MatrixControl::MatrixControl(QString DescModeOn)
{
    m_pDescModeOn=DescModeOn;
}

void MatrixControl::SetDescMode(void)
{
    int t;
    QStringList items;
         items << tr("On") << tr("Off");

         bool ok;
         if(m_pDescModeOn == "On")
             t = 0;
         else
             t = 1;
         QString item = QInputDialog::getItem(this, tr("QInputDialog::getItem()"),
                                              tr("Description Mode:"), items, t, false, &ok);
         m_pDescModeOn=item;
}

QString MatrixControl::getDescMode(void)
{
    return(m_pDescModeOn);
}
//========================================================================================

AVSRC_LIST gAvSrcList = {0,{0}};
AVSRC_LIST *gpAvSrcList = &gAvSrcList;

CDetectCamera::CDetectCamera()
{
    m_pOnvifCtx = NULL;
}


void CDetectCamera::doRefresh()
{
    gAvSrcList.no_avsrc = 0;
    doDetection();
}

 void CDetectCamera::doDetection()
 {
#ifdef WIN32
    //err = WSAStartup(wVersionRequested, &wsaData);
    QTcpSocket Socket; // Dummy call to perform equivalent of WSAStartup
    Socket.connectToHost("127.0.0.1", 56777);
#endif
    if(gAvSrcList.no_avsrc == 0) {
         m_pOnvifCtx = GetOnvifDeviceItf()->Open();
         if(m_pOnvifCtx) {
             GetOnvifDeviceItf()->Start(m_pOnvifCtx, onvifCallback, this);
             //emit resultReady(result);
             GetOnvifDeviceItf()->Stop(m_pOnvifCtx);
            getAvsrcList(&gAvSrcList);
            GetOnvifDeviceItf()->Close(m_pOnvifCtx);
            m_pOnvifCtx = NULL;
         }
    }
     emit updateProgress(MAX_AVSRC);
     emit detectComplete(0);

 }

 void CDetectCamera::onvifCallback(void *pClientCtx, int nProgress)
{
     CDetectCamera *pObj = (CDetectCamera *)pClientCtx;
     //pObj->pCameraList = GetOnvifDeviceItf()->GetCameraList(pObj->m_pOnvifCtx);
     emit pObj->updateProgress(nProgress);
}

 int CDetectCamera::getAvsrcList(AVSRC_LIST *pAvsrcList)
 {
     ONVIF_CAMERA_LIST *pOnvifCamList = GetOnvifDeviceItf()->GetCameraList(m_pOnvifCtx);
     pAvsrcList->no_avsrc = 0;
     if(pOnvifCamList){
         pAvsrcList->no_avsrc = pOnvifCamList->no_of_camera_found;
         for(int i=0; i < pOnvifCamList->no_of_camera_found && i < MAX_AVSRC; i++){
             char *szUrl = pAvsrcList->avsrcList[i].url;
             strcpy(szUrl, pOnvifCamList->camera_descript[i].ip_addr);
         }
         return 0;
     }
     return -1;
 }

QString GetAvSrcForInputSession(QString InputSession)
{
    char szAvSrc[256];
    QByteArray ba = InputSession.toLocal8Bit();
    const char *szInputSession = ba.data();
    ini_gets(szInputSession, "input", "",szAvSrc, 256, ONYX_USER_FILE_NAME);
    QString res = szAvSrc;
    return res;
}

void SetAvSrcForInputSession(QString InputSession, QString input)
{
    QByteArray ba1 = InputSession.toLocal8Bit();
    const char *szInputSession = ba1.data();
    QByteArray ba2 = input.toLocal8Bit();
    const char *szInput = ba2.data();
    ini_puts(szInputSession, "input", szInput, ONYX_USER_FILE_NAME);
}

QString GetConfigParam(const char *szConfFile, const char *szSection, const char *szKey)
{
    char szValue[256];
    ini_gets(szSection, szKey, "",szValue, 256, szConfFile);
    QString res = szValue;
    return res;
}

void SetConfigParam(const char *szConfFile, const char *szSection, const char *szKey, QString value)
{
    QByteArray ba1 = value.toLocal8Bit();
    const char *szValue = ba1.data();
    ini_puts(szSection, szKey, szValue, szConfFile);
}


CSelectAvSrc::CSelectAvSrc(QWidget * parent)
    :QWidget(parent)
{
    m_InputSessionId=QString("None");
}

void CSelectAvSrc::SetAvSrcOption(QString Option)
{
    m_InputSessionId = Option;
}

void CSelectAvSrc::setAvSrc(void)
{
    QByteArray ba = m_InputSessionId.toLocal8Bit();
    const char *szInputSession = ba.data();

    QString szStrmType =  GetConfigParam(ONYX_USER_FILE_NAME, szInputSession, KEY_INPUT_TYPE);

    if(szStrmType.compare("rtsp") == 0) {
        QString szIpAddr =  GetConfigParam(ONYX_USER_FILE_NAME, szInputSession, KEY_INPUT_HOST);
        QString szPort =  GetConfigParam(ONYX_USER_FILE_NAME, szInputSession, KEY_INPUT_PORT);
        QString szStream =  GetConfigParam(ONYX_USER_FILE_NAME, szInputSession, KEY_INPUT_STREAM);
        QString  szAudCodec =  GetConfigParam(ONYX_USER_FILE_NAME, szInputSession, KEY_INPUT_STREAM_AUD_CODEC);
        CSelectCamera dialog(this, szIpAddr, szPort, szStream, szAudCodec);

         if (dialog.exec() == QDialog::Accepted){
             SetConfigParam(ONYX_USER_FILE_NAME, szInputSession, KEY_INPUT_HOST, dialog.m_IpAddress);
             SetConfigParam(ONYX_USER_FILE_NAME, szInputSession, KEY_INPUT_PORT, dialog.m_Port);
             SetConfigParam(ONYX_USER_FILE_NAME, szInputSession, KEY_INPUT_STREAM, dialog.m_Stream);
             SetConfigParam(ONYX_USER_FILE_NAME, szInputSession, KEY_INPUT_STREAM_AUD_CODEC, dialog.m_AudioCodec);
         }
    } else {
        QString szFile =  GetConfigParam(ONYX_USER_FILE_NAME, szInputSession, KEY_INPUT_FILE_LOCATION);
        CAdvertisement dialog(this, szFile);

         if (dialog.exec() == QDialog::Accepted){
             SetConfigParam(ONYX_USER_FILE_NAME, szInputSession, KEY_INPUT_FILE_LOCATION, dialog.m_inputAvSrc);
         }
    }
}

COnyxSetup::COnyxSetup(QWidget *pParent):
    QWidget(pParent)
{
    m_pOption=QString("None");
}

void COnyxSetup::SetOption(QString Option)
{
    m_pOption = Option;
}

void COnyxSetup::setup(void)
{
    if(m_pOption == QString("rtsppublish")){
        rtsppublish dialog(this);
        if (dialog.exec() == QDialog::Accepted){
            //TODO:
        }
    } else if(m_pOption == QString("rtmppublish")){
            CRtmpPublish dialog(this);
            if (dialog.exec() == QDialog::Accepted){
                //TODO:
            }

    } else if(m_pOption == QString("hlspublish")){
            CHlsPublish dialog(this);
            if (dialog.exec() == QDialog::Accepted){
                //TODO:
            }

    } else if(m_pOption == QString("hlsserver")){
            CHlsServer dialog(this);
            if (dialog.exec() == QDialog::Accepted){
                //TODO:
            }
    } else if(m_pOption == QString("rtspserver")){
            CRtspServer dialog(this);
            if (dialog.exec() == QDialog::Accepted){
                //TODO:
            }
    } else if(m_pOption == QString("activeservers")){
            CActiveServerList dialog(this);
            if (dialog.exec() == QDialog::Accepted){
                //TODO: Setup Wifi
            }
    } else if(m_pOption == QString("settingsgeneral")){
            CSettingsGeneral dialog(this);
            if (dialog.exec() == QDialog::Accepted){
                if(gpMenuVisibility) {
                    gpMenuVisibility->SetMenuAutoHide(dialog.m_fMenuAutoHide);
                }
            }
    } else if(m_pOption == QString("wifi")){
        CWifiSetup dialog(this);
         if (dialog.exec() == QDialog::Accepted){
             //TODO: Setup Wifi
         }
    }
}

//==========================================
CLaunchOnyx::CLaunchOnyx()
{

}

void CLaunchOnyx::SetOption(QString Option)
{
    m_pOption = Option;
}

void CLaunchOnyx::launch(void)
{
    QProcess process;
    process.startDetached(m_pOption);
}


int COnyxCmd::ReadReply(QTcpSocket *pSocket, char *pReply, int nReplySize)
{
    int nReadSize;
    int bytesRead = 0;
    int res = 0;
    ONYX_MSG_HDR_T Hdr = {0};
   if(pSocket->waitForReadyRead(3000)){
        pSocket->read((char *)&Hdr, sizeof(ONYX_MSG_HDR_T));
        nReadSize = Hdr.ulSize < nReplySize ? Hdr.ulSize : nReplySize;
        if(nReadSize > 0){
            if(pSocket->bytesAvailable() < nReplySize){
                pSocket->waitForReadyRead(3000);
            }
            if(pSocket->bytesAvailable() >= nReplySize){
                bytesRead = pSocket->read(pReply, nReadSize);
            } else {
                res = -1;
            }

        }
   } else {
       res = -1;
   }
    return res;
}

int COnyxCmd::SendCmd(int nCmdId, char *pCmd, int nCmdSize, char *pReply, int nReplySize)
{
    int nReply = 0;
    ONYX_MSG_HDR_T Hdr = {0};
    if(m_pSocket->state() != QAbstractSocket::ConnectedState) {
        m_pSocket->connectToHost("127.0.0.1", ONYX_CMD_PORT);
        if(m_pSocket->waitForConnected(3000) == false) {
            qDebug() << "Cmd:" << nCmdId << "Error Reply=" << nReply;
            nReply = -1;
            goto Exit;
        }
    }

    Hdr.ulCmdId = nCmdId;
    Hdr.ulSize = nCmdSize;
    m_pSocket->write((char *)&Hdr, sizeof(ONYX_MSG_HDR_T));
    if(nCmdSize > 0) {
        m_pSocket->write(pCmd, nCmdSize);
    }
    m_pSocket->flush();
    nReply = ReadReply(m_pSocket, pReply, nReplySize);
Exit:
    return nReply;
}

int COnyxCmd::SendHello()
{
    int nReply = SendCmd(UI_MSG_HELLOW, NULL, 0,  NULL, 0);
    return nReply;
}


int COnyxCmd::SendSelectLayout(int nLayout)
{
    uiSelectLayout_t uiSelectLayout;
    char Reply[256];
    uiSelectLayout.nLayoutId = nLayout;
    int nReply = SendCmd(UI_MSG_SELECT_LAYOUT, (char *)&uiSelectLayout, sizeof(uiSelectLayout),  Reply, 256);
    return 0;
}


CHlsStatusForm::CHlsStatusForm()
{

    const char *szLabelColor = "QLabel { background-color : rgb(255, 0, 255); color : rgb(255, 255, 255); }";
    const char *szValueColor = "QLabel { background-color : rgb(255, 0, 255); color : blue; }";

    QLabel *pLabelStatus = new QLabel("Status:");
    pLabelStatus->setStyleSheet(szLabelColor);
    m_pStatus = new QLabel("Init");
    m_pStatus->setStyleSheet(szValueColor);

    QLabel *pLabelRecorded = new QLabel("Recorded:");
    pLabelRecorded->setStyleSheet(szLabelColor);
    m_pRecorded = new QLabel("0000");
    m_pRecorded->setStyleSheet(szValueColor);

    QLabel *pLabelDropped = new QLabel("Dropped:");
    pLabelDropped->setStyleSheet(szLabelColor);
    m_pDropped = new QLabel("0000");
    m_pDropped->setStyleSheet(szValueColor);

    QLabel *pLabelUploaded = new QLabel("Uploaded:");
    pLabelUploaded->setStyleSheet(szLabelColor);
    m_pUploaded = new QLabel("0000");
    m_pUploaded->setStyleSheet(szValueColor);

    QLabel *pLabelOverRuns = new QLabel("Published:");
    pLabelOverRuns->setStyleSheet(szLabelColor);
    m_pPublished = new QLabel("0000");
    m_pPublished->setStyleSheet(szValueColor);

    addRow(pLabelStatus, m_pStatus);
    addRow(pLabelRecorded, m_pRecorded);
    addRow(pLabelDropped, m_pDropped);
    addRow(pLabelUploaded, m_pUploaded);
    addRow(pLabelOverRuns, m_pPublished);
}

void CHlsStatusForm::doUpdate(int nStaus, int nRecTime, int nLostBufferTime, int nUploadTime, int nPublishTime)
{
    QString szTmp;
    switch(nStaus)
    {
        case HLS_PUBLISH_STATE_UNKNOWN:
            szTmp = "Idle";
            break;


        case HLS_PUBLISH_STOPPED:
            szTmp = "Stopped";
            break;

        case HLS_PUBLISH_CAN_NOT_CONNECT_MW:
            szTmp = "Not Running";
            break;

        case HLS_PUBLISH_RUNNING:
            szTmp = "Running";
            break;
        case HLS_PUBLISH_ERROR_CONNECT_FAIL:
            szTmp = "Failed(01)";
            break;
        case HLS_PUBLISH_ERROR_XFR_FAIL:
            szTmp = "Failed(02)";
            break;
        case HLS_PUBLISH_ERROR_INPUT_FAIL:
            szTmp = "Failed(03)";
            break;
        default:
            szTmp = "Error";
            break;

    }
    m_pStatus->setText(szTmp);

    szTmp = QString::number(nRecTime);
    m_pRecorded->setText(szTmp);

    szTmp = QString::number(nLostBufferTime);
    m_pDropped->setText(szTmp);

    szTmp = QString::number(nUploadTime);
    m_pUploaded->setText(szTmp);

    szTmp = QString::number(nPublishTime);
    m_pPublished->setText(szTmp);
    QApplication::processEvents();
}

#define MAX_TIME_STRING  128

// TODO: Replace with time obtained from onyx_mw
void GetAvailabilityStartTime(time_t time_now, QString &szTime)
{
    struct tm ast_time;
    char availability_start_time[MAX_TIME_STRING];
    ast_time = *gmtime(&time_now);
    strftime(availability_start_time, 64, "%Y-%m-%dT%H:%M:%S", &ast_time);
    szTime = availability_start_time;
}

CSwitchesStatusForm::CSwitchesStatusForm()
{
    int i;
    const char *szLabelColor = "QLabel { background-color : rgb(255, 0, 255); color : rgb(255, 255, 255); font-size:20px;}";
    const char *szValueColor = "QLabel { background-color : rgb(255, 0, 255); color : blue; font-size:20px;}";

    setFormAlignment(Qt::AlignTop);

    QLabel *pLabelStartTime = new QLabel("Start Time:");
    pLabelStartTime->setStyleSheet(szLabelColor);
    m_pStartTime = new QLabel("Not Started");
    m_pStartTime->setStyleSheet(szLabelColor);
    addRow(pLabelStartTime, m_pStartTime);

    QLabel *pLabelInputs = new QLabel("Input Streams:");
    pLabelInputs->setStyleSheet(szLabelColor);
    addRow(pLabelInputs);
    for (i=0; i < SWITCH_STAT_MAX_NUM; i++) {
        char szTmp[32];
        sprintf(szTmp, "Switch%d:", i);
        QLabel *pLabelSwitch = new QLabel(szTmp);
        pLabelSwitch->setStyleSheet(szLabelColor);
        m_pSwitchInput[i] = new QLabel("Init");
        m_pSwitchInput[i]->setStyleSheet(szValueColor);
        addRow(pLabelSwitch, m_pSwitchInput[i]);
    }
    QLabel *pLabelOutputs = new QLabel("Output Streams:");
    pLabelOutputs->setStyleSheet(szLabelColor);
    addRow(pLabelOutputs);
    for (i=0; i < SWITCH_STAT_MAX_NUM; i++) {
        char szTmp[32];
        sprintf(szTmp, "Switch%d:", i);
        QLabel *pLabelSwitch = new QLabel(szTmp);
        pLabelSwitch->setStyleSheet(szLabelColor);
        m_pSwitchOutput[i] = new QLabel("Init");
        m_pSwitchOutput[i]->setStyleSheet(szValueColor);
        addRow(pLabelSwitch, m_pSwitchOutput[i]);
    }

}


void CSwitchesStatusForm::doUpdate(void *_pStats)
{
    QString szTmp;
    switchesStats_t *pswitchesStats = (switchesStats_t *)_pStats;
    GetAvailabilityStartTime((time_t)pswitchesStats->nStartTime, szTmp);
    m_pStartTime->setText(szTmp);
    for (int i=0; i < pswitchesStats->nNumSwitches; i++) {
        switchStats_t *pSwitchStat = &pswitchesStats->switchesStats[i];
        swicthPortStats_t *pInpuStat = &pSwitchStat->inputStats;
        szTmp.sprintf(" V:%04d A:%04d", pInpuStat->nVidStrmTime, pInpuStat->nAudStrmTime);
        m_pSwitchInput[i]->setText(szTmp);
        szTmp.clear();
        for (int j=0; j < pSwitchStat->nNumOutputs; j++) {
            QString szTmpItem;
            swicthPortStats_t *pOutStat = &pSwitchStat->outputStats[j];
            szTmpItem.sprintf("[%s V:%04d A:%04d]", pOutStat->szId, pOutStat->nVidStrmTime, pOutStat->nAudStrmTime);
            szTmp.append(szTmpItem);
        }
        m_pSwitchOutput[i]->setText(szTmp);
    }
    QApplication::processEvents();
}

COnyxCmd::COnyxCmd()
{
    memset(&m_hlspublisgStats, 0x00, sizeof(m_hlspublisgStats));
    memset(&m_switchesStats, 0x00, sizeof(m_switchesStats));
    m_pSocket = new QTcpSocket(this);
    m_fCmdPending = 0;
}

COnyxCmd::~COnyxCmd()
{
    m_pSocket->close();
}

void COnyxCmd::doGetHlsPublishStats()
{
    int nReply = SendCmd(UI_MSG_HLS_PUBLISH_STATS, NULL, 0, (char *)&m_hlspublisgStats, sizeof(hlspublisgStats_t));
    if(nReply != 0){
        //memset(&m_hlspublisgStats, 0x00, sizeof(m_hlspublisgStats));
        m_hlspublisgStats.nState = HLS_PUBLISH_CAN_NOT_CONNECT_MW;
    }
    emit doneGetHlsPublishStats();
}

void COnyxCmd::doGetSwitchesStats()
{
    if(!m_fCmdPending) {
        m_fCmdPending = 1;
        int nReply = SendCmd(UI_MSG_SWITCHES_STATS, NULL, 0, (char *)&m_switchesStats, sizeof(switchesStats_t));
        if(nReply != 0){
            //memset(&m_hlspublisgStats, 0x00, sizeof(m_hlspublisgStats));
            //m_hlspublisgStats.nState = HLS_PUBLISH_CAN_NOT_CONNECT_MW;
        }
        emit doneGetSwitchesStats();
        m_fCmdPending = 0;
    }
}


CSwitchesStatusUpdater::CSwitchesStatusUpdater(QObject *parent) : QTimer(parent)
{
    m_pOnyxCmd = new COnyxCmd;
    m_pThread = new QThread ;
    m_pOnyxCmd->moveToThread(m_pThread);
    m_pThread->start();
    m_timePassed = 0;
    connect(this, SIGNAL(timeout()), m_pOnyxCmd, SLOT(doGetSwitchesStats()));
    connect(m_pOnyxCmd, SIGNAL(doneGetSwitchesStats()), this, SLOT(update()));
}


void CSwitchesStatusUpdater::update()
{
    switchesStats_t *pStats = &m_pOnyxCmd->m_switchesStats;
    emit sendUpdate(pStats);
}

void CHlsStatusUpdater::update()
{
    hlspublisgStats_t *pStats = &m_pOnyxCmd->m_hlspublisgStats;
    emit sendUpdate(pStats->nState, pStats->nStreamInTime, pStats->nLostBufferTime, pStats->nStreamOutTime, pStats->nTotalSegmentTime);
}

CHlsStatusUpdater::CHlsStatusUpdater(QObject *parent) : QTimer(parent)
{
    m_pOnyxCmd = new COnyxCmd;
    m_pThread = new QThread ;
    m_pOnyxCmd->moveToThread(m_pThread);
    m_pThread->start();
    m_timePassed = 0;
    //connect(this, SIGNAL(timeout()), this, SLOT(update()));
    connect(this, SIGNAL(timeout()), m_pOnyxCmd, SLOT(doGetHlsPublishStats()));
    connect(m_pOnyxCmd, SIGNAL(doneGetHlsPublishStats()), this, SLOT(update()));
}

void CSystemSettings::Reboot()
{
    int ret=QMessageBox::No;
    QMessageBox     msgBox;
    msgBox.setStyleSheet("Height: 64px");
    msgBox.setText("Are you sure you want to Restart?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowFlags(Qt::FramelessWindowHint);
    ret=msgBox.exec();

    if(ret==QMessageBox::Yes) {
        QProcess  Process;
        Process.startDetached("reboot");
        QCoreApplication::quit();
    }
}
