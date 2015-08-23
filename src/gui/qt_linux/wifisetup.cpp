#include "wifisetup.h"
#include "ui_wifisetup.h"

#ifdef WIN32
#define snprintf sprintf_s
#endif
#include "wpa_ctrl.h"

enum {
    AUTH_NONE_OPEN,
    AUTH_NONE_WEP,
    AUTH_NONE_WEP_SHARED,
    AUTH_IEEE8021X,
    AUTH_WPA_PSK,
    AUTH_WPA_EAP,
    AUTH_WPA2_PSK,
    AUTH_WPA2_EAP
};

#define WPA_GUI_KEY_DATA "[key is configured]"

char *wpas_ctrl_path = "/var/run/wpa_supplicant/wlan0";
struct wpa_ctrl *pWpaCtrl = NULL;


int CWifiSetup::ctrlRequest(const char *cmd, char *buf, size_t *buflen)
{
    int ret;

    if (pWpaCtrl == NULL)
        return -3;
    ret = wpa_ctrl_request(pWpaCtrl, cmd, strlen(cmd), buf, buflen, NULL);
    if (ret == -2)
        qDebug() << cmd << "command timed out.\n";
    else if (ret < 0)
        qDebug() << cmd << " : failed.\n";

    return ret;
}

void CWifiSetup::scanRequest()
{
    char reply[10];
    size_t reply_len = sizeof(reply);

    qDebug() << "Scan Request.\n";
    if (pWpaCtrl == NULL){
        return;
    }

    ctrlRequest("SCAN", reply, &reply_len);
    QTimer::singleShot(2000, this, SLOT(updateScanResults()));
}

int CWifiSetup::setNetworkParam(int id, const char *field,
                   const char *value, bool quote)
{
    char reply[10], cmd[256];
    size_t reply_len;
    snprintf(cmd, sizeof(cmd), "SET_NETWORK %d %s %s%s%s",
         id, field, quote ? "\"" : "", value, quote ? "\"" : "");
    reply_len = sizeof(reply);
    ctrlRequest(cmd, reply, &reply_len);
    return strncmp(reply, "OK", 2) == 0 ? 0 : -1;
}


