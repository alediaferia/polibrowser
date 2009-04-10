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
#include "webview.h"

#include "windowshandler.h"
#include "networkcookiejar.h"
#include "mainwindow.h"

#include <QContextMenuEvent>
#include <QWebFrame>
#include <QWebHitTestResult>
#include <QNetworkRequest>

#include <KMenu>
#include <KAction>
#include <KLocale>
#include <KIcon>
#include <KDebug>

WebPage::WebPage(QObject *parent) : QWebPage(parent), m_newTab(false)
{
    setNetworkAccessManager(WindowsHandler::instance()->networkAccessManager());
    networkAccessManager()->setCookieJar(new NetworkCookieJar(this));
}

WebPage::~WebPage()
{}

bool WebPage::acceptNavigationRequest ( QWebFrame * frame, const QNetworkRequest & request, NavigationType type )
{
    kDebug() << "Accepting Navigation Request";
    return QWebPage::acceptNavigationRequest(frame, request, type);
}

MainWindow* WebPage::mainWindow()
{
    QObject *window = parent();
    while (window) {
        MainWindow *mw = qobject_cast<MainWindow*>(window);
        if (mw) {
            return mw;
        }
        window = window->parent();
    }

    return 0;
}

QWebPage* WebPage::createWindow ( WebWindowType type )
{
    if (m_newTab) {
        m_newTab = false;
        MainWindow *mw = mainWindow();
        if (mw) {
            mw->addTab();
            QWebPage *page = mw->currentView()->page();
            return page;
        }
    }

    MainWindow *window = WindowsHandler::instance()->createWindow();
    QWebPage *page = window->currentView()->page();
    return page;
}

///////////WEBVIEW STUFF

WebView::WebView (QWidget *parent) : QWebView(parent)
{
    setPage(new WebPage(this));

    pageAction(QWebPage::OpenLinkInNewWindow)->setIcon(KIcon("window-new"));
    pageAction(QWebPage::DownloadLinkToDisk)->setIcon(KIcon("document-save-as"));
    pageAction(QWebPage::OpenImageInNewWindow)->setIcon(KIcon("window-new"));
    pageAction(QWebPage::DownloadImageToDisk)->setIcon(KIcon("document-save"));
}
void WebView::contextMenuEvent(QContextMenuEvent *event)
{
    QWebHitTestResult result = page()->mainFrame()->hitTestContent(event->pos());
    if (!result.linkUrl().isEmpty()) {
        KMenu menu(this);

        KAction *newTab = new KAction(this);
        connect (newTab, SIGNAL(triggered()), this, SLOT(openInNewTab()));
        newTab->setText(i18n("Open in new tab"));
        newTab->setIcon(KIcon("tab-new"));
        menu.addAction(newTab);

        menu.addAction(pageAction(QWebPage::OpenLinkInNewWindow));
        menu.addSeparator();
        menu.addAction(pageAction(QWebPage::DownloadLinkToDisk));
        menu.addAction(pageAction(QWebPage::CopyLinkToClipboard));

        if (!result.imageUrl().isEmpty()) {
            menu.addAction(pageAction(QWebPage::OpenImageInNewWindow));
            menu.addAction(pageAction(QWebPage::DownloadImageToDisk));
        }

        menu.exec(mapToGlobal(event->pos()));
        return;
    }

    QWebView::contextMenuEvent(event);
}

void WebView::openInNewTab()
{
    static_cast<WebPage*>(page())->m_newTab = true;
    triggerPageAction(QWebPage::OpenLinkInNewWindow);
}
