#ifndef ACTIVESERVERLIST_H
#define ACTIVESERVERLIST_H

#include <QDialog>

namespace Ui {
class CActiveServerList;
}

class CActiveServerList : public QDialog
{
    Q_OBJECT

public:
    explicit CActiveServerList(QWidget *parent = 0);
    ~CActiveServerList();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::CActiveServerList *ui;
};

#endif // ACTIVESERVERLIST_H
