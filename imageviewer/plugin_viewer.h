/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2007-02-11
 * Description : a kipi plugin to show image using an OpenGL interface.
 *
 * Copyright (C) 2007-2008 by Markus Leuthold <kusi at forum dot titlis dot org>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef PLUGIN_VIEWER_H
#define PLUGIN_VIEWER_H

// LibKIPI includes

#include <libkipi/plugin.h>

// Local includes

#include "viewerwidget.h"

class KAction;

using namespace KIPI;

namespace KIPIViewerPlugin
{

class Plugin_viewer :public Plugin
{
    Q_OBJECT

public:

    Plugin_viewer(QObject* const parent, const QVariantList&);
    ~Plugin_viewer();
    
    void setup(QWidget* widget);
    Category category(KAction* action) const;

public Q_SLOTS:

    void  slotActivate();

protected:

    ViewerWidget* m_widget;
    KAction*      m_actionViewer;
};

} // namespace KIPIViewerPlugin

#endif /* PLUGIN_VIEWER_H */