int CWifiSetup::addNetwork()
{
    char reply[10], cmd[256];
    size_t reply_len;
    int id;
    int psklen = ui->m_Password->text().length();
    int auth = ui->m_AuthSelect->currentIndex();

    if (auth == AUTH_WPA_PSK || auth == AUTH_WPA2_PSK) {
        if (psklen < 8 || psklen > 64) {
            QMessageBox::warning(
                this,
                tr("WPA Pre-Shared Key Error"),
                tr("WPA-PSK requires a passphrase of 8 to 63 "
                   "characters\n"
                   "or 64 hex digit PSK"));
            ui->m_Password->setFocus();
            return -1;
        }
    }
#if 0
    if (idstrEdit->isEnabled() && !idstrEdit->text().isEmpty()) {
        QRegExp rx("^(\\w|-)+$");
        if (rx.indexIn(idstrEdit->text()) < 0) {
            QMessageBox::warning(
                this, tr("Network ID Error"),
                tr("Network ID String contains non-word "
                   "characters.\n"
                   "It must be a simple string, "
                   "without spaces, containing\n"
                   "only characters in this range: "
                   "[A-Za-z0-9_-]\n"));
            idstrEdit->setFocus();
            return;
        }
    }
#endif

    memset(reply, 0, sizeof(reply));
    reply_len = sizeof(reply) - 1;

    ctrlRequest("ADD_NETWORK", reply, &reply_len);
    if (reply[0] == 'F') {
        QMessageBox::warning(this, "onyx_gui",
                     tr("Failed to add "
                    "network to wpa_supplicant\n"
                    "configuration."));
        return -1;
    }
    m_NetworkId = id = atoi(reply);


    setNetworkParam(id, "ssid", ui->m_Ssid->text().toAscii().constData(), true);

    const char *key_mgmt = NULL, *proto = NULL, *pairwise = NULL;
    switch (auth) {
    case AUTH_NONE_OPEN:
    case AUTH_NONE_WEP:
    case AUTH_NONE_WEP_SHARED:
        key_mgmt = "NONE";
        break;
    case AUTH_IEEE8021X:
        key_mgmt = "IEEE8021X";
        break;
    case AUTH_WPA_PSK:
        key_mgmt = "WPA-PSK";
        proto = "WPA";
        break;
    case AUTH_WPA_EAP:
        key_mgmt = "WPA-EAP";
        proto = "WPA";
        break;
    case AUTH_WPA2_PSK:
        key_mgmt = "WPA-PSK";
        proto = "WPA2";
        break;
    case AUTH_WPA2_EAP:
        key_mgmt = "WPA-EAP";
        proto = "WPA2";
        break;
    }

    if (auth == AUTH_NONE_WEP_SHARED)
        setNetworkParam(id, "auth_alg", "SHARED", false);
    else
        setNetworkParam(id, "auth_alg", "OPEN", false);

    if (auth == AUTH_WPA_PSK || auth == AUTH_WPA_EAP ||
        auth == AUTH_WPA2_PSK || auth == AUTH_WPA2_EAP)  {
#if 0
        int encr = encrSelect->currentIndex();
        if (encr == 0)
            pairwise = "TKIP";
        else
            pairwise = "CCMP";
#endif
    }



    if (proto)
        setNetworkParam(id, "proto", proto, false);
    if (key_mgmt)
        setNetworkParam(id, "key_mgmt", key_mgmt, false);
    if (pairwise) {
        setNetworkParam(id, "pairwise", pairwise, false);
        setNetworkParam(id, "group", "TKIP CCMP WEP104 WEP40", false);
    }

    if (ui->m_Password->isEnabled() &&
        strcmp(ui->m_Password->text().toAscii().constData(),
           WPA_GUI_KEY_DATA) != 0)
        setNetworkParam(id, "psk",
                ui->m_Password->text().toAscii().constData(),
                psklen != 64);
#if 0
    if (eapSelect->isEnabled()) {
        const char *eap =
            eapSelect->currentText().toAscii().constData();
        setNetworkParam(id, "eap", eap, false);
        if (strcmp(eap, "SIM") == 0 || strcmp(eap, "AKA") == 0)
            setNetworkParam(id, "pcsc", "", true);
        else
            setNetworkParam(id, "pcsc", "NULL", false);
    }
#endif

#if 0
    if (phase2Select->isEnabled()) {
        QString eap = eapSelect->currentText();
        QString inner = phase2Select->currentText();
        char phase2[32];
        phase2[0] = '\0';
        if (eap.compare("PEAP") == 0) {
            if (inner.startsWith("EAP-"))
                snprintf(phase2, sizeof(phase2), "auth=%s",
                     inner.right(inner.size() - 4).
                     toAscii().constData());
        } else if (eap.compare("TTLS") == 0) {
            if (inner.startsWith("EAP-"))
                snprintf(phase2, sizeof(phase2), "autheap=%s",
                     inner.right(inner.size() - 4).
                     toAscii().constData());
            else
                snprintf(phase2, sizeof(phase2), "auth=%s",
                     inner.toAscii().constData());
        } else if (eap.compare("FAST") == 0) {
            const char *provisioning = NULL;
            if (inner.startsWith("EAP-")) {
                snprintf(phase2, sizeof(phase2), "auth=%s",
                     inner.right(inner.size() - 4).
                     toAscii().constData());
                provisioning = "fast_provisioning=2";
            } else if (inner.compare("GTC(auth) + MSCHAPv2(prov)")
                   == 0) {
                snprintf(phase2, sizeof(phase2),
                     "auth=GTC auth=MSCHAPV2");
                provisioning = "fast_provisioning=1";
            } else
                provisioning = "fast_provisioning=3";
            if (provisioning) {
                char blob[32];
                setNetworkParam(id, "phase1", provisioning,
                        true);
                snprintf(blob, sizeof(blob),
                     "blob://fast-pac-%d", id);
                setNetworkParam(id, "pac_file", blob, true);
            }
        }
        if (phase2[0])
            setNetworkParam(id, "phase2", phase2, true);
        else
            setNetworkParam(id, "phase2", "NULL", false);
    } else
        setNetworkParam(id, "phase2", "NULL", false);
    if (identityEdit->isEnabled() && identityEdit->text().length() > 0)
        setNetworkParam(id, "identity",
                identityEdit->text().toAscii().constData(),
                true);
    else
        setNetworkParam(id, "identity", "NULL", false);
    if (passwordEdit->isEnabled() && passwordEdit->text().length() > 0 &&
        strcmp(passwordEdit->text().toAscii().constData(),
           WPA_GUI_KEY_DATA) != 0)
        setNetworkParam(id, "password",
                passwordEdit->text().toAscii().constData(),
                true);
    else if (passwordEdit->text().length() == 0)
        setNetworkParam(id, "password", "NULL", false);
    if (cacertEdit->isEnabled() && cacertEdit->text().length() > 0)
        setNetworkParam(id, "ca_cert",
                cacertEdit->text().toAscii().constData(),
                true);
    else
        setNetworkParam(id, "ca_cert", "NULL", false);
    writeWepKey(id, wep0Edit, 0);
    writeWepKey(id, wep1Edit, 1);
    writeWepKey(id, wep2Edit, 2);
    writeWepKey(id, wep3Edit, 3);

    if (wep0Radio->isEnabled() && wep0Radio->isChecked())
        setNetworkParam(id, "wep_tx_keyidx", "0", false);
    else if (wep1Radio->isEnabled() && wep1Radio->isChecked())
        setNetworkParam(id, "wep_tx_keyidx", "1", false);
    else if (wep2Radio->isEnabled() && wep2Radio->isChecked())
        setNetworkParam(id, "wep_tx_keyidx", "2", false);
    else if (wep3Radio->isEnabled() && wep3Radio->isChecked())
        setNetworkParam(id, "wep_tx_keyidx", "3", false);

    if (idstrEdit->isEnabled() && idstrEdit->text().length() > 0)
        setNetworkParam(id, "id_str",
                idstrEdit->text().toAscii().constData(),
                true);
    else
        setNetworkParam(id, "id_str", "NULL", false);

    if (prioritySpinBox->isEnabled()) {
        QString prio;
        prio = prio.setNum(prioritySpinBox->value());
        setNetworkParam(id, "priority", prio.toAscii().constData(),
                false);
    }
#endif
    snprintf(cmd, sizeof(cmd), "ENABLE_NETWORK %d", id);
    reply_len = sizeof(reply);
    ctrlRequest(cmd, reply, &reply_len);
    if (strncmp(reply, "OK", 2) != 0) {
        QMessageBox::warning(this, "wpa_gui",
                     tr("Failed to enable "
                    "network in wpa_supplicant\n"
                    "configuration."));
        /* Network was added, so continue anyway */
    }
    //wpagui->triggerUpdate();
    ctrlRequest("SAVE_CONFIG", reply, &reply_len);

    //close();
    return 0;
}


