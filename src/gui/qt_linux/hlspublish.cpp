#include "hlspublish.h"
#include "ui_hlspublish.h"
#include "onyxcontrol.h"

CHlsPublish::CHlsPublish(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CHlsPublish)
{
    QString ParamValue;
    int    nParamValue;
    m_pVkbd = new Keyboard(this);
    ui->setupUi(this);


    char *szConfFile = ONYX_PUBLISH_FILE_NAME;
    QString Protocol = GetConfigParam(szConfFile, SECTION_HLS_PUBLISH, KEY_HLS_PROTOCOL);
    ui->m_FolderName->setText(GetConfigParam(szConfFile,SECTION_HLS_PUBLISH,KEY_HLS_FOLDER));
    ui->m_StreamName->setText(GetConfigParam(szConfFile,SECTION_HLS_PUBLISH, KEY_HLS_STREAM));

    ui->m_SegmentDuration->setMaximum(20);
    ui->m_SegmentDuration->setMinimum(1);
    ParamValue = GetConfigParam(szConfFile,SECTION_HLS_PUBLISH, KEY_HLS_SEGMENT_DURATION);
    nParamValue = ParamValue.toInt() / 1000;
    ui->m_SegmentDuration->setValue(nParamValue);


    ui->m_HostAndBucket->setText(GetConfigParam(szConfFile,SECTION_HLS_PUBLISH, KEY_HLS_HTTP_SERVER));
    ui->m_S3Bucket->setText(GetConfigParam(szConfFile,SECTION_HLS_PUBLISH, KEY_HLS_S3_BUCKET));
    ui->m_AccessId->setText(GetConfigParam(szConfFile,SECTION_HLS_PUBLISH, KEY_HLS_ACCESS_ID));
    ui->m_SecurityKey->setText(GetConfigParam(szConfFile,SECTION_HLS_PUBLISH, KEY_HLS_SECURITY_KEY));

    if(Protocol.compare(HLS_PROTOCOL_HTTP) == 0) {
        ui->m_ProtoHttp->setChecked(1);
        ui->m_ProtoFile->setChecked(0);
        EnableHttpStorageParams(1);
    } else {
        EnableHttpStorageParams(0);
        ui->m_ProtoHttp->setChecked(0);
        ui->m_ProtoFile->setChecked(1);
    }
    setWindowFlags(Qt::SplashScreen);


    connect( ui->m_FolderName,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd()));
    connect( ui->m_SegmentDuration,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd()));
    connect( ui->m_HostAndBucket,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd()));
    connect( ui->m_S3Bucket,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd()));
    connect( ui->m_AccessId,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd()));
    connect( ui->m_SecurityKey,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd()));

}
void CHlsPublish::EnableHttpStorageParams(int fEnable)
{
    ui->m_HostAndBucket->setEnabled(fEnable);
    ui->m_AccessId->setEnabled(fEnable);
    ui->m_SecurityKey->setEnabled(fEnable);
    ui->m_S3Bucket->setEnabled(fEnable);
}

CHlsPublish::~CHlsPublish()
{
    delete m_pVkbd;
    delete ui;
}

void CHlsPublish::on_buttonBox_accepted()
{
    char *szConfFile = ONYX_PUBLISH_FILE_NAME;
    QString ParamValue;
    int    nParamValue;

    SetConfigParam(szConfFile, SECTION_HLS_PUBLISH, KEY_HLS_FOLDER,ui->m_FolderName->text());
    SetConfigParam(szConfFile, SECTION_HLS_PUBLISH, KEY_HLS_STREAM,ui->m_StreamName->text());
    //SetConfigParam(szConfFile, SECTION_HLS_PUBLISH, KEY_HLS_SEGMENT_DURATION,ui->m_SegmentDuration->text());
    nParamValue = ui->m_SegmentDuration->value() * 1000;
    ParamValue = QString::number(nParamValue);
    SetConfigParam(szConfFile, SECTION_HLS_PUBLISH, KEY_HLS_SEGMENT_DURATION,ParamValue);

    SetConfigParam(szConfFile, SECTION_HLS_PUBLISH, KEY_HLS_HTTP_SERVER,ui->m_HostAndBucket->text());
    SetConfigParam(szConfFile, SECTION_HLS_PUBLISH, KEY_HLS_S3_BUCKET,ui->m_S3Bucket->text());

    SetConfigParam(szConfFile, SECTION_HLS_PUBLISH, KEY_HLS_ACCESS_ID,ui->m_AccessId->text());
    SetConfigParam(szConfFile, SECTION_HLS_PUBLISH, KEY_HLS_SECURITY_KEY,ui->m_SecurityKey->text());

    if(ui->m_ProtoHttp->isChecked()) {
        SetConfigParam(szConfFile, SECTION_HLS_PUBLISH, KEY_HLS_PROTOCOL,HLS_PROTOCOL_HTTP);
    } else {
        SetConfigParam(szConfFile, SECTION_HLS_PUBLISH, KEY_HLS_PROTOCOL,HLS_PROTOCOL_FILE);
    }

}

void CHlsPublish::on_m_ProtoHttp_toggled(bool checked)
{
    if(checked)
        EnableHttpStorageParams(1);
}

void CHlsPublish::on_m_ProtoFile_toggled(bool checked)
{
    if(checked)
        EnableHttpStorageParams(0);
}

void CHlsPublish::open_vkbd()
{
    QLineEdit *line = (QLineEdit *)sender();
    m_pVkbd->setLineEdit(line);
    m_pVkbd->move(200,360);
    m_pVkbd->raise();
    m_pVkbd->show();
}

#ifdef WIN32
#define KEY_FILE_FOLDER "c:/teststreams"
#else
#define KEY_FILE_FOLDER "/media/"
#endif
#define KEY_ACCESS_ID   "accessKeyId"
#define KEY_SECRET_KEY  "secretKey"

QString GetAwsKeyParam(const char *szConfFile, const char *szSection, const char *szKey);
void CHlsPublish::on_m_LoadFromFile_clicked()
{
    const int maxSize = 256;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Key File"),KEY_FILE_FOLDER, tr("Files (*.*)"));
    if(!fileName.isEmpty()){
        FILE *pFile = fopen(fileName.toStdString().c_str(), "rb");
        if(pFile != NULL){
            char szLine[maxSize];
            while(fgets(szLine, maxSize, pFile) != NULL){
                if(strncmp(szLine, KEY_ACCESS_ID, strlen(KEY_ACCESS_ID)) == 0){
                    QString Tmp;
                    char szAccesId[maxSize];
                    strcpy(szAccesId, szLine + strlen(KEY_ACCESS_ID) + 1);
                    Tmp = szAccesId;
                    Tmp = Tmp.trimmed();
                    ui->m_AccessId->setText(Tmp);
                } else if(strncmp(szLine, KEY_SECRET_KEY, strlen(KEY_SECRET_KEY)) == 0){
                    QString Tmp;
                    char szSecretKey[maxSize];
                    strcpy(szSecretKey, szLine + strlen(KEY_SECRET_KEY) + 1);
                    Tmp = szSecretKey;
                    Tmp = Tmp.trimmed();
                    ui->m_SecurityKey->setText(Tmp);
                }
            }
        }
    }
}
