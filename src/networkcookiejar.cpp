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
}

NetworkCookieJar::~NetworkCookieJar()
{
}

// void NetworkCookieJar::saveAllCookiesToDisk()
// {
//     foreach (const QNetworkCookie &cookie, allCookies()) {
//         saveCookieToDisk(cookie);
//     }
// }

QString NetworkCookieJar::randomCookieName() const
{
    return QString::number(qrand()).append(".txt");
}

// QList<QNetworkCookie> NetworkCookieJar::loadCookiesFromDisk()
// {
//     QList<QNetworkCookie> cookieList;
// 
//     QDir cookieDir(cookieDirectory());
//     kDebug() << cookieDir.absolutePath();
//     foreach (const QString &cookieFile, cookieDir.entryList(QDir::Files | QDir::NoDotAndDotDot)) {
//         cookieList << parseCookieFile(cookieDir.absolutePath() + QDir::separator() + cookieFile);
//     }
// 
//     return cookieList;
// }

bool NetworkCookieJar::isValidByDomain(const QNetworkCookie &cookie, const QUrl &url) const
{
    kDebug() << "cookie =" << cookie.domain()
             << "url =" << url.host();

    if (cookie.domain().startsWith(QLatin1Char('.'))) {
        return url.host().endsWith(cookie.domain()) || cookie.domain().mid(1) == url.host();
    }

    return cookie.domain() == url.host();
}

bool NetworkCookieJar::isValidByPath(const QNetworkCookie &cookie, const QUrl &url) const
{
    QString urlPath = url.path();
    if (urlPath.isEmpty()) {
        urlPath = QLatin1Char('/');
    }
    kDebug() << "cookie path" << cookie.path()
             << "url path" << urlPath;

    if (urlPath.startsWith(cookie.path())) {
        return true;
    }

    return false;
}

bool NetworkCookieJar::isValidByExpirationDate(const QNetworkCookie &cookie) const
{
    QDateTime now = QDateTime::currentDateTime();
    if (!cookie.isSessionCookie() && cookie.expirationDate() < now) {
        return false;
    }

    return true;
}

QList<QNetworkCookie> NetworkCookieJar::cookiesForUrl(const QUrl &url) const
{
    QList<QNetworkCookie> result;
    QString urlDomain = url.host();

    QDirIterator it(m_cookiesDirectory, QDir::Files | QDir::NoDotAndDotDot);
    while (it.hasNext()) {
        QList<QNetworkCookie> parsedCookies = parseCookieFile(it.next());
        foreach (const QNetworkCookie &cookie, parsedCookies) {
            if (!isValidByDomain(cookie, url)) {
                kDebug() << "skipping cookie due to domain";
                continue;
            }
            kDebug() << "accepted cookie by domain";
            if (!isValidByPath(cookie, url)) {
                kDebug() << "skipping cookie due to path";
                continue;
            }
            kDebug() << "accepted cookie by path";

            if (!isValidByExpirationDate(cookie)) {
                continue;
            }

            // since the API we need to sort results by path length
            QList<QNetworkCookie>::iterator i = result.begin();
            while (i != result.end()) {
                if (cookie.path().length() < i->path().length()) {
                    i = result.insert(i, cookie);
                    break;
                } else {
                    ++i;
                }
            }
            if (i == result.end()) { // if we reach the end with the iterator then our cookie has the shortest path, so just append it
                result.append(cookie);
            }
        }
    }

    return result;
}

bool NetworkCookieJar::setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url)
{
    QString defaultPath = url.path();
    if (defaultPath.isEmpty()) {
        defaultPath = QLatin1Char('/');
    }

    int added = 0;
    foreach (QNetworkCookie cookie, cookieList) {

        if (cookie.domain().isEmpty()) {
            cookie.setDomain(url.host());
        } else if (!isValidByDomain(cookie, url)) { // not accepted
            kDebug() << "cookie not accepted due to domain";
            continue;
        }
        kDebug() << "cookie accepted by domain";

        if (!isValidByExpirationDate(cookie)) { // not accepted
            kDebug() << "cookie not accepted due to expiration date";
            continue;
        }

        if (!isValidByPath(cookie, url)) {
            if (cookie.path().isEmpty()) { // fixed (we are not so strict =)
                kDebug() << "setting default path to" << defaultPath;
                cookie.setPath(defaultPath);
            }
        }

        // let's look for already existing cookies and eventually delete from the disk
        QDirIterator it(m_cookiesDirectory, QDir::Files | QDir::NoDotAndDotDot);
        while (it.hasNext()) {
            const QString cookieFile = it.next();
            QList<QNetworkCookie> parsedCookies = parseCookieFile(cookieFile);
            foreach (const QNetworkCookie &diskCookie, parsedCookies) {
                if (diskCookie.name() == cookie.name() &&
                    diskCookie.path() == cookie.path() &&
                    diskCookie.domain() == cookie.domain()) {
      
                    kDebug() << "removal" << cookieFile << QFile::remove(cookieFile);
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
    QDir saveDir(m_cookiesDirectory);
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

QList<QNetworkCookie> NetworkCookieJar::parseCookieFile(const QString &fileName) const
{
    QFile c(fileName);
    if (!c.open(QIODevice::ReadOnly)) {
        kWarning() << "unable to open cookie" << c.fileName();
        return QList<QNetworkCookie>();
     }
     QList<QNetworkCookie> parsedCookies = QNetworkCookie::parseCookies(c.readAll());
     c.close();

    return parsedCookies;
}

void NetworkCookieJar::setCookiesDirectory(const QString &dir)
{
    m_cookiesDirectory = dir;
}

QString NetworkCookieJar::cookiesDirectory() const
{
    return m_cookiesDirectory;
}
