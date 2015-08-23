/*
 * {module name} ButtonFactory
 *
 * {module description} Create application launch buttons for the menus
 *
 * Copyright (C) {YEAR} Texas Instruments Incorporated - http://www.ti.com/ 
 * 
 * 
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the   
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

#include "buttonfactory.h"
#include "buttonlaunch.h"
#include "menupage.h"
#include "mainwindow.h"
#include "onyxcontrol.h"

#include <QtDBus/QDBusConnection>
#ifdef WIN32
#define IMAGE_FOLDER_LOCATION "C:/Projects/onyx/onyx_em/onyx_gui/images/"
#define HTML_FOLDER_LOCATION  "C:/Projects/onyx/onyx_em/onyx_gui/html/"
#else
#define IMAGE_FOLDER_LOCATION "/usr/share/onyx_gui/images/"
#define HTML_FOLDER_LOCATION  "/usr/share/onyx_gui/html/"
#endif
// Construct a custom QWebPluginFactory to create HTML-embeddable application launch buttons.

ButtonFactory::ButtonFactory(MenuPage * const pParentWindow, QMdiArea * const pMdiArea, QWebView * const pView, MatrixControl * const mControl)
{
    m_pView = pView;
    m_pMdiArea = pMdiArea;
    m_pParentWindow = pParentWindow;
    m_pMControl = mControl;


    // Construct list of supported file types.
    QStringList fileExtensionsList;
    fileExtensionsList << "html" << "HTML";

    // Construct list of supported mime types.
    QWebPluginFactory::MimeType mimeType;
    mimeType.name = QString("application/x-matrix");
    mimeType.description = QString("Onyx GUI App Launch Button");
    mimeType.fileExtensions = fileExtensionsList;
    QList<MimeType> mimeTypesList;
    mimeTypesList << mimeType;

    // Construct list of supported QWebPluginFactory types.
    QWebPluginFactory::Plugin plugin;
    plugin.name = QString("LaunchButton");
    plugin.description = QString("Onyx GUI Button Plugin");
    plugin.mimeTypes = mimeTypesList;

    m_PluginList << plugin;
}

// Return our constructed list of supported QWebPluginFactory types.

QList<QWebPluginFactory::Plugin> ButtonFactory::plugins() const
{
    return m_PluginList;
}

// Create an HTML-embeddable application launch button.

QObject * ButtonFactory::create(const QString &mimeType, const QUrl &url, const QStringList &argumentNames, const QStringList &argumentValues) const
{
    // Prevent compiler warning.
    url.isEmpty();

    // Validate parameters from the requesting HTML object tag.
    if (m_PluginList.at(0).mimeTypes.at(0).name != mimeType)
        return 0;

    if ((argumentNames.size() < 2) || (argumentValues.size() < 2))
        return 0;

    if (argumentNames.at(0).toLocal8Bit().constData() != QString("iconName"))
        return 0;

    if (argumentNames.at(1).toLocal8Bit().constData() != QString("appName"))
        return 0;

    // Parse the required parameters from the object tag.
    QString iconName = argumentValues.at(0).toLocal8Bit().constData();
    QString appName = argumentValues.at(1).toLocal8Bit().constData();
    iconName = IMAGE_FOLDER_LOCATION + iconName;
    // Application name parameter is required.
    if (appName == QString(""))
        return 0;

    // Parse optional parameters.
    QString appParameter;
    bool appTextFlag = true;
    bool terminateMatrix = false;
    QString appDescription;

    for (int i = 2; i < argumentNames.size(); i++) {
        if (argumentNames.at(i).toLocal8Bit().constData() == QString("appParameters")) {
            appParameter = argumentValues.at(i).toLocal8Bit().constData();
        }
        else if ((argumentNames.at(i).toLocal8Bit().constData() == QString("appText")) && (argumentValues.at(i).toLocal8Bit().constData() == QString("Disable"))) {
            appTextFlag = false;
	}
        else if ((argumentNames.at(i).toLocal8Bit().constData() == QString("appDesc"))) {
                    appDescription = argumentValues.at(i).toLocal8Bit().constData();
        }
        else if ((argumentNames.at(i).toLocal8Bit().constData() == QString("terminateMatrix")) && (argumentValues.at(i).toLocal8Bit().constData() == QString("Enable"))) { 
            terminateMatrix = true;
        }
    }

    // Create the requested button with a text label.
    QWidget * pWidget = new QWidget(m_pView);
    QWidget *pLaunchButton = NULL;

    if (appName == QString("HlsPublishStatus")) {

        CHlsStatusUpdater *pUpdater = new CHlsStatusUpdater();

        pWidget->setStyleSheet("border-style: none");
        CHlsStatusForm *pForm = new CHlsStatusForm();
        connect(pUpdater, SIGNAL(sendUpdate(int,int,int,int,int)), pForm, SLOT(doUpdate(int,int,int,int,int)));

        pUpdater->setInterval(1000);
        pUpdater->start();

        pWidget->setLayout(pForm);
    } else if (appName == QString("SwitchesStatus")) {

            CSwitchesStatusUpdater *pUpdater = new CSwitchesStatusUpdater();

            pWidget->setStyleSheet("border-style: none");
            CSwitchesStatusForm  *pForm = new CSwitchesStatusForm();
            connect(pUpdater, SIGNAL(sendUpdate(void *)), pForm, SLOT(doUpdate(void *)));

            pUpdater->setInterval(1000);
            pUpdater->start();

            pWidget->setLayout(pForm);
    } else {
        pWidget->setStyleSheet("border-style: none");
        QVBoxLayout * pLayout = new QVBoxLayout();

        // A tool button allows an icon image.
        QToolButton * pButton = new QToolButton();
        pButton->setIconSize(QSize(96, 96));

        // Load an icon image from the specified file and place it on the tool button.
        QIcon icon;
        icon.addFile(iconName, QSize(), QIcon::Normal, QIcon::Off);
        pButton->setIcon(icon);
        //pLayout->setContentsMargins(0, 0, 0, 0);
        pLayout->addWidget(pButton);
        pLaunchButton = pButton;
        pWidget->setLayout(pLayout);
    }


    pWidget->show();

    if (appName == QString("HlsPublishStatus") || appName == QString("SwitchesStatus")) {
        // Do nothing
    }
    else if (appName == QString("Close"))
    {
        // Close and return to the parent window. If a main menu the exit matrix 
        // Connect event from Close button to close menu.
        connect(pLaunchButton, SIGNAL(clicked(bool)), m_pParentWindow, SLOT(Close()));
#ifdef WIN32
#else
        QDBusConnection connection = QDBusConnection::sessionBus(); //PC
        connection.registerObject("/matrix_gui", m_pParentWindow);
        connection.registerService("com.trolltech.Example");
#endif
    }
    else if (appName == QString("AddPage"))
    {
        // Add an additional main menu page of icons
        MenuPage * pMenuPage = new MenuPage(m_pMdiArea, appParameter, true, m_pParentWindow, m_pMControl);
        connect(pLaunchButton, SIGNAL(clicked(bool)), m_pParentWindow, SLOT(CloseMenu()));
        // Connect button event to the Menu Launch object.
        connect(pLaunchButton, SIGNAL(clicked(bool)), pMenuPage, SLOT(Launch()));
#ifdef WIN32
#else
        QDBusConnection connection = QDBusConnection::sessionBus(); //JLPC
        connection.registerObject("/matrix_gui", pMenuPage);
        connection.registerService("com.trolltech.Example");
#endif
    }
    else if (appName == QString("AddSubPage"))
    {
        // Add an additional submenu page of icons
        MenuPage * pMenuPage = new MenuPage(m_pMdiArea, appParameter, false, m_pParentWindow->m_pParentWindow, m_pMControl);
        connect(pLaunchButton, SIGNAL(clicked(bool)), m_pParentWindow, SLOT(CloseMenu()));
        // Connect button event to the Menu Launch object.
        connect(pLaunchButton, SIGNAL(clicked(bool)), pMenuPage, SLOT(Launch()));

#ifdef WIN32
#else
        QDBusConnection connection = QDBusConnection::sessionBus(); //JLPC
        connection.registerObject("/matrix_gui", pMenuPage);
        connection.registerService("com.trolltech.Example");
#endif
    }
    else if (appName == QString("Submenu"))
    {
        if (appParameter.endsWith("html",Qt::CaseInsensitive))
                appParameter = HTML_FOLDER_LOCATION + appParameter;

        // Create the link from a main menu to a sub menu
        MenuPage * pMenuPage = new MenuPage(m_pMdiArea, appParameter, false, m_pParentWindow, m_pMControl);

        // Connect button event to the Menu Launch object.
        connect(pLaunchButton, SIGNAL(clicked(bool)), pMenuPage, SLOT(Launch()));
#ifdef WIN32
#else
        QDBusConnection connection = QDBusConnection::sessionBus();
        connection.registerObject("/matrix_gui", pMenuPage);
        connection.registerService("com.trolltech.Example");
#endif
    }
    else if (appName == QString("Description"))
    {
        // Close the description page, return to parent and launch the application
        connect(pLaunchButton, SIGNAL(clicked(bool)), m_pParentWindow, SLOT(Close()));
        connect(pLaunchButton, SIGNAL(clicked(bool)), m_pParentWindow->m_pParentWindow, SLOT(LaunchApp()));
    }
    else if (appName == QString("TestControl"))
    {
     // Launch a QWidget to allow users to control matrix system settings
      connect(pLaunchButton, SIGNAL(clicked(bool)), m_pMControl, SLOT(SetDescMode()));
    }
    else if (appName == QString("SelectAvSrc"))
    {
      CSelectAvSrc *pSelectAvSrc = new CSelectAvSrc(m_pParentWindow);
      pSelectAvSrc->SetAvSrcOption(appParameter);
      connect(pLaunchButton, SIGNAL(clicked(bool)), pSelectAvSrc, SLOT(setAvSrc()));
#ifdef WIN32
#else

        QDBusConnection connection = QDBusConnection::sessionBus();
        connection.registerObject("/matrix_gui", pSelectAvSrc);
        connection.registerService("com.trolltech.Example");
#endif

    }
    else if (appName == QString("OnyxSetup"))
    {
     // Launch a QWidget to allow users to control matrix system settings
      COnyxSetup *pSetup = new COnyxSetup(m_pParentWindow);
      pSetup->SetOption(appParameter);
      connect(pLaunchButton, SIGNAL(clicked(bool)), pSetup, SLOT(setup()));

#ifdef WIN32
#else

        QDBusConnection connection = QDBusConnection::sessionBus();
        connection.registerObject("/matrix_gui", pSetup);
        connection.registerService("com.trolltech.Example");
#endif

    }

    else if (appName == QString("runscript"))
    {
     // Launch a QWidget to allow users to control matrix system settings
      CLaunchOnyx *pLaunchOnyx = new CLaunchOnyx;
      pLaunchOnyx->SetOption(appParameter);
      connect(pLaunchButton, SIGNAL(clicked(bool)), pLaunchOnyx, SLOT(launch()));
    }
    else
    {
        // Create a ButtonLaunch object to start a process for an external app.        
        ButtonLaunch * pLaunch = new ButtonLaunch(m_pParentWindow, m_pMdiArea, appName, appParameter, appTextFlag, terminateMatrix, appDescription, m_pMControl);

        // Connect button event to the Button Launch object.
        connect(pLaunchButton, SIGNAL(clicked(bool)), pLaunch, SLOT(Launch()));
#ifdef WIN32
#else

        QDBusConnection connection = QDBusConnection::sessionBus();
        connection.registerObject("/matrix_gui", pLaunch);
        connection.registerService("com.trolltech.Example");
#endif
    }
    return pWidget;
}