void CWifiSetup::updateScanResults()
{
    char reply[2048];
    size_t reply_len;
    int index;
    char cmd[20];

    ui->m_Networks->clear();

    index = 0;
    qDebug() << "ScanResults.\n";
    while (1/*wpagui*/) {
        sprintf(cmd,  "BSS %d", index++);
        if (index > 1000)
            break;

        reply_len = sizeof(reply) - 1;
        if (ctrlRequest(cmd, reply, &reply_len) < 0)
            break;
        reply[reply_len] = '\0';

        QString bss(reply);
        if (bss.isEmpty() || bss.startsWith("FAIL"))
            break;

        QString ssid, bssid, freq, signal, flags;

        QStringList lines = bss.split(QRegExp("\\n"));
        for (QStringList::Iterator it = lines.begin();
             it != lines.end(); it++) {
            int pos = (*it).indexOf('=') + 1;
            if (pos < 1)
                continue;

            if ((*it).startsWith("bssid="))
                bssid = (*it).mid(pos);
            else if ((*it).startsWith("freq="))
                freq = (*it).mid(pos);
            else if ((*it).startsWith("level="))
                signal = (*it).mid(pos);
            else if ((*it).startsWith("flags="))
                flags = (*it).mid(pos);
            else if ((*it).startsWith("ssid="))
                ssid = (*it).mid(pos);
        }

        QTreeWidgetItem *item = new QTreeWidgetItem(ui->m_Networks);
        if (item) {
            item->setText(0, ssid);
            item->setText(1, bssid);
            item->setText(2, freq);
            item->setText(3, signal);
            item->setText(4, flags);
        }

        if (bssid.isEmpty())
            break;
    }
}

