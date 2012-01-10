/* ============================================================
 * 
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2012-01-05
 * Description : a widget to find missing binaries.
 *
 * Copyright (C) 2012-2012 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// NOTE: this has nothing to do with a binary search, it is a widget to search for binaries...

#ifndef BINARY_SEARCH_H
#define BINARY_SEARCH_H

// Qt includes

#include <QString>
#include <QGroupBox>

// Local includes

#include "binaryiface.h"

namespace KIPIPlugins
{

class KIPIPLUGINS_EXPORT BinarySearch : public QGroupBox
{
    Q_OBJECT

public:

    BinarySearch(QWidget* parent);
    ~BinarySearch();

    void addBinary(KIPIPlugins::BinaryIface& binary);
    void addDirectory(const QString& dir);
    bool allBinariesFound();

public Q_SLOTS:

    void slotAreBinariesFound(bool);

Q_SIGNALS:

    void signalBinariesFound(bool);
    void signalAddDirectory(const QString& dir);

private:

    struct BinarySearchPriv;
    BinarySearchPriv* const d;
};

} // namespace KIPIPlugins

#endif /* BINARY_SEARCH_H */