#ifndef SETTINGSGENERAL_H
#define SETTINGSGENERAL_H

#include <QDialog>

namespace Ui {
class CSettingsGeneral;
}

class CSettingsGeneral : public QDialog
{
    Q_OBJECT
    
public:
    explicit CSettingsGeneral(QWidget *parent = 0);
    ~CSettingsGeneral();
    int m_fMenuAutoHide;

private slots:
    void on_m_MenuAutoHide_clicked();

    void on_buttonBox_accepted();

private:
    Ui::CSettingsGeneral *ui;
};

#endif // SETTINGSGENERAL_H