void CWifiSetup::updateNetworks()
{
    char buf[2048], *start, *end, *id, *ssid, *bssid, *flags;
    size_t len;
    int first_active = -1;
    int was_selected = -1;
    bool current = false;

    if(pWpaCtrl == NULL) {
        goto Exit;
    }
#if 0
    if (!networkMayHaveChanged)
        return;

    if (networkList->currentRow() >= 0)
        was_selected = networkList->currentRow();

    networkSelect->clear();
    networkList->clear();

    if (ctrl_conn == NULL)
        return;
#endif
    len = sizeof(buf) - 1;
    if (wpa_ctrl_request(pWpaCtrl, "LIST_NETWORKS", strlen("LIST_NETWORKS"), buf, &len, NULL) < 0){
        goto Exit;
    }

    buf[len] = '\0';
    start = strchr(buf, '\n');
    if (start == NULL)
        return;
    start++;

    while (*start) {
        bool last = false;
        end = strchr(start, '\n');
        if (end == NULL) {
            last = true;
            end = start;
            while (end[0] && end[1])
                end++;
        }
        *end = '\0';

        id = start;
        ssid = strchr(id, '\t');
        if (ssid == NULL)
            break;
        *ssid++ = '\0';
        bssid = strchr(ssid, '\t');
        if (bssid == NULL)
            break;
        *bssid++ = '\0';
        flags = strchr(bssid, '\t');
        if (flags == NULL)
            break;
        *flags++ = '\0';
#if 0
        if (strstr(flags, "[DISABLED][P2P-PERSISTENT]")) {
            if (last)
                break;
            start = end + 1;
            continue;
        }
#endif
        QString network(id);
        network.append(": ");
        network.append(ssid);

        //ui->m_Networks->addItem(network);
#if 0
        if (strstr(flags, "[CURRENT]")) {
            networkSelect->setCurrentIndex(networkSelect->count() -
                              1);
            current = true;
        } else if (first_active < 0 &&
               strstr(flags, "[DISABLED]") == NULL)
            first_active = networkSelect->count() - 1;
#endif
        if (last)
            break;
        start = end + 1;
    }
#if 0
    if (networkSelect->count() > 1)
        networkSelect->addItem(tr("Select any network"));

    if (!current && first_active >= 0)
        networkSelect->setCurrentIndex(first_active);

    if (was_selected >= 0 && networkList->count() > 0) {
        if (was_selected < networkList->count())
            networkList->setCurrentRow(was_selected);
        else
            networkList->setCurrentRow(networkList->count() - 1);
    }
    else
        networkList->setCurrentRow(networkSelect->currentIndex());

    networkMayHaveChanged = false;
#endif
Exit:
    return;
}

