/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2007-11-11
 * Description : a kipi plugin to slide images.
 *
 * Copyright (C) 2007-2008 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
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

#ifndef SLIDESHOWLOADER_H
#define SLIDESHOWLOADER_H

// QT includes

#include <Q3ValueList>
#include <QMap>
#include <QMutex>
#include <QImage>
#include <QThread>
#include <QPair>

// KDE includes

#include <kurl.h>

typedef QPair<QString, int> FileAnglePair;
typedef Q3ValueList<FileAnglePair> FileList;

namespace KIPISlideShowPlugin
{

typedef QMap<KUrl, QImage> LoadedImages;

class LoadThread : public QThread
{

    public:

    LoadThread(LoadedImages* loadedImages, QMutex* imageLock, const KUrl path, 
                const int angle, int width, int height);
    ~LoadThread();

    protected:

    void run();

    private:

    QMutex*       m_imageLock;
    LoadedImages* m_loadedImages;

    KUrl          m_path;
    QString       m_filename;
    int           m_angle;
    int           m_swidth;
    int           m_sheight;
};

typedef QMap<KUrl, LoadThread*> LoadingThreads;

class SlideShowLoader
{

    public: 

    SlideShowLoader(FileList &pathList, uint cacheSize, int width, int height, int beginAtIndex=0);
    ~SlideShowLoader();
    
    void next();
    void prev();
    
    QImage getCurrent();
    QString currFileName();
    KUrl    currPath();
    
    private:

    void checkIsIn(int index); 
    
    LoadingThreads* m_loadingThreads;
    LoadedImages*   m_loadedImages;
    FileList        m_pathList;

    QMutex*         m_imageLock;
    QMutex*         m_threadLock;
    
    uint            m_cacheSize;
    int             m_currIndex;
    int             m_swidth;
    int             m_sheight;
};

}  // NameSpace KIPISlideShowPlugin

#endif // SLIDESHOWLOADER_H
