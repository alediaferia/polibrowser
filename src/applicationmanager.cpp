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
#include "applicationmanager.h"
#include "mainwindow.h"
#include "networkcookiejar.h"

#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QDir>

#include <KAboutData>
#include <KCmdLineArgs>
#include <KDebug>

ApplicationManager::ApplicationManager(QObject *parent) : QObject(parent), m_nmanager(0)
{
//     connect(qApp, SIGNAL(destroyed()), this, SLOT(destroyWindows()));
}

ApplicationManager::~ApplicationManager()
{
    destroyWindows();
}

void ApplicationManager::destroyWindows()
{
    kDebug() << "deleting all windows";
    qDeleteAll(m_windows);
}

ApplicationManager *ApplicationManager::m_instance = 0;

ApplicationManager* ApplicationManager::instance()
{
    if (!m_instance) {
        m_instance = new ApplicationManager(qApp);
    }

    return m_instance;
}

MainWindow* ApplicationManager::createWindow()
{
    MainWindow *mainWindow = new MainWindow();
    mainWindow->show();

    m_windows << QPointer<MainWindow>(mainWindow);

    return mainWindow;
}

QNetworkAccessManager* ApplicationManager::networkAccessManager()
{
    if (!m_nmanager) {
        m_nmanager = new QNetworkAccessManager(this);

        NetworkCookieJar *jar = new NetworkCookieJar(this);
        jar->setCookiesDirectory(cookieDirectory());
        m_nmanager->setCookieJar(jar);

        QNetworkDiskCache *cache = new QNetworkDiskCache(this);
        cache->setCacheDirectory(cacheDirectory());
        m_nmanager->setCache(cache);
    }

    return m_nmanager;
}

QString ApplicationManager::applicationSaveDirectory() const
{
    QDir saveDir = QDir::home();
    kDebug() << saveDir.absolutePath();
    QString appSaveDir = "." + KCmdLineArgs::aboutData()->appName();
    kDebug() << "appSaveDir" << appSaveDir;
    if (!saveDir.exists(appSaveDir)) { // we create the dir .polibrowser if it does not exist
        if (!saveDir.mkdir(appSaveDir)) {
            return QString();
        }
    }

    saveDir.cd(appSaveDir);
    return saveDir.absolutePath();
}

QString ApplicationManager::cacheDirectory() const
{
    QString saveDirPath = applicationSaveDirectory();
    if (saveDirPath.isEmpty()) {
        return QString();
    }

    QDir cacheDir(saveDirPath);
    if (!cacheDir.exists()) {
        return QString();
    }

    if (!cacheDir.exists("cache") && !cacheDir.mkdir("cache")) {
        return QString();
    }

    if (!cacheDir.cd("cache")) {
        return QString();
    }

    return cacheDir.absolutePath();
}

QString ApplicationManager::cookieDirectory() const
{
    QString saveDirPath = applicationSaveDirectory();
    if (saveDirPath.isEmpty()) {
        return QString();
    }

    QDir cookieDir(saveDirPath);
    kDebug() << cookieDir.absolutePath();

    if (!cookieDir.exists()) { // an error occurred
        return QString();
    }
    kDebug() << cookieDir.absolutePath();

    if (!cookieDir.exists("cookies") && !cookieDir.mkdir("cookies")) {
        return QString();
    }

    if (!cookieDir.cd("cookies")) {
        return QString();
    }
    kDebug() << cookieDir.absolutePath();

    // if we are here then cookieDirectory() == ~/.polibrowser/cookies
    return cookieDir.absolutePath();
}
