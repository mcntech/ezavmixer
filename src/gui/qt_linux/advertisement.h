#ifndef ADVERTISEMENT_H
#define ADVERTISEMENT_H

#include <QDialog>
#include "keyboard/keyboard.h"


namespace Ui {
class CAdvertisement;
}

class CAdvertisement : public QDialog
{
    Q_OBJECT
    
public:
    explicit CAdvertisement(QWidget *parent, QString avSrc);
    ~CAdvertisement();


private slots:
    void on_file_browse_clicked();

    void on_advt_none_toggled(bool checked);

    void on_advt_file_toggled(bool checked);

    void on_advt_server_toggled(bool checked);

    void open_vkbd();

public slots:
    void verify();

private:
    Ui::CAdvertisement *ui;
    Keyboard        *m_pVkbd;

public:
    QString          m_inputAvSrc;
};

#endif // ADVERTISEMENT_H
