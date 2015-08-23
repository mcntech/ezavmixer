#ifndef HLSPUBLISH_H
#define HLSPUBLISH_H

#include <QDialog>
#include "keyboard/keyboard.h"

namespace Ui {
class CHlsPublish;
}

class CHlsPublish : public QDialog
{
    Q_OBJECT
    
public:
    explicit CHlsPublish(QWidget *parent = 0);
    ~CHlsPublish();

    void EnableHttpStorageParams(int fEnable);
private slots:
    void on_buttonBox_accepted();

    void on_m_ProtoHttp_toggled(bool checked);

    void on_m_ProtoFile_toggled(bool checked);
    void open_vkbd();
    void on_m_LoadFromFile_clicked();

private:
    Ui::CHlsPublish *ui;
    Keyboard        *m_pVkbd;
};

#endif // HLSPUBLISH_H
