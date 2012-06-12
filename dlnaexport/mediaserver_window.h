/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2012-05-28
 * Description : a KIPI plugin to export pics through DLNA technology.
 *
 * Copyright (C) 2012 by Smit Mehta <smit dot meh at gmail dot com>
 * Copyright (C) 2011 Tuomo Penttinen <tp at herqq dot org>
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

#ifndef MEDIASERVER_WINDOW_H
#define MEDIASERVER_WINDOW_H

// Qt includes

#include <QObject>
#include <QString>

namespace KIPIDLNAExportPlugin
{

class MediaServerWindow : public QObject
{
    Q_OBJECT

public:

    MediaServerWindow(QObject* const parent=0);
    virtual ~MediaServerWindow();

    void onAddContentButtonClicked(const QString&, bool);

private:

    class Private;
    Private* const d;
};

} // namespace KIPIDLNAExportPlugin

#endif // MEDIASERVER_WINDOW_H
