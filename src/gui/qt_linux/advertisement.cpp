#include <QFileDialog>
#include "advertisement.h"
#include "ui_advertisement.h"
#include "onyxcontrol.h"

#ifdef WIN32
#define ADVT_FOLDER "c:/teststreams"
#else
#define ADVT_FOLDER "/media/sda1"
#endif

CAdvertisement::CAdvertisement(QWidget *parent, QString inputAvSrc) :
    QDialog(parent),
    ui(new Ui::CAdvertisement)
{

    m_inputAvSrc = inputAvSrc;
    m_pVkbd = new Keyboard(this);
    ui->setupUi(this);
    setWindowFlags(Qt::SplashScreen);
    ui->advt_file->setChecked(true);

    if(m_inputAvSrc.isEmpty() || m_inputAvSrc.compare("disable") == 0) {
        ui->advt_none->setChecked(true);
    } else   {
        if(m_inputAvSrc.contains("http://")) {
            ui->advt_server->setChecked(true);
            ui->lineEdit_AdvtServer->setText(m_inputAvSrc);
        } else {
            ui->advt_file->setChecked(true);
            ui->lineEdit_AdvtFile->setText(m_inputAvSrc);
        }
    }
    connect( ui->buttonOkCancel, SIGNAL(accepted()), this, SLOT(verify()));
    connect( ui->lineEdit_AdvtServer,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd()));
}

CAdvertisement::~CAdvertisement()
{
    delete ui;
}

void CAdvertisement::on_file_browse_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Advertisement File"),ADVT_FOLDER, tr("Files (*.ts)"));
    if(!fileName.isEmpty()){
        ui->lineEdit_AdvtFile->setText(fileName);
    }
}

void CAdvertisement::on_advt_none_toggled(bool checked)
{
    if(checked){
        ui->lineEdit_AdvtFile->setDisabled(true);
        ui->lineEdit_AdvtServer->setDisabled(true);
    }
}

void CAdvertisement::on_advt_file_toggled(bool checked)
{
    if(checked){
        ui->lineEdit_AdvtFile->setEnabled(true);
        ui->lineEdit_AdvtServer->setDisabled(true);
    }
}

void CAdvertisement::on_advt_server_toggled(bool checked)
{
    if(checked){
        ui->lineEdit_AdvtFile->setDisabled(true);
        ui->lineEdit_AdvtServer->setEnabled(true);
    }
}

void CAdvertisement::open_vkbd()
{
    QLineEdit *line = (QLineEdit *)sender();
    m_pVkbd->setLineEdit(line);
    m_pVkbd->move(200,200);
    m_pVkbd->raise();
    m_pVkbd->show();
}

void CAdvertisement::verify()
{
    accept();
    if(ui->advt_none->isChecked()){
        m_inputAvSrc = "disable";
    } else if(ui->advt_file->isChecked()){
        m_inputAvSrc = ui->lineEdit_AdvtFile->text();
    } else if(ui->advt_server->isChecked()){
        m_inputAvSrc = ui->lineEdit_AdvtServer->text();
    }
}

