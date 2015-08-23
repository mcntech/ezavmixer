#include "selectcamera.h"
#include "ui_selectcamera.h"

CSelectCamera::CSelectCamera(QWidget *parent, QString szIpAddress, QString  szPort, QString  szStream, QString  szAudCodec) :
    QDialog(parent),
    ui(new Ui::CSelectCamera)
{
    m_pDetectCamera = new CDetectCamera();
    m_pCamDetectThread = new QThread;

    m_pDetectCamera->moveToThread(m_pCamDetectThread);
    m_pCamDetectThread->start();
    m_IpAddress = szIpAddress;
    m_Port = szPort;
    m_Stream = szStream;
    m_AudioCodec = szAudCodec;
    m_pVkbd = new Keyboard(this);
    ui->setupUi(this);

    ui->m_Port->setText(m_Port);
    ui->m_Stream->setText(m_Stream);

    ui->m_AudioCodec->addItem(AUDIO_CODEC_NONE);
    ui->m_AudioCodec->addItem(AUDIO_CODEC_G711U);

    int nCurrent = ui->m_AudioCodec->findText(m_AudioCodec);
    if(nCurrent != -1){
        ui->m_AudioCodec->setCurrentIndex(nCurrent);
    }


    ui->m_progress->setMaximum(MAX_AVSRC);

    connect(m_pDetectCamera, SIGNAL(detectComplete(int)),
                    this, SLOT(updateOnvifCameralist(int)));
    connect(ui->m_refresh, SIGNAL(clicked()),
                    m_pDetectCamera, SLOT(doRefresh()));
    connect(m_pDetectCamera, SIGNAL(updateProgress(int)),
                    ui->m_progress, SLOT(setValue(int)));

    connect(this, SIGNAL(startOnvifDetection()),
                    m_pDetectCamera, SLOT(doDetection()));

    connect( ui->m_Port,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd()));
    connect( ui->m_Stream,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd()));

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(verify()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    setWindowFlags(Qt::SplashScreen);
    emit startOnvifDetection();
}

CSelectCamera::~CSelectCamera()
{
    delete ui;
}

void CSelectCamera::updateOnvifCameralist(int nPercent)
{
    AVSRC_LIST *pAvsrcList = gpAvSrcList;
    ui->m_CameraList->clear();

    for(int i=0; i < pAvsrcList->no_avsrc; i++){
        ui->m_CameraList->addItem(pAvsrcList->avsrcList[i].url);
    }
    ui->m_CameraList->addItem("disable");
    int nCurrent = ui->m_CameraList->findText(m_IpAddress);
    if(nCurrent != -1){
        ui->m_CameraList->setCurrentIndex(nCurrent);
    }
}

void CSelectCamera::verify()
{
    m_IpAddress = ui->m_CameraList->currentText();
    m_Port = ui->m_Port->text();
    m_Stream = ui->m_Stream->text();
    m_AudioCodec = ui->m_AudioCodec->currentText();
    accept();
}

void CSelectCamera::open_vkbd()
{
    QLineEdit *line = (QLineEdit *)sender();
    m_pVkbd->setLineEdit(line);
    m_pVkbd->move(200,200);
    m_pVkbd->raise();
    m_pVkbd->show();
}