int CWifiSetup::GetNetworkList()
{
    char buf[2048], *start, *end, *pos;
    size_t len;

    if(pWpaCtrl) {
        //  populate the list
        if(wpa_ctrl_request(pWpaCtrl, "STATUS", strlen("STATUS"), buf, &len, NULL) < 0){
            ui->m_Status->setText(tr("Could not get status from wpa_supplicant"));
        } else {
            buf[len] = '\0';

            bool auth_updated = false, ssid_updated = false;
            bool bssid_updated = false, ipaddr_updated = false;
            bool status_updated = false;
            char *pairwise_cipher = NULL, *group_cipher = NULL;
            char *mode = NULL;

            start = buf;
            while (*start) {
                bool last = false;
                end = strchr(start, '\n');
                if (end == NULL) {
                    last = true;
                    end = start;
                    while (end[0] && end[1])
                        end++;
                }
                *end = '\0';

                pos = strchr(start, '=');
                if (pos) {
                    *pos++ = '\0';
                    if (strcmp(start, "bssid") == 0) {
                        bssid_updated = true;
                        //textBssid->setText(pos);
                    } else if (strcmp(start, "ssid") == 0) {
                        ssid_updated = true;
                        ui->m_Ssid->setText(pos);
                        //textSsid->setText(pos);
                    } else if (strcmp(start, "ip_address") == 0) {
                        ipaddr_updated = true;
                        //textIpAddress->setText(pos);
                    } else if (strcmp(start, "wpa_state") == 0) {
                        status_updated = true;
                        //textStatus->setText(wpaStateTranslate(pos));
                    } else if (strcmp(start, "key_mgmt") == 0) {
                        auth_updated = true;
                        //textAuthentication->setText(pos);
                        /* TODO: could add EAP status to this */
                    } else if (strcmp(start, "pairwise_cipher") == 0) {
                        pairwise_cipher = pos;
                    } else if (strcmp(start, "group_cipher") == 0) {
                        group_cipher = pos;
                    } else if (strcmp(start, "mode") == 0) {
                        mode = pos;
                    }
                }

                if (last)
                    break;
                start = end + 1;
            }
            ui->m_Status->setText(tr("Scanning OK"));
        }
    } else {
        ui->m_Status->setText(tr("Could not Open Interface"));
    }
    return 0;
}

void CWifiSetup::paramsFromScanResults(QTreeWidgetItem *sel)
{
    /* SSID BSSID frequency signal flags */
    ui->m_Ssid->setText(sel->text(0));

    QString flags = sel->text(4);
    int auth, encr = 0;
    if (flags.indexOf("[WPA2-EAP") >= 0)
        auth = AUTH_WPA2_EAP;
    else if (flags.indexOf("[WPA-EAP") >= 0)
        auth = AUTH_WPA_EAP;
    else if (flags.indexOf("[WPA2-PSK") >= 0)
        auth = AUTH_WPA2_PSK;
    else if (flags.indexOf("[WPA-PSK") >= 0)
        auth = AUTH_WPA_PSK;
    else
        auth = AUTH_NONE_OPEN;

    if (flags.indexOf("-CCMP") >= 0)
        encr = 1;
    else if (flags.indexOf("-TKIP") >= 0)
        encr = 0;
    else if (flags.indexOf("WEP") >= 0) {
        encr = 1;
        if (auth == AUTH_NONE_OPEN)
            auth = AUTH_NONE_WEP;
    } else
        encr = 0;

    ui->m_AuthSelect->setCurrentIndex(auth);

#if 0
    authChanged(auth);
    encrSelect->setCurrentIndex(encr);

    wepEnabled(auth == AUTH_NONE_WEP);

    getEapCapa();

    if (flags.indexOf("[WPS") >= 0)
        useWpsButton->setEnabled(true);
    bssid = sel->text(1);
#endif
}

