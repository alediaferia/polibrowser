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
#include <QDateTime>
#include <QLatin1Char>
#include <QDirIterator>
#include <QUrl>

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
    QDir cookieDir(cookieDirectory());
    foreach (const QString &cookie, cookieDir.entryList(QDir::Files | QDir::NoDotAndDotDot)) { // this avoids duplications
        cookieDir.remove(cookie);
    }
    saveAllCookiesToDisk();
}

void NetworkCookieJar::saveAllCookiesToDisk()
{
    foreach (const QNetworkCookie &cookie, allCookies()) {
        saveCookieToDisk(cookie);
    }
}

QString NetworkCookieJar::randomCookieName() const
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
    }

    return cookieList;
}

bool NetworkCookieJar::setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url)
{
    QString defaultPath = url.path();
    if (defaultPath.isEmpty()) {
        defaultPath = QLatin1Char('/');
    }
    QString defaultDomain = url.host();
    QDateTime now = QDateTime::currentDateTime();

    int added = 0;
    foreach (QNetworkCookie cookie, cookieList) {
        if (!cookie.domain().startsWith('.') || !defaultDomain.endsWith(cookie.domain())) { // not accepted
            continue;
        }

        if (!cookie.isSessionCookie() && cookie.expirationDate() < now) { // not accepted
            continue;
        }

        if (cookie.path().isEmpty()) { // fixed (we are not so strict =)
            kDebug() << "setting default path to" << defaultPath;
            cookie.setPath(defaultPath);
        }

        // let's look for already existing cookies and eventually delete from the disk
        QDirIterator it(cookieDirectory(), QDir::Files | QDir::NoDotAndDotDot);
        while (it.hasNext()) {
            QFile c(it.next());
            if (!c.open(QIODevice::ReadOnly)) {
                kWarning() << "unable to open cookie" << c.fileName() << "skipping.";
                continue;
            }
            QList<QNetworkCookie> parsedCookies = QNetworkCookie::parseCookies(c.readAll());
            c.close();
            foreach (const QNetworkCookie &diskCookie, parsedCookies) {
                if (diskCookie.name() == cookie.name() &&
                    diskCookie.path() == cookie.path() &&
                    diskCookie.domain() == cookie.domain()) {
      
                    kDebug() << "removal" << c.remove();
                }
            }
        }

        saveCookieToDisk(cookie);
        added++;
    }

    return (added > 0);
}

void NetworkCookieJar::saveCookieToDisk(const QNetworkCookie &cookie)
{
    QDir saveDir(cookieDirectory());
    kDebug() << saveDir.absolutePath();
    if (!saveDir.exists()) {
        kWarning() << "unable to save cookies";
        return;
    }

    QString fileName = randomCookieName();

    while (saveDir.exists(fileName)) {
        fileName = randomCookieName();
    }

    QFile cookieFile(saveDir.absolutePath() + QDir::separator() + fileName);
    kDebug() << "cookie" << cookieFile.fileName();
    if (!cookieFile.open(QIODevice::WriteOnly)) {
        kWarning() << "unable to write cookie";
        return;
    }
    cookieFile.write(cookie.toRawForm());
    cookieFile.close();
}
