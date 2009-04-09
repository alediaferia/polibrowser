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
#include <QWebPage>
#include <QWebView>

class QWidget;
class QContextMenuEvent;
class WebView;
class MainWindow;

class WebPage : public QWebPage
{
    Q_OBJECT
public:
    WebPage(QObject *parent = 0);
    ~WebPage();

private:
    bool m_newTab;
    friend class WebView;

    MainWindow* mainWindow();

protected:
    bool acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type);
    QWebPage *createWindow(WebWindowType type);
};

class WebView : public QWebView
{
    Q_OBJECT
public:
    WebView(QWidget *parent = 0);
    ~WebView() {}

    void contextMenuEvent(QContextMenuEvent *event);

protected slots:
    void openInNewTab();
};
