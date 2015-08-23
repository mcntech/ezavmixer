#ifndef WIFISETUP_H
#define WIFISETUP_H

#include <QDialog>
#include "onyxcontrol.h"
#include "keyboard/keyboard.h"


namespace Ui {
class CWifiSetup;
}

class CWifiSetup : public QDialog
{
    Q_OBJECT
    
public:
    explicit CWifiSetup(QWidget *parent = 0);
    ~CWifiSetup();
    
private slots:
    void open_vkbd();
    void on_buttonBox_accepted();
    void scanRequest();
    void updateScanResults();
    int  addNetwork();
    void bssSelected(QTreeWidgetItem *sel);
    void selectNetwork(  );
    void removeNetwork(int nNetworkId);
    void removeAllNetworks();
    void connectNetwork();

private:
    Ui::CWifiSetup *ui;
    Keyboard       *m_pVkbd;

    int ctrlRequest(const char *cmd, char *buf, size_t *buflen);
    int setNetworkParam(int id, const char *field,  const char *value, bool quote);
    void paramsFromScanResults(QTreeWidgetItem *sel);
    void paramsFromConfig(int network_id);

    int GetNetworkList();
    void updateNetworks();

    int m_NetworkId;
};

#endif // WIFISETUP_H
