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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <KXmlGuiWindow>

class KUrl;
class QWebView;
class KHistoryComboBox;
class KTabWidget;
class QProgressBar;
class QSlider;
class QNetworkRequest;
class QNetworkReply;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setupActions();
    QWebView *currentView();
    void loadConfig();

public slots:
    void goBack();
    void goForward();
    void reload();
    void stop();
    void goToUrl(const KUrl &);
    void goTo(const QString &address);
    void loadHome();
    void loadAddress(); // loads the address specified in the combobox
    void downloadUrl(const KUrl &);
    void addTab();

protected slots:
    void switchTab(int index);
    void slotTitleChanged(const QString &);
    void slotIconChanged();
    void closeTab(QWidget *);
    void slotLinkHovered(const QString &, const QString &, const QString &);
    void handleDownloadRequest(const QNetworkRequest &request);
    void handleUnsupportedContent(QNetworkReply *reply);
    void slotLoadFinished(bool);
    void slotUrlChanged(const QUrl &);

private:
    QWebView *m_webView;
    KTabWidget *m_tabWidget;
    KHistoryComboBox *m_combo;
    QProgressBar *m_progressBar;
    QSlider *m_zoomSlider;
    QString m_home;

    int indexByView(QObject *);
};

#endif
