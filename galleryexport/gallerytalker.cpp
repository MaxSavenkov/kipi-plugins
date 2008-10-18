/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006 by Colin Guthrie <kde@colin.guthr.ie>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008 by Andrea Diamantini <adjam7 at gmail dot com>
 *
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// standard includes
#include <cstring>
#include <cstdio>

// Qt includes
#include <QByteArray>
#include <QTextStream>
#include <QFile>
#include <QImage>
#include <QRegExp>
#include <QImageReader>

// KDE includes
#include <KLocale>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <KDebug>
#include <KStandardDirs>


// LibKExiv2 includes.
#include <libkexiv2/kexiv2.h>

// local includes
#include "gallerytalker.h"
#include "galleryitem.h"
#include "gallerympform.h"

// self
#include "gallerytalker.moc"

namespace KIPIGalleryExportPlugin
{

GalleryTalker::GalleryTalker(QWidget* parent)
        : m_parent(parent),  m_job(0),  m_loggedIn(false)
{
}


GalleryTalker::~GalleryTalker()
{
    if (m_job)
        m_job->kill();
}


bool GalleryTalker::s_using_gallery2 = true;
QString GalleryTalker::s_authToken = "";

bool GalleryTalker::loggedIn() const
{
    return m_loggedIn;
}



void GalleryTalker::login(const KUrl& url, const QString& name,
                          const QString& passwd)
{
    m_job = 0;
    m_url = url;
    m_state = GE_LOGIN;
    m_talker_buffer.resize(0);

    GalleryMPForm form;
    form.addPair("cmd", "login");
    form.addPair("protocol_version", "2.11");
    form.addPair("uname", name);
    form.addPair("password", passwd);
    form.finish();

    kWarning() << endl << endl << " form : " << form.formData() << endl << endl;

    m_job = KIO::http_post(m_url, form.formData(), KIO::HideProgressInfo);
    m_job->addMetaData("content-type", form.contentType());
    m_job->addMetaData("cookies", "manual");

    connect(m_job, SIGNAL(data(KIO::Job*, const QByteArray&)), this, SLOT(slotTalkerData(KIO::Job*, const QByteArray&)));
    connect(m_job, SIGNAL(result(KJob *)), this, SLOT(slotResult(KJob *)));

    emit signalBusy(true);
}



void GalleryTalker::listAlbums()
{
    m_job = 0;
    m_state = GE_LISTALBUMS;
    m_talker_buffer.resize(0);

    GalleryMPForm form;
    if (s_using_gallery2)
        form.addPair("cmd", "fetch-albums-prune");
    else
        form.addPair("cmd", "fetch-albums");
    form.addPair("protocol_version", "2.11");
    form.finish();

    m_job = KIO::http_post(m_url, form.formData(), KIO::HideProgressInfo);
    m_job->addMetaData("content-type", form.contentType());
    m_job->addMetaData("cookies", "manual");
    m_job->addMetaData("setcookies", m_cookie);

    connect(m_job, SIGNAL(data(KIO::Job*, const QByteArray&)), this, SLOT(slotTalkerData(KIO::Job*, const QByteArray&)));
    connect(m_job, SIGNAL(result(KJob *)), this, SLOT(slotResult(KJob *)));

    emit signalBusy(true);
}



void GalleryTalker::listPhotos(const QString& albumName)
{
    m_job = 0;
    m_state = GE_LISTPHOTOS;
    m_talker_buffer.resize(0);

    GalleryMPForm form;
    form.addPair("cmd", "fetch-album-images");
    form.addPair("protocol_version", "2.11");
    form.addPair("set_albumName", albumName);
    form.finish();

    m_job = KIO::http_post(m_url, form.formData(), KIO::HideProgressInfo);
    m_job->addMetaData("content-type", form.contentType());
    m_job->addMetaData("cookies", "manual");
    m_job->addMetaData("setcookies", m_cookie);

    connect(m_job, SIGNAL(data(KIO::Job*, const QByteArray&)), this, SLOT(slotTalkerData(KIO::Job*, const QByteArray&)));
    connect(m_job, SIGNAL(result(KJob *)), this, SLOT(slotResult(KJob *)));

    emit signalBusy(true);
} 



void GalleryTalker::createAlbum(const QString& parentAlbumName,
                                const QString& albumName,
                                const QString& albumTitle,
                                const QString& albumCaption)
{
    m_job = 0;
    m_state = GE_CREATEALBUM;
    m_talker_buffer.resize(0);

    GalleryMPForm form;
    form.addPair("cmd", "new-album");
    form.addPair("protocol_version", "2.11");
    form.addPair("set_albumName", parentAlbumName);
    if (!albumName.isEmpty())
        form.addPair("newAlbumName", albumName);
    if (!albumTitle.isEmpty())
        form.addPair("newAlbumTitle", albumTitle);
    if (!albumCaption.isEmpty())
        form.addPair("newAlbumDesc", albumCaption);
    form.finish();

    m_job = KIO::http_post(m_url, form.formData(), KIO::HideProgressInfo);
    m_job->addMetaData("content-type", form.contentType());
    m_job->addMetaData("cookies", "manual");
    m_job->addMetaData("setcookies", m_cookie);

    connect(m_job, SIGNAL(data(KIO::Job*, const QByteArray&)), this, SLOT(slotTalkerData(KIO::Job*, const QByteArray&)));
    connect(m_job, SIGNAL(result(KJob *)), this, SLOT(slotResult(KJob *)));

    emit signalBusy(true);
}



bool GalleryTalker::addPhoto(const QString& albumName,
                             const QString& photoPath,
                             const QString& caption,
                             bool  captionIsTitle, bool captionIsDescription,
                             bool  rescale, int maxDim)
{
    m_job = 0;
    QString path = photoPath;
    QString display_filename = QFile::encodeName(KUrl(path).fileName());

    m_state = GE_ADDPHOTO;
    m_talker_buffer.resize(0);

    GalleryMPForm form;
    form.addPair("cmd", "add-item");
    form.addPair("protocol_version", "2.11");
    form.addPair("set_albumName", albumName);

    if (!caption.isEmpty()) {
        if (captionIsTitle)
            form.addPair("caption", caption);
        if (captionIsDescription)
            form.addPair("extrafield.Description", caption);
    }
    QImage image(photoPath);

    if (!image.isNull()) {
        // image file - see if we need to rescale it
        if (rescale && (image.width() > maxDim || image.height() > maxDim)) {
            image = image.scaled(maxDim, maxDim);
            path = KStandardDirs::locateLocal("tmp", KUrl(photoPath).fileName());
            image.save(path);

            if ("JPEG" == QString(QImageReader::imageFormat(photoPath)).toUpper()) {
                KExiv2Iface::KExiv2 exiv2;
                if (exiv2.load(photoPath)) {
                    exiv2.save(path);
                }
            }
            kDebug( 51000 ) << "Resizing and saving to temp file: "
            << path << endl;
        }
    }

    // The filename bit can perhaps be calculated in addFile()
    // but not sure of the temporary filename that could be
    // used for resizing... so I've added it explicitly for now.
    if (!form.addFile(path, display_filename))
        return false;

    form.finish();

    m_job = KIO::http_post(m_url, form.formData(), KIO::HideProgressInfo);
    m_job->addMetaData("content-type", form.contentType());
    m_job->addMetaData("cookies", "manual");
    m_job->addMetaData("setcookies", m_cookie);

    connect(m_job, SIGNAL(data(KIO::Job*, const QByteArray&)), this, SLOT(slotTalkerData(KIO::Job*, const QByteArray&)));
    connect(m_job, SIGNAL(result(KJob *)), this, SLOT(slotResult(KJob *)));

    emit signalBusy(true);

    return true;
}



void GalleryTalker::cancel()
{
    if (m_job) {
        m_job->kill();
        m_job = 0;
    }
}



void GalleryTalker::slotTalkerData(KIO::Job*, const QByteArray& data)
{
    if (data.isEmpty())
        return;

    int oldSize = m_talker_buffer.size();
    m_talker_buffer.resize(oldSize + data.size());
    memcpy(m_talker_buffer.data() + oldSize, data.data(), data.size());
}


void GalleryTalker::slotResult(KJob *job)
{
    emit signalBusy(false);
    KIO::Job *tempjob = static_cast<KIO::Job*>(job);

    if (tempjob->error())
    {
        if (m_state == GE_LOGIN)
        {
            emit signalLoginFailed(tempjob->errorString());
        }
        else
        {
            if (m_state == GE_ADDPHOTO)
            {
                emit signalAddPhotoFailed(tempjob->errorString());
            }
            else
            {
                tempjob->ui()->setWindow(m_parent);
                tempjob->ui()->showErrorMessage();
            }
        }
        return;
    }

    switch (m_state)
    {
        case(GE_LOGIN):
            parseResponseLogin(m_talker_buffer);
            break;
        case(GE_LISTALBUMS):
            parseResponseListAlbums(m_talker_buffer);
            break;
        case(GE_LISTPHOTOS):
            parseResponseListPhotos(m_talker_buffer);
            break;
        case(GE_CREATEALBUM):
            parseResponseCreateAlbum(m_talker_buffer);
            break;
        case(GE_ADDPHOTO):
            parseResponseAddPhoto(m_talker_buffer);
            break;
    }

    if (m_state == GE_LOGIN && m_loggedIn)      // FIXME adjust cookies..
    {
        QStringList cookielist = (tempjob->queryMetaData("setcookies")).split('\n');
        m_cookie = "Cookie:";


        if(!cookielist.isEmpty())
        {
            QRegExp rx("^GALLERYSID=.+");
            QString str, app;
            foreach(str, cookielist)
            {
                if(str.contains("Set-Cookie: "))
                {
                    QStringList cl = str.split(" ");
                    int n = cl.lastIndexOf(rx);
                    if(n!= -1) 
                    {
                        app = cl.at(n);
                    }
                }
            }
            m_cookie += app;
        }

        tempjob->kill();
        listAlbums();
    }
}


void GalleryTalker::parseResponseLogin(const QByteArray &data)
{
    QString *str = new QString(data);
    QTextStream ts(str, QIODevice::ReadOnly);
    ts.setCodec("UTF-8");
    QString line;
    bool foundResponse = false;

    m_loggedIn = false;

    while (!ts.atEnd()) {
        line = ts.readLine();
        if (!foundResponse) {
            foundResponse = line.startsWith("#__GR2PROTO__");
        } else {
            QStringList strlist = line.split('=');
            if (strlist.count() == 2) {
                if (("status" == strlist[0]) && ("0" == strlist[1])) {
                    m_loggedIn = true;
                } else {
                    if ("auth_token" == strlist[0]) {
                        s_authToken = strlist[1];
                    }
                }
            }
        }
    }

    kWarning() << endl << endl << "authToken = " << s_authToken << endl << endl;

    if (!foundResponse) {
        emit signalLoginFailed(i18n("Gallery URL probably incorrect"));
        return;
    }

    if (!m_loggedIn) {
        emit signalLoginFailed(i18n("Incorrect username or password specified"));
    }
}


// -------------------------------------------------------------------------------------
void GalleryTalker::parseResponseListAlbums(const QByteArray &data)
{
    QString str(data);
    QTextStream ts(&str, QIODevice::ReadOnly);
    ts.setCodec("UTF-8");
    QString line;
    bool foundResponse = false;
    bool success = false;

    typedef QList<GAlbum> GAlbumList;
    GAlbumList albumList;
    GAlbumList::iterator iter = albumList.begin();

    while (!ts.atEnd()) {
        line = ts.readLine();
        if (!foundResponse) {
            foundResponse = line.startsWith("#__GR2PROTO__");
        } else {
            QStringList strlist = line.split('=');
            if (strlist.count() == 2) {
                QString key   = strlist[0];
                QString value = strlist[1];

                if (key == "status") {
                    success = (value == "0");
                } else 
                    if (key.startsWith("album.name")) {
                        GAlbum album;
                        album.name    = value;
                        album.ref_num = key.section(".", 2, 2).toInt();
                        iter = albumList.insert(iter, album);
                } else 
                    if (key.startsWith("album.title")) {
                        (*iter).title = value;
                } else 
                    if (key.startsWith("album.summary")) {
                        (*iter).summary = value;
                } else 
                    if (key.startsWith("album.parent")) {
                        (*iter).parent_ref_num = value.toInt();
                } else 
                    if (key.startsWith("album.perms.add")) {
                        (*iter).add = (value == "true");
                } else 
                    if (key.startsWith("album.perms.write")) {
                        (*iter).write = (value == "true");
                } else 
                    if (key.startsWith("album.perms.del_item")) {
                        (*iter).del_item = (value == "true");
                } else 
                    if (key.startsWith("album.perms.del_alb")) {
                        (*iter).del_alb = (value == "true");
                } else 
                    if (key.startsWith("album.perms.create_sub")) {
                        (*iter).create_sub = (value == "true");
                } else 
                    if (key == "auth_token") {
                    s_authToken = value;
                }
            }
        }
    }

    if (!foundResponse) {
        emit signalError(i18n("Invalid response received from remote Gallery"));
        return;
    }

    if (!success) {
        emit signalError(i18n("Failed to list albums"));
        return;
    }

    // We need parent albums to come first for rest of the code to work
    qSort(albumList);

    emit signalAlbums(albumList);
}


// -------------------------------------------------------------------------------------------

void GalleryTalker::parseResponseListPhotos(const QByteArray &data)
{
    QString str(data);
    QTextStream ts(&str, QIODevice::ReadOnly);
    ts.setCodec("UTF-8");
    QString line;
    bool foundResponse = false;
    bool success = false;

    typedef QList<GPhoto> GPhotoList;
    GPhotoList photoList;
    GPhotoList::iterator iter = photoList.begin();

    QString albumURL;

    while (!ts.atEnd()) {
        line = ts.readLine();

        if (!foundResponse) {
            foundResponse = line.startsWith("#__GR2PROTO__");
        } else {
            QStringList strlist = line.split('=');
            if (strlist.count() == 2) {
                QString key   = strlist[0];
                QString value = strlist[1];

                if (key == "status") {
                    success = (value == "0");
                } else 
                    if (key.startsWith("image.name")) {
                        GPhoto photo;
                        photo.name    = value;
                        photo.ref_num = key.section(".", 2, 2).toInt();
                        iter = photoList.insert(iter, photo);
                } else 
                    if (key.startsWith("image.caption")) { 
                        (*iter).caption = value;
                } else 
                    if (key.startsWith("image.thumbName")) {
                        (*iter).thumbName = value;
                } else 
                    if (key.startsWith("baseurl")) {
                        albumURL = value.replace("\\", "");
                }
            }
        }
    }

    if (!foundResponse) {
        emit signalError(i18n("Invalid response received from remote Gallery"));
        return;
    }

    if (!success) {
        emit signalError(i18n("Failed to list photos"));
        return;
    }

    emit signalPhotos(photoList);
}



void GalleryTalker::parseResponseCreateAlbum(const QByteArray &data)
{
    QString str(data);
    QTextStream ts(&str, QIODevice::ReadOnly);
    ts.setCodec("UTF-8");
    QString line;
    bool foundResponse = false;
    bool success = false;

    while (!ts.atEnd()) {
        line = ts.readLine();

        if (!foundResponse) {
            foundResponse = line.startsWith("#__GR2PROTO__");
            kWarning() << "foundResponse: " << foundResponse << endl;
        } else {
            QStringList strlist = line.split('=');
            if (strlist.count() == 2) {
                kWarning() << "line count = 2 " << endl;
                QString key   = strlist[0];
                QString value = strlist[1];
                kWarning() << "key = " << key <<endl;
                kWarning() << "value =  " << value  << endl;
                if (key == "status") {      // key == "status" NOT FOUND!!!
                    success = (value == "0");
                    kWarning( 51000 ) << "Create Album. success: " << success << endl;
                } else if (key.startsWith("status_text")) {
                    kDebug( 51000 ) << "STATUS: Create Album: " << value << endl;
                }

            }
        }
    }

    if (!foundResponse) {
        emit signalError(i18n("Invalid response received from remote Gallery"));
        return;
    }

    if (!success) {
        emit signalError(i18n("Failed to create new album"));
        return;
    }

    listAlbums();
}


void GalleryTalker::parseResponseAddPhoto(const QByteArray &data)
{
    QString str(data);
    QTextStream ts(&str, QIODevice::ReadOnly);
    ts.setCodec("UTF-8");
    QString line;
    bool foundResponse = false;
    bool success = false;

    while (!ts.atEnd()) {
        line = ts.readLine();

        if (!foundResponse) {
            // Gallery1 sends resizing debug code sometimes so we
            // have to detect things slightly differently
            foundResponse = (line.startsWith("#__GR2PROTO__")
                             || (line.startsWith("<br>- Resizing")
                                 && line.endsWith("#__GR2PROTO__")));
        } else {
            QStringList strlist = line.split('=');
            if (strlist.count() == 2) {
                QString key   = strlist[0];
                QString value = strlist[1];

                if (key == "status") {
                    success = (value == "0");
                    kWarning( 51000 ) << "Add photo. success: " << success << endl;
                } else if (key.startsWith("status_text")) {
                    kDebug( 51000 ) << "STATUS: Add Photo: " << value << endl;
                }

            }
        }
    }

    if (!foundResponse) {
        emit signalAddPhotoFailed(i18n("Invalid response received from remote Gallery"));
        return;
    }

    if (!success) {
        emit signalAddPhotoFailed(i18n("Failed to upload photo"));
    } else {
        emit signalAddPhotoSucceeded();
    }
}


}
