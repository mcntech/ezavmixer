#include "rtmppublish.h"
#include "ui_rtmppublish.h"
#include "onyxcontrol.h"

CRtmpPublish::CRtmpPublish(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CRtmpPublish)
{
    char *szConfFile = ONYX_PUBLISH_FILE_NAME;
    m_pVkbd = new Keyboard(this);
    ui->setupUi(this);

    QString PrimaryServerEnable = GetConfigParam(szConfFile, SECTION_RTMP_PUBLISH, KEY_RTMP_PUBLISH_PRIMARY_ENABLE);
    ui->m_PrimaryEnable->setChecked(PrimaryServerEnable.toInt());

    QString SecondaryServerEnable = GetConfigParam(szConfFile, SECTION_RTMP_PUBLISH, KEY_RTMP_PUBLISH_SECONDARY_ENABLE);
    ui->m_SecondaryEnable->setChecked(SecondaryServerEnable.toInt());

    ui->m_PrimaryServer->setText(GetConfigParam(szConfFile, SECTION_RTMP_PUBLISH, KEY_RTMP_PUBLISH_PRIMARY_HOST));
    ui->m_PrimaryPort->setText(GetConfigParam(szConfFile,SECTION_RTMP_PUBLISH,KEY_RTMP_PUBLISH_PRIMARY_PORT));
    ui->m_PrimaryApplication->setText(GetConfigParam(szConfFile,SECTION_RTMP_PUBLISH, KEY_RTMP_PUBLISH_PRIMARY_APP));
    ui->m_PrimaryStream->setText(GetConfigParam(szConfFile,SECTION_RTMP_PUBLISH, KEY_RTMP_PUBLISH_PRIMARY_STRM));
    ui->m_PrimaryUserId->setText(GetConfigParam(szConfFile,SECTION_RTMP_PUBLISH, KEY_RTMP_PUBLISH_PRIMARY_USER));
    ui->m_PrimaryPassword->setText(GetConfigParam(szConfFile,SECTION_RTMP_PUBLISH, KEY_RTMP_PUBLISH_PRIMARY_USER));

    ui->m_SecondaryServer->setText(GetConfigParam(szConfFile, SECTION_RTMP_PUBLISH, KEY_RTMP_PUBLISH_SECONDARY_HOST));
    ui->m_SecondaryPort->setText(GetConfigParam(szConfFile,SECTION_RTMP_PUBLISH,KEY_RTMP_PUBLISH_SECONDARY_PORT));
    ui->m_SecondaryApplication->setText(GetConfigParam(szConfFile,SECTION_RTMP_PUBLISH, KEY_RTMP_PUBLISH_SECONDARY_APP));
    ui->m_SecondaryStream->setText(GetConfigParam(szConfFile,SECTION_RTMP_PUBLISH, KEY_RTMP_PUBLISH_SECONDARY_STRM));
    ui->m_SecondaryUserId->setText(GetConfigParam(szConfFile,SECTION_RTMP_PUBLISH, KEY_RTMP_PUBLISH_SECONDARY_USER));
    ui->m_SecondaryPassword->setText(GetConfigParam(szConfFile,SECTION_RTMP_PUBLISH, KEY_RTMP_PUBLISH_SECONDARY_PASSWD));

    connect( ui->buttonBox, SIGNAL(accepted()), this, SLOT(verify()));

    connect( ui->m_PrimaryServer,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd_server_address()));
    connect( ui->m_PrimaryPort,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd_server_address()));
    connect( ui->m_PrimaryApplication,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd_server_address()));
    connect( ui->m_PrimaryStream,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd_server_address()));
    connect( ui->m_SecondaryUserId,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd_server_address()));
    connect( ui->m_PrimaryPassword,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd_server_address()));

    connect( ui->m_SecondaryServer,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd_server_address()));
    connect( ui->m_PrimaryPort,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd_server_address()));
    connect( ui->m_SecondaryApplication,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd_server_address()));
    connect( ui->m_SecondaryStream,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd_server_address()));
    connect( ui->m_SecondaryUserId,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd_server_address()));
    connect( ui->m_SecondaryPassword,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd_server_address()));


    setWindowFlags(Qt::SplashScreen);
}

CRtmpPublish::~CRtmpPublish()
{
    delete m_pVkbd;
    delete ui;
}

void CRtmpPublish::on_buttonBox_accepted()
{
    char *szConfFile = ONYX_PUBLISH_FILE_NAME;
    accept();
    SetConfigParam(szConfFile, SECTION_RTMP_PUBLISH, KEY_RTMP_PUBLISH_PRIMARY_HOST,ui->m_PrimaryServer->text());
    SetConfigParam(szConfFile,SECTION_RTMP_PUBLISH,KEY_RTMP_PUBLISH_PRIMARY_PORT,ui->m_PrimaryPort->text());
    SetConfigParam(szConfFile,SECTION_RTMP_PUBLISH, KEY_RTMP_PUBLISH_PRIMARY_APP,ui->m_PrimaryApplication->text());
    SetConfigParam(szConfFile,SECTION_RTMP_PUBLISH, KEY_RTMP_PUBLISH_PRIMARY_STRM,ui->m_PrimaryStream->text());
    SetConfigParam(szConfFile,SECTION_RTMP_PUBLISH, KEY_RTMP_PUBLISH_PRIMARY_USER,ui->m_PrimaryUserId->text());
    SetConfigParam(szConfFile,SECTION_RTMP_PUBLISH, KEY_RTSP_PUBLISH_PASSWD,ui->m_PrimaryPassword->text());

    if(ui->m_PrimaryEnable->isChecked()) {
        SetConfigParam(szConfFile, SECTION_RTMP_PUBLISH, KEY_RTMP_PUBLISH_PRIMARY_ENABLE,"1");
    } else {
        SetConfigParam(szConfFile, SECTION_RTMP_PUBLISH, KEY_RTMP_PUBLISH_PRIMARY_ENABLE,"0");
    }

    SetConfigParam(szConfFile, SECTION_RTMP_PUBLISH, KEY_RTMP_PUBLISH_SECONDARY_HOST,ui->m_SecondaryServer->text());
    SetConfigParam(szConfFile,SECTION_RTMP_PUBLISH,KEY_RTMP_PUBLISH_SECONDARY_PORT,ui->m_SecondaryPort->text());
    SetConfigParam(szConfFile,SECTION_RTMP_PUBLISH, KEY_RTMP_PUBLISH_SECONDARY_APP,ui->m_SecondaryApplication->text());
    SetConfigParam(szConfFile,SECTION_RTMP_PUBLISH, KEY_RTMP_PUBLISH_SECONDARY_STRM,ui->m_SecondaryStream->text());
    SetConfigParam(szConfFile,SECTION_RTMP_PUBLISH, KEY_RTMP_PUBLISH_SECONDARY_USER,ui->m_SecondaryUserId->text());
    SetConfigParam(szConfFile,SECTION_RTMP_PUBLISH, KEY_RTMP_PUBLISH_SECONDARY_PASSWD,ui->m_SecondaryPassword->text());

    if(ui->m_SecondaryEnable->isChecked()) {
        SetConfigParam(szConfFile, SECTION_RTMP_PUBLISH, KEY_RTMP_PUBLISH_SECONDARY_ENABLE,"1");
    } else {
        SetConfigParam(szConfFile, SECTION_RTMP_PUBLISH, KEY_RTMP_PUBLISH_SECONDARY_ENABLE,"0");
    }


}