void CWifiSetup::bssSelected(QTreeWidgetItem *sel)
{
    paramsFromScanResults(sel);
}

void CWifiSetup::paramsFromConfig(int network_id)
{
    int i, res;

    m_NetworkId = network_id;

    char reply[1024], cmd[256], *pos;
    size_t reply_len;

    sprintf(cmd,  "GET_NETWORK %d ssid", network_id);
    reply_len = sizeof(reply) - 1;
    if (ctrlRequest(cmd, reply, &reply_len) >= 0 &&
        reply_len >= 2 && reply[0] == '"') {
        reply[reply_len] = '\0';
        pos = strchr(reply + 1, '"');
        if (pos)
            *pos = '\0';
        ui->m_Ssid->setText(reply + 1);
    }

    snprintf(cmd, sizeof(cmd), "GET_NETWORK %d proto", network_id);
    reply_len = sizeof(reply) - 1;
    int wpa = 0;
    if (ctrlRequest(cmd, reply, &reply_len) >= 0) {
        reply[reply_len] = '\0';
        if (strstr(reply, "RSN") || strstr(reply, "WPA2"))
            wpa = 2;
        else if (strstr(reply, "WPA"))
            wpa = 1;
    }

    int auth = AUTH_NONE_OPEN, encr = 0;
    snprintf(cmd, sizeof(cmd), "GET_NETWORK %d key_mgmt", network_id);
    reply_len = sizeof(reply) - 1;
    if (ctrlRequest(cmd, reply, &reply_len) >= 0) {
        reply[reply_len] = '\0';
        if (strstr(reply, "WPA-EAP"))
            auth = wpa & 2 ? AUTH_WPA2_EAP : AUTH_WPA_EAP;
        else if (strstr(reply, "WPA-PSK"))
            auth = wpa & 2 ? AUTH_WPA2_PSK : AUTH_WPA_PSK;
        else if (strstr(reply, "IEEE8021X")) {
            auth = AUTH_IEEE8021X;
            encr = 1;
        }
    }

    snprintf(cmd, sizeof(cmd), "GET_NETWORK %d pairwise", network_id);
    reply_len = sizeof(reply) - 1;
    if (ctrlRequest(cmd, reply, &reply_len) >= 0) {
        reply[reply_len] = '\0';
        if (strstr(reply, "CCMP") && auth != AUTH_NONE_OPEN &&
            auth != AUTH_NONE_WEP && auth != AUTH_NONE_WEP_SHARED)
            encr = 1;
        else if (strstr(reply, "TKIP"))
            encr = 0;
        else if (strstr(reply, "WEP"))
            encr = 1;
        else
            encr = 0;
    }

    snprintf(cmd, sizeof(cmd), "GET_NETWORK %d psk", network_id);
    reply_len = sizeof(reply) - 1;
    res = ctrlRequest(cmd, reply, &reply_len);
    if (res >= 0 && reply_len >= 2 && reply[0] == '"') {
        reply[reply_len] = '\0';
        pos = strchr(reply + 1, '"');
        if (pos)
            *pos = '\0';
        ui->m_Password->setText(reply + 1);
    } else {
        qDebug() << "Failed to get PSK\n";
    }
#if 0
    else if (res >= 0 && key_value_isset(reply, reply_len)) {
        ui->m_Password->setText(WPA_GUI_KEY_DATA);
    }
#endif
#if 0
    snprintf(cmd, sizeof(cmd), "GET_NETWORK %d identity", network_id);
    reply_len = sizeof(reply) - 1;
    if (ctrlRequest(cmd, reply, &reply_len) >= 0 &&
        reply_len >= 2 && reply[0] == '"') {
        reply[reply_len] = '\0';
        pos = strchr(reply + 1, '"');
        if (pos)
            *pos = '\0';
        identityEdit->setText(reply + 1);
    }
#endif


    if (auth == AUTH_NONE_WEP) {
        snprintf(cmd, sizeof(cmd), "GET_NETWORK %d auth_alg",
             network_id);
        reply_len = sizeof(reply) - 1;
        if (ctrlRequest(cmd, reply, &reply_len) >= 0) {
            reply[reply_len] = '\0';
            if (strcmp(reply, "SHARED") == 0)
                auth = AUTH_NONE_WEP_SHARED;
        }
    }

    ui->m_AuthSelect->setCurrentIndex(auth);
#if 0
    authChanged(auth);
    encrSelect->setCurrentIndex(encr);
    wepEnabled(auth == AUTH_NONE_WEP || auth == AUTH_NONE_WEP_SHARED);

    removeButton->setEnabled(true);
    addButton->setText("Save");
#endif
}

