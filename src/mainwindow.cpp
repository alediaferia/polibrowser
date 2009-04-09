 /* 
 * Copyright 2009 Alessandro Diaferia <alediaferia@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy 
 * defined in Section 14 of version 3 of the license.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

#include "mainwindow.h"
#include "webview.h"
#include "windowshandler.h"

#include <QtWebKit/QWebView>
#include <QtWebKit/QWebPage>
#include <QtWebKit/QWebSettings>
#include <QTimer>
#include <QLineEdit>
#include <QRegExp>
#include <QProgressBar>
#include <QFontMetrics>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <KLocale>
#include <KAction>
#include <KActionCollection>
#include <KIcon>
#include <KMenuBar>
#include <KToolBar>
#include <KStatusBar>
#include <KUrl>
#include <KHistoryComboBox>
#include <KTabWidget>
#include <KDebug>
#include <KFileDialog>
#include <KMimeType>
#include <KConfig>
#include <KConfigGroup>

#include <KIO/NetAccess>

MainWindow::MainWindow(QWidget *parent) : KXmlGuiWindow(parent), m_combo(new KHistoryComboBox(this)), m_zoomSlider(new QSlider(Qt::Horizontal, this))
{
    m_tabWidget = new KTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setTabBarHidden(true);
    connect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(switchTab(int)));
    connect(m_tabWidget, SIGNAL(mouseDoubleClick()), this, SLOT(addTab()));
    connect(m_tabWidget, SIGNAL(closeRequest(QWidget*)), this, SLOT(closeTab(QWidget*)));

    m_zoomSlider->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    setupActions();

    setCentralWidget(m_tabWidget);
    addTab();

    setupGUI();

    loadHome();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupActions()
{
    KAction *backAction = new KAction(this);
    backAction->setText(i18n("Back"));
    backAction->setIcon(KIcon("go-previous"));
    actionCollection()->addAction("back", backAction);
    connect(backAction, SIGNAL(triggered()), this, SLOT(goBack()));

    KAction *forwardAction = new KAction(this);
    forwardAction->setText(i18n("Forward"));
    forwardAction->setIcon(KIcon("go-next"));
    actionCollection()->addAction("forward", forwardAction);
    connect(forwardAction, SIGNAL(triggered()), this, SLOT(goForward()));

    KAction *reloadAction = new KAction(this);
    reloadAction->setText(i18n("Reload"));
    reloadAction->setIcon(KIcon("view-refresh"));
    actionCollection()->addAction("reload", reloadAction);
    connect(reloadAction, SIGNAL(triggered()), this, SLOT(reload()));

    KAction *stopAction = new KAction(this);
    stopAction->setText(i18n("Stop"));
    stopAction->setIcon(KIcon("process-stop"));
    actionCollection()->addAction("stop", stopAction);
    connect(stopAction, SIGNAL(triggered()), this, SLOT(stop()));

    m_combo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(m_combo, SIGNAL(returnPressed()), this, SLOT(loadAddress()));

    KAction *comboAction = new KAction(this);
    comboAction->setText(i18n("Location Combo Box"));
    actionCollection()->addAction("location_combo", comboAction);
    comboAction->setDefaultWidget(m_combo);

    QAction *goAction = actionCollection()->addAction("go_url");
    goAction->setIcon(KIcon("go-jump-locationbar"));
    goAction->setText(i18n("Go"));
    connect(goAction, SIGNAL(triggered()), this, SLOT(loadAddress()));

    actionCollection()->addAction("go", menuBar()->addMenu(i18n("Go")));
    actionCollection()->addAction("file", menuBar()->addMenu(i18n("File")));

    KAction *tabAction = new KAction(this);
    tabAction->setText(i18n("New tab"));
    tabAction->setIcon(KIcon("tab-new"));
    tabAction->setShortcut(Qt::CTRL+Qt::Key_T);
    actionCollection()->addAction("new_tab", tabAction);
    connect(tabAction, SIGNAL(triggered()), this, SLOT(addTab()));

    KAction *newWindow = new KAction(this);
    newWindow->setText(i18n("New window"));
    newWindow->setIcon(KIcon("window-new"));
    newWindow->setShortcut(Qt::CTRL+Qt::Key_N);
    actionCollection()->addAction("new_window", newWindow);
    connect(newWindow, SIGNAL(triggered()), WindowsHandler::instance(), SLOT(createWindow()));

    KAction *zoomAction = new KAction(this);
    zoomAction->setText(i18n("Zoom factor slider"));
    actionCollection()->addAction("zoom_slider", zoomAction);
    zoomAction->setDefaultWidget(m_zoomSlider);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setMaximumHeight(QFontMetrics(font()).height());
    m_progressBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    statusBar()->addPermanentWidget(m_progressBar);
}

void MainWindow::loadConfig()
{
    KConfigGroup cfg(KGlobal::config(), "HomePage");
    m_home = cfg.readEntry("url", "http://www.kde.org");
}

void MainWindow::goBack()
{
    currentView()->back();
}

void MainWindow::goForward()
{
    currentView()->forward();
}

void MainWindow::reload()
{
    currentView()->reload();
}

void MainWindow::stop()
{
    currentView()->stop();
}

void MainWindow::goToUrl(const KUrl &url)
{
    currentView()->setUrl(url);
}

void MainWindow::goTo(const QString &address)
{
    KUrl url(address);

    if (url.scheme().isEmpty()) {
        url = "//" + url.url();
        url.setScheme("http");
    }

    goToUrl(url);
}

void MainWindow::loadHome()
{
    // TODO: use settings to store home url
    if (m_home.isNull()) {
        loadConfig();
    }

    goTo(m_home);
}

void MainWindow::loadAddress()
{
    QString address = m_combo->lineEdit()->text();

    goTo(address);
}

void MainWindow::switchTab(int index)
{
    Q_UNUSED(index)

    disconnect(m_progressBar);
    connect(currentView(), SIGNAL(loadProgress(int)), m_progressBar, SLOT(setValue(int)));
    connect(currentView(), SIGNAL(loadProgress(int)), m_progressBar, SLOT(show()));

    
    m_combo->setCurrentItem(KUrl(currentView()->url()).prettyUrl(), true);
    setCaption(currentView()->title());
}

void MainWindow::addTab()
{
    QWebView *webView = new WebView(this);
    webView->page()->settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
    webView->page()->settings()->setAttribute(QWebSettings::JavaEnabled, true);
    webView->page()->settings()->setAttribute(QWebSettings::PluginsEnabled, true);

    webView->page()->setForwardUnsupportedContent(true);

    connect (webView, SIGNAL(titleChanged(const QString &)), this, SLOT(slotTitleChanged(const QString &)));
    connect (webView, SIGNAL(iconChanged()), this, SLOT(slotIconChanged()));
    connect (webView, SIGNAL(loadFinished(bool)), this, SLOT(slotLoadFinished(bool)));
    connect (webView, SIGNAL(urlChanged(const QUrl &)), this, SLOT(slotUrlChanged(const QUrl &)));
    connect (webView->page(), SIGNAL(linkHovered(const QString &, const QString&, const QString&)),
             this, SLOT(slotLinkHovered(const QString &, const QString &, const QString &)));
    connect (webView->page(), SIGNAL(downloadRequested(const QNetworkRequest &)), this, SLOT(handleDownloadRequest(const QNetworkRequest &)));
    connect (webView->page(), SIGNAL(unsupportedContent(QNetworkReply *)), this, SLOT(handleUnsupportedContent(QNetworkReply *)));

    m_tabWidget->addTab(webView, i18n("Untitled Tab"));
    if (m_tabWidget->count() > 1) {
        m_tabWidget->setTabBarHidden(false);
    }

    m_tabWidget->setCurrentWidget(webView);
}

QWebView *MainWindow::currentView()
{
    return static_cast<QWebView*>(m_tabWidget->currentWidget());
}

void MainWindow::slotTitleChanged(const QString &title)
{
    int index = indexByView(sender());
    if (index == -1) {
        return;
    }

    m_tabWidget->setTabText(index, title);

    if (index == m_tabWidget->indexOf(currentView())) {
        setCaption(title);
    }
}

void MainWindow::slotIconChanged()
{
    int index = indexByView(sender());
    if (index == -1) {
        return;
    }

    m_tabWidget->setTabIcon(index, qobject_cast<QWebView*>(sender())->icon());
}

int MainWindow::indexByView(QObject *object)
{
    QWebView *view = qobject_cast<QWebView*>(object);
    int index = m_tabWidget->indexOf(view);

    return index;
}

void MainWindow::closeTab(QWidget *page)
{
    disconnect(page);
    m_tabWidget->removeTab(m_tabWidget->indexOf(page));
    delete page;

    if (m_tabWidget->count() == 1) {
        m_tabWidget->setTabBarHidden(true);
    }
}

void MainWindow::slotLinkHovered(const QString &link, const QString &title, const QString &textContext)
{
    Q_UNUSED(title)
    Q_UNUSED(textContext)

    statusBar()->showMessage(link);
}

void MainWindow::handleDownloadRequest(const QNetworkRequest &request)
{
    downloadUrl(request.url());
}

void MainWindow::handleUnsupportedContent(QNetworkReply *reply)
{
    if (!reply->error()) {
        downloadUrl(reply->url());
    }
}

void MainWindow::downloadUrl(const KUrl &url)
{
    if (!url.isValid()) {
        return;
    }

    QString suffix = KMimeType::extractKnownExtension(url.fileName());

    KFileDialog fileDialog(KUrl(), "*."+suffix, this);
    fileDialog.setConfirmOverwrite(true);
    fileDialog.setOperationMode(KFileDialog::Saving);
    fileDialog.setSelection(url.fileName());
    fileDialog.exec();

    KUrl destination = fileDialog.selectedUrl();
    if (!destination.isValid()) {
        // TODO: warning message
        return;
    }

    KIO::NetAccess::file_copy(url, destination);
}

void MainWindow::slotLoadFinished(bool ok)
{
    Q_UNUSED(ok)
    m_progressBar->hide();
    currentView()->setFocus(Qt::OtherFocusReason);
}

void MainWindow::slotUrlChanged(const QUrl &url)
{
    m_combo->setCurrentItem(KUrl(url).prettyUrl(), true);
}
