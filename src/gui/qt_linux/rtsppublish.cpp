#include "rtsppublish.h"
#include "ui_rtsppublish.h"
#include "onyxcontrol.h"

rtsppublish::rtsppublish(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::rtsppublish)
{
    m_pVkbd = new Keyboard(this);
    ui->setupUi(this);

    char *szConfFile = ONYX_PUBLISH_FILE_NAME;
    ui->server_address->setText(GetConfigParam(szConfFile, SECTION_RTSP_PUBLISH, KEY_RTSP_PUBLISH_HOST));
    ui->server_port->setText(GetConfigParam(szConfFile,SECTION_RTSP_PUBLISH,KEY_RTSP_PUBLISH_RTSP_PORT));
    ui->application_name->setText(GetConfigParam(szConfFile,SECTION_RTSP_PUBLISH, KEY_RTSP_PUBLISH_APPLICATION));
    ui->user->setText(GetConfigParam(szConfFile,SECTION_RTSP_PUBLISH, KEY_RTSP_PUBLISH_USER));
    ui->password->setText(GetConfigParam(szConfFile,SECTION_RTSP_PUBLISH, KEY_RTSP_PUBLISH_PASSWD));

    connect( ui->buttonBox, SIGNAL(accepted()), this, SLOT(verify()));

    connect( ui->server_address,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd_server_address()));
    connect( ui->server_port,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd_server_address()));
    connect( ui->user,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd_server_address()));
    connect( ui->password,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd_server_address()));
    connect( ui->application_name,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd_server_address()));
    setWindowFlags(Qt::SplashScreen);
}

void rtsppublish::verify()
{
    char *szConfFile = ONYX_PUBLISH_FILE_NAME;

    accept();
    SetConfigParam(szConfFile, SECTION_RTSP_PUBLISH, KEY_RTSP_PUBLISH_HOST,ui->server_address->text());
    SetConfigParam(szConfFile,SECTION_RTSP_PUBLISH,KEY_RTSP_PUBLISH_RTSP_PORT,ui->server_port->text());
    SetConfigParam(szConfFile,SECTION_RTSP_PUBLISH, KEY_RTSP_PUBLISH_APPLICATION,ui->application_name->text());
    SetConfigParam(szConfFile,SECTION_RTSP_PUBLISH, KEY_RTSP_PUBLISH_USER,ui->user->text());
    SetConfigParam(szConfFile,SECTION_RTSP_PUBLISH, KEY_RTSP_PUBLISH_PASSWD,ui->password->text());

}

rtsppublish::~rtsppublish()
{
    delete ui;
}

void rtsppublish::open_vkbd_server_address()
{
    QLineEdit *line = (QLineEdit *)sender();
    m_pVkbd->setLineEdit(line);
    m_pVkbd->move(200,200);
    m_pVkbd->raise();
    m_pVkbd->show();
}