void CWifiSetup::removeNetwork(int nNetworkId)
{
    QString cmd;
    char reply[10];
    size_t reply_len = sizeof(reply);

    cmd = QString::number(nNetworkId);
    cmd.prepend("REMOVE_NETWORK ");
    ctrlRequest(cmd.toAscii().constData(), reply, &reply_len);
    //triggerUpdate();
}

void CWifiSetup::removeAllNetworks()
{
    QString cmd = "all";
    char reply[10];
    size_t reply_len = sizeof(reply);

    cmd.prepend("REMOVE_NETWORK ");
    ctrlRequest(cmd.toAscii().constData(), reply, &reply_len);
    //triggerUpdate();
}

void CWifiSetup::selectNetwork()
{
    QString cmd;
    char reply[10];
    size_t reply_len = sizeof(reply);

    cmd = QString::number(m_NetworkId);
    cmd.prepend("SELECT_NETWORK ");
    ctrlRequest(cmd.toAscii().constData(), reply, &reply_len);
    //triggerUpdate();
    //stopWpsRun(false);
}

void CWifiSetup::connectNetwork()
{
    QString cmd;
    char reply[10];
    size_t reply_len = sizeof(reply);

    removeAllNetworks();

    if(addNetwork() == 0) {
        cmd = QString::number(m_NetworkId);
        cmd.prepend("SELECT_NETWORK ");
        ctrlRequest(cmd.toAscii().constData(), reply, &reply_len);
        //triggerUpdate();
        //stopWpsRun(false);
    }
}

CWifiSetup::CWifiSetup(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CWifiSetup)
{
    m_pVkbd = new Keyboard(this);
    ui->setupUi(this);

    pWpaCtrl = wpa_ctrl_open(wpas_ctrl_path);

    connect( ui->m_Ssid,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd()));
    connect( ui->m_Password,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd()));

    connect( ui->m_IpAddress,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd()));
    //connect( ui->m_NetworkMask,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd()));
    connect( ui->m_Gateway,SIGNAL(selectionChanged()),this ,SLOT(open_vkbd()));
    connect(ui->m_BtnScan, SIGNAL(clicked()), this, SLOT(scanRequest()));

    connect(this, SIGNAL(timeout()), this, SLOT(updateScanResults()));
    connect(ui->m_Networks, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(bssSelected(QTreeWidgetItem *)));

    connect(ui->m_BtnConnect, SIGNAL(clicked()), this, 	SLOT(connectNetwork()));

    setWindowFlags(Qt::SplashScreen);
    //updateNetworks();
    paramsFromConfig(0);
    scanRequest();
    updateScanResults();
    GetNetworkList();
}


CWifiSetup::~CWifiSetup()
{
   if(pWpaCtrl)
       wpa_ctrl_close(pWpaCtrl);
    delete m_pVkbd;
    delete ui;
}

void CWifiSetup::on_buttonBox_accepted()
{

}

void CWifiSetup::open_vkbd()
{
    QLineEdit *line = (QLineEdit *)sender();
    m_pVkbd->setLineEdit(line);
    m_pVkbd->move(200,200);
    m_pVkbd->raise();
    m_pVkbd->show();
}

