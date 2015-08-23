#include "rtspserver.h"
#include "ui_rtspserver.h"
#include "onyxcontrol.h"

CRtspServer::CRtspServer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CRtspServer)
{
    m_pVkbd = new Keyboard(this);
    ui->setupUi(this);

    char *szConfFile = ONYX_PUBLISH_FILE_NAME;
    ui->m_StreamName->setText(GetConfigParam(szConfFile, SECTION_RTSP_SERVER, KEY_RTSP_SERVER_STREAM));
    ui->m_Port->setText(GetConfigParam(szConfFile, SECTION_RTSP_SERVER, KEY_RTSP_SERVER_RTSP_PORT));

    setWindowFlags(Qt::SplashScreen);

    connect( ui->m_StreamName,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd()));
    connect( ui->m_Port,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd()));

}

CRtspServer::~CRtspServer()
{
    delete m_pVkbd;
    delete ui;
}

void CRtspServer::on_buttonBox_accepted()
{
    char *szConfFile = ONYX_PUBLISH_FILE_NAME;
    SetConfigParam(szConfFile, SECTION_RTSP_SERVER, KEY_RTSP_SERVER_STREAM,ui->m_StreamName->text());
    SetConfigParam(szConfFile, SECTION_RTSP_SERVER, KEY_RTSP_SERVER_RTSP_PORT,ui->m_Port->text());

}

void CRtspServer::open_vkbd()
{
    QLineEdit *line = (QLineEdit *)sender();
    m_pVkbd->setLineEdit(line);
    m_pVkbd->move(200,360);
    m_pVkbd->raise();
    m_pVkbd->show();
}
