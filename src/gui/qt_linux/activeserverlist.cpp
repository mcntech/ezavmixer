#include "activeserverlist.h"
#include "ui_activeserverlist.h"
#include "onyxcontrol.h"

CActiveServerList::CActiveServerList(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CActiveServerList)
{
    char *szConfFile = ONYX_PUBLISH_FILE_NAME;

    ui->setupUi(this);

    QString Protocol = GetConfigParam(szConfFile, SECTION_SERVERS, EN_RTSP_SERVER);
    ui->m_RtspServer->setChecked(Protocol.toInt());
    Protocol = GetConfigParam(szConfFile, SECTION_SERVERS, EN_RTSP_PUBLISH);
    ui->m_RtspPublish->setChecked(Protocol.toInt());
    Protocol = GetConfigParam(szConfFile, SECTION_SERVERS, EN_HLS_SERVER);
    ui->m_HlsServer->setChecked(Protocol.toInt());
    Protocol = GetConfigParam(szConfFile, SECTION_SERVERS, EN_HLS_PUBLISH);
    ui->m_HlsPublish->setChecked(Protocol.toInt());
    Protocol = GetConfigParam(szConfFile, SECTION_SERVERS, EN_RTMP_PUBLISH);
    ui->m_RtmpPublish->setChecked(Protocol.toInt());

    setWindowFlags(Qt::SplashScreen);
}

CActiveServerList::~CActiveServerList()
{
    delete ui;
}

void CActiveServerList::on_buttonBox_accepted()
{
    char *szConfFile = ONYX_PUBLISH_FILE_NAME;
    QString Protocol;

    if(ui->m_RtspServer->isChecked())  Protocol = "1"; else Protocol = "0";
    SetConfigParam(szConfFile, SECTION_SERVERS, EN_RTSP_SERVER, Protocol);

    if(ui->m_RtspPublish->isChecked())  Protocol = "1"; else Protocol = "0";
    SetConfigParam(szConfFile, SECTION_SERVERS, EN_RTSP_PUBLISH, Protocol);

    if(ui->m_HlsServer->isChecked())  Protocol = "1"; else Protocol = "0";
    SetConfigParam(szConfFile, SECTION_SERVERS, EN_HLS_SERVER, Protocol);

    if(ui->m_HlsPublish->isChecked())  Protocol = "1"; else Protocol = "0";
    SetConfigParam(szConfFile, SECTION_SERVERS, EN_HLS_PUBLISH, Protocol);

    if(ui->m_RtmpPublish->isChecked())  Protocol = "1"; else Protocol = "0";
    SetConfigParam(szConfFile, SECTION_SERVERS, EN_RTMP_PUBLISH, Protocol);

}
