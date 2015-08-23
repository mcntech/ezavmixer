#include "settingsgeneral.h"
#include "ui_settingsgeneral.h"
#include "onyxcontrol.h"

#ifdef WIN32
#define ONYX_GUI_FILE_NAME "C:/Projects/onyx/onyx_em/conf/onyx_gui.conf"
#else
#define ONYX_GUI_FILE_NAME "/etc/onyx_gui.conf"
#endif
#define SECTION_GENERAL     "general"

#define KEY_MENU_AUTO_HIDE  "menu_auto_hide"

CSettingsGeneral::CSettingsGeneral(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CSettingsGeneral)
{
    ui->setupUi(this);
    char *szConfFile = ONYX_GUI_FILE_NAME;

    QString MenuAutoHide = GetConfigParam(szConfFile, SECTION_GENERAL, SECTION_GENERAL);
    m_fMenuAutoHide = MenuAutoHide.toInt();
    ui->m_MenuAutoHide->setChecked(m_fMenuAutoHide);

    setWindowFlags(Qt::SplashScreen);

}

CSettingsGeneral::~CSettingsGeneral()
{
    delete ui;
}

void CSettingsGeneral::on_m_MenuAutoHide_clicked()
{

}

void CSettingsGeneral::on_buttonBox_accepted()
{
    char *szConfFile = ONYX_GUI_FILE_NAME;
    if(ui->m_MenuAutoHide->isChecked()) {
        m_fMenuAutoHide = 1;
        SetConfigParam(szConfFile, SECTION_GENERAL, SECTION_GENERAL,"1");
    } else {
        m_fMenuAutoHide = 0;
        SetConfigParam(szConfFile, SECTION_GENERAL, SECTION_GENERAL,"0");
    }
}
