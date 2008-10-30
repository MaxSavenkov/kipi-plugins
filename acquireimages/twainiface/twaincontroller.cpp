/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2008-27-10
 * Description : Twain interface
 *
 * Copyright (C) 2002-2003 Stephan Stapel <stephan dot stapel at web dot de>
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "qtwain.h"
#include "qtwain.moc"

// Windows includes.

#include <windows.h>

// Qt includes.

#include <QPixmap>
#include <QDataStream>
#include <QByteArray>
#include <QMessageBox>

TwainController::TwainController(QWidget* parent)
               : QWidget(parent), TwainIface()
{
    // This is a dumy widget not visible. We use Qwidget to dispatch Windows event to Twain interface.
    // This is not possible to do it using QObject as well.
    hide();
}

TwainController::~TwainController()
{
    ReleaseTwain();
}

void TwainController::showEvent(QShowEvent*)
{
    // set the parent here to be sure to have a really
    // valid window as the twain parent!
    setParent(this);
    QTimer::singleShot(0, this, SLOT(slotInit()));
}

void TwainController::slotInit()
{
    selectSource();
    if (!acquire())
        QMessageBox::critical(this, QString(), i18n("Cannot acquire image..."));
}

bool TwainController::winEvent(MSG* pMsg, long* result)
{
    processMessage(*pMsg);
    return false;
}

bool TwainController::selectSource()
{
    return (SelectSource() == true);
}

bool TwainController::acquire(unsigned int maxNumImages)
{
    int nMaxNum = 1;

    if (maxNumImages == UINT_MAX)
        nMaxNum = TWCPP_ANYCOUNT;


    return (Acquire(nMaxNum) == true);
}

bool TwainController::isValidDriver() const
{
    return (IsValidDriver() == true);
}

void TwainController::setParent(QWidget* parent)
{
    m_parent = parent;
    if (m_parent)
    {
        if (!onSetParent())
        {
            // TODO
        }
    }
}

bool TwainController::onSetParent()
{
    WId id = m_parent->winId();
    InitTwain(id);

    return IsValidDriver();
}

bool TwainController::processMessage(MSG& msg)
{
    if (msg.message == 528) // TODO: don't really know why...
        return false;

    if (m_hMessageWnd == 0)
        return false;

    return (ProcessMessage(msg) == true);
}

void TwainController::CopyImage(TW_MEMREF pdata, TW_IMAGEINFO& info)
{
    if (pdata && (info.ImageWidth != -1) && (info.ImageLength != - 1))
    {
        // Under Windows, Twain interface return a DIB data structure.
        // See http://en.wikipedia.org/wiki/Device-independent_bitmap#DIBs_in_memory for details.
        HGLOBAL hDIB     = (HGLOBAL)(long)pdata;
        int size         = (int)GlobalSize(hDIB);
        const char* bits = (const char*)GlobalLock(hDIB);

        // DIB is BMP without header. we will add it to load data in QImage using std loader from Qt.
        QByteArray baBmp;
        QDataStream ds(&baBmp, QIODevice::WriteOnly);

        ds.writeRawData("BM", 2);

        qint32 filesize = size + 14;
        ds << filesize;

        qint16 reserved = 0;
        ds << reserved;
        ds << reserved;

        qint32 pixOffset = 14 + 40 + 0;
        ds << pixOffset;

        ds.writeRawData(bits, size);

        QImage img = QImage::fromData(baBmp, "BMP");
        GlobalUnlock(hDIB);

        emit signalImageAcquired(img);
    }
}
