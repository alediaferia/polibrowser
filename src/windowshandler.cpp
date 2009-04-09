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
#include "windowshandler.h"
#include "mainwindow.h"

#include <QApplication>
#include <QNetworkAccessManager>

#include <KDebug>

WindowsHandler::WindowsHandler(QObject *parent) : QObject(parent), m_nmanager(0)
{
//     connect(qApp, SIGNAL(destroyed()), this, SLOT(destroyWindows()));
}

WindowsHandler::~WindowsHandler()
{
    destroyWindows();
}

void WindowsHandler::destroyWindows()
{
    kDebug() << "deleting all windows";
    qDeleteAll(m_windows);
}

WindowsHandler *WindowsHandler::m_instance = 0;

WindowsHandler* WindowsHandler::instance()
{
    if (!m_instance) {
        m_instance = new WindowsHandler(qApp);
    }

    return m_instance;
}

MainWindow* WindowsHandler::createWindow()
{
    MainWindow *mainWindow = new MainWindow();
    mainWindow->show();

    m_windows << QPointer<MainWindow>(mainWindow);

    return mainWindow;
}

QNetworkAccessManager* WindowsHandler::networkAccessManager()
{
    if (!m_nmanager) {
        m_nmanager = new QNetworkAccessManager(this);
    }

    return m_nmanager;
}


