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
#ifndef NETWORKCOOKIEJAR_H
#define NETWORKCOOKIEJAR_H

#include <QNetworkCookieJar>
#include <QList>

class QNetworkCookie;

class NetworkCookieJar : public QNetworkCookieJar
{
    Q_OBJECT
public:
    NetworkCookieJar(QObject *parent);
    ~NetworkCookieJar();

    bool setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url);
    QList<QNetworkCookie> cookiesForUrl(const QUrl &) const;

    void setCookiesDirectory(const QString &dir);
    QString cookiesDirectory() const;

private:
//     QList<QNetworkCookie> loadCookiesFromDisk();
    QList<QNetworkCookie> parseCookieFile(const QString &fileName) const;

//     void saveAllCookiesToDisk();
    void saveCookieToDisk(const QNetworkCookie &);

    QString cookieDirectory() const;

    QString randomCookieName() const;

    bool isValidByDomain(const QNetworkCookie &, const QUrl &) const;
    bool isValidByPath(const QNetworkCookie &, const QUrl &) const;
    bool isValidByExpirationDate(const QNetworkCookie &) const;

private:
    QString m_cookiesDirectory;

};

#endif
