#include "hlsserver.h"
#include "ui_hlsserver.h"
#include "onyxcontrol.h"

CHlsServer::CHlsServer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CHlsServer)
{
    QString ParamValue;
    int    nParamValue;
    m_pVkbd = new Keyboard(this);
    ui->setupUi(this);

    char *szConfFile = ONYX_PUBLISH_FILE_NAME;
    ui->m_FolderName->setText(GetConfigParam(szConfFile,SECTION_HLS_SERVER,KEY_HLS_FOLDER));
    ui->m_StreamName->setText(GetConfigParam(szConfFile,SECTION_HLS_SERVER, KEY_HLS_STREAM));

    ui->m_Port->setText(GetConfigParam(szConfFile,SECTION_HLS_SERVER,KEY_HLS_SRVR_PORT));
    ui->m_ServerRoot->setText(GetConfigParam(szConfFile,SECTION_HLS_SERVER, KEY_HLS_SRVR_ROOT));

    ui->m_SegmentDuration->setMaximum(20);
    ui->m_SegmentDuration->setMinimum(1);
    ParamValue = GetConfigParam(szConfFile,SECTION_HLS_SERVER, KEY_HLS_SEGMENT_DURATION);
    nParamValue = ParamValue.toInt() / 1000;
    ui->m_SegmentDuration->setValue(nParamValue);

    connect( ui->m_FolderName,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd()));
    connect( ui->m_StreamName,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd()));
    connect( ui->m_SegmentDuration,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd()));
    connect( ui->m_Port,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd()));
    connect( ui->m_ServerRoot,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd()));


    setWindowFlags(Qt::SplashScreen);
}

CHlsServer::~CHlsServer()
{
    delete m_pVkbd;
    delete ui;
}

void CHlsServer::on_buttonBox_accepted()
{
    char *szConfFile = ONYX_PUBLISH_FILE_NAME;
    QString ParamValue;
    int    nParamValue;

    SetConfigParam(szConfFile, SECTION_HLS_SERVER, KEY_HLS_FOLDER,ui->m_FolderName->text());
    SetConfigParam(szConfFile, SECTION_HLS_SERVER, KEY_HLS_STREAM,ui->m_StreamName->text());

    nParamValue = ui->m_SegmentDuration->value() * 1000;
    ParamValue = QString::number(nParamValue);
    SetConfigParam(szConfFile, SECTION_HLS_SERVER, KEY_HLS_SEGMENT_DURATION,ParamValue);

    SetConfigParam(szConfFile, SECTION_HLS_SERVER, KEY_HLS_SRVR_PORT,ui->m_Port->text());
    SetConfigParam(szConfFile, SECTION_HLS_SERVER, KEY_HLS_SRVR_ROOT,ui->m_ServerRoot->text());
}

void CHlsServer::open_vkbd()
{
    QLineEdit *line = (QLineEdit *)sender();
    m_pVkbd->setLineEdit(line);
    m_pVkbd->move(200,360);
    m_pVkbd->raise();
    m_pVkbd->show();
}
