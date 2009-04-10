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
#include "networkcookiejar.h"

#include <QFile>
#include <QDir>
#include <QTime>
#include <QNetworkCookie>

#include <KCmdLineArgs>
#include <KAboutData>
#include <KDebug>

NetworkCookieJar::NetworkCookieJar(QObject *parent) : QNetworkCookieJar(parent)
{
    qsrand(QTime::currentTime().second());
    setAllCookies(loadCookiesFromDisk());
}

NetworkCookieJar::~NetworkCookieJar()
{
    saveCookiesToDisk();
}

void NetworkCookieJar::saveCookiesToDisk()
{
    QDir saveDir(cookieDirectory());
    kDebug() << saveDir.absolutePath();
    if (!saveDir.exists()) {
        kWarning() << "unable to save cookies";
        return;
    }

    foreach (const QNetworkCookie &cookie, allCookies()) {
        QString fileName = randomCookieName();
        while (saveDir.exists(fileName)) {
            fileName = randomCookieName();
        }
        QFile cookieFile(saveDir.absolutePath() + QDir::separator() + fileName);
        kDebug() << "cookie" << cookieFile.fileName();
        if (!cookieFile.open(QIODevice::WriteOnly)) {
            kWarning() << "unable to write cookie";
            continue;
        }
        cookieFile.write(cookie.toRawForm());
        cookieFile.close();
    }
}

QString NetworkCookieJar::randomCookieName()
{
    return QString::number(qrand()).append(".txt");
}

QString NetworkCookieJar::cookieDirectory()
{
    QDir cookieDir = QDir::home();
    kDebug() << cookieDir.absolutePath();
    QString appSaveDir = "." + KCmdLineArgs::aboutData()->appName();
    if (!cookieDir.exists(appSaveDir)) { // we create the dir .polibrowser if it does not exist
        if (!cookieDir.mkdir(appSaveDir)) {
            return QString();
        }
    }
    kDebug() << cookieDir.absolutePath();

    if (!cookieDir.cd(appSaveDir)) { // an error occurred
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

QList<QNetworkCookie> NetworkCookieJar::loadCookiesFromDisk()
{
    QList<QNetworkCookie> cookieList;

    QDir cookieDir(cookieDirectory());
    kDebug() << cookieDir.absolutePath();
    foreach (const QString &cookieFile, cookieDir.entryList(QDir::Files | QDir::NoDotAndDotDot)) {
        kDebug() << "reading" << cookieFile;
        QFile c(cookieDir.absolutePath() + QDir::separator() + cookieFile);
        if (!c.open(QIODevice::ReadOnly)) {
            kWarning() << "unable to read from file" << c.fileName();
            continue;
        }
        cookieList << QNetworkCookie::parseCookies(c.readAll());
        c.close();
        cookieDir.remove(cookieFile);
    }

    return cookieList;
}
