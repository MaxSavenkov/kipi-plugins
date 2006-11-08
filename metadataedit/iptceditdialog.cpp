/* ============================================================
 * Authors: Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2006-10-12
 * Description : a dialog to edit IPTC metadata
 * 
 * Copyright 2006 by Gilles Caulier
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

// Qt includes.

#include <qtimer.h>
#include <qframe.h>
#include <qlayout.h>
#include <qpushbutton.h>

// KDE includes.

#include <klocale.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <khelpmenu.h>
#include <kpopupmenu.h>
#include <kaboutdata.h>

// Local includes.

#include "pluginsversion.h"
#include "exiv2iface.h"
#include "iptccaption.h"
#include "iptccredits.h"
#include "iptcstatus.h"
#include "iptcorigin.h"
#include "iptcdatetime.h"
#include "iptckeywords.h"
#include "iptcsubjects.h"
#include "iptccategories.h"
#include "iptceditdialog.h"
#include "iptceditdialog.moc"

namespace KIPIMetadataEditPlugin
{

class IPTCEditDialogDialogPrivate
{

public:

    IPTCEditDialogDialogPrivate()
    {
        modified      = false;
        isReadOnly    = false;
        page_caption  = 0;
        page_subjects = 0;
        page_keywords = 0;
        page_credits  = 0;
        page_status   = 0;
        page_origin   = 0;
        page_datetime = 0;

        captionPage   = 0;
        subjectsPage  = 0;
        keywordsPage  = 0;
        creditsPage   = 0;
        statusPage    = 0;
        originPage    = 0;
        datetimePage  = 0;
    }

    bool                  modified;
    bool                  isReadOnly;

    QByteArray            iptcData;

    QFrame               *page_caption;
    QFrame               *page_subjects;
    QFrame               *page_keywords;
    QFrame               *page_categories;
    QFrame               *page_credits;
    QFrame               *page_status;
    QFrame               *page_origin;
    QFrame               *page_datetime;

    KURL::List            urls;

    KURL::List::iterator  currItem;

    IPTCCaption          *captionPage;
    IPTCSubjects         *subjectsPage;
    IPTCKeywords         *keywordsPage;
    IPTCCategories       *categoriesPage;
    IPTCCredits          *creditsPage;
    IPTCStatus           *statusPage;
    IPTCOrigin           *originPage;
    IPTCDateTime         *datetimePage;
};

IPTCEditDialog::IPTCEditDialog(QWidget* parent, KURL::List urls)
              : KDialogBase(IconList, QString::null, 
                            urls.count() > 1 ? Help|User1|User2|Stretch|Ok|Apply|Close 
                                             : Help|Stretch|Ok|Apply|Close, 
                            Ok, parent, 0, true, true,
                            KStdGuiItem::guiItem(KStdGuiItem::Forward),
                            KStdGuiItem::guiItem(KStdGuiItem::Back) )
{
    d = new IPTCEditDialogDialogPrivate;
    d->urls     = urls;
    d->currItem = d->urls.begin();

    // ---------------------------------------------------------------

    d->page_caption    = addPage(i18n("Caption"), i18n("Caption Informations"),
                                 BarIcon("editclear", KIcon::SizeMedium));
    d->captionPage     = new IPTCCaption(d->page_caption);

    d->page_subjects   = addPage(i18n("Subjects"), i18n("Subjects Informations"),
                                 BarIcon("cookie", KIcon::SizeMedium));
    d->subjectsPage    = new IPTCSubjects(d->page_subjects);

    d->page_keywords   = addPage(i18n("Keywords"), i18n("Keywords Informations"),
                                 BarIcon("bookmark", KIcon::SizeMedium));
    d->keywordsPage    = new IPTCKeywords(d->page_keywords);

    d->page_categories = addPage(i18n("Categories"), i18n("Categories Informations"),
                                 BarIcon("bookmark_folder", KIcon::SizeMedium));
    d->categoriesPage  = new IPTCCategories(d->page_categories);

    d->page_credits    = addPage(i18n("Credits"), i18n("Credits Informations"),
                                 BarIcon("identity", KIcon::SizeMedium));
    d->creditsPage     = new IPTCCredits(d->page_credits);
  
    d->page_status     = addPage(i18n("Status"), i18n("Status Informations"),
                                 BarIcon("messagebox_info", KIcon::SizeMedium));
    d->statusPage      = new IPTCStatus(d->page_status);

    d->page_origin     = addPage(i18n("Origin"), i18n("Origin Informations"),
                                 BarIcon("www", KIcon::SizeMedium));
    d->originPage      = new IPTCOrigin(d->page_origin);

    d->page_datetime   = addPage(i18n("Date & Time"), i18n("Date and Time Informations"),
                                 BarIcon("today", KIcon::SizeMedium));
    d->datetimePage    = new IPTCDateTime(d->page_datetime);

    // ---------------------------------------------------------------
    // About data and help button.

    KAboutData* about = new KAboutData("kipiplugins",
                                       I18N_NOOP("Edit Metadata"),
                                       kipiplugins_version,
                                       I18N_NOOP("A Plugin to edit pictures metadata"),
                                       KAboutData::License_GPL,
                                       "(c) 2006, Gilles Caulier",
                                       0,
                                       "http://extragear.kde.org/apps/kipi");

    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and Maintainer"),
                     "caulier dot gilles at kdemail dot net");

    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Edit Metadata Handbook"),
                                 this, SLOT(slotHelp()), 0, -1, 0);
    actionButton(Help)->setPopup( helpMenu->menu() );

    // ------------------------------------------------------------

    connect(d->captionPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->subjectsPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->keywordsPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->categoriesPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->creditsPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->statusPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->originPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->datetimePage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    // ------------------------------------------------------------

    readSettings();
    slotItemChanged();
}

IPTCEditDialog::~IPTCEditDialog()
{
    delete d;
}

void IPTCEditDialog::slotHelp()
{
    KApplication::kApplication()->invokeHelp("metadataedit", "kipi-plugins");
}

void IPTCEditDialog::closeEvent(QCloseEvent *e)
{
    if (!e) return;
    saveSettings();
    e->accept();
}

void IPTCEditDialog::slotClose()
{
    saveSettings();
    KDialogBase::slotClose();
}

void IPTCEditDialog::readSettings()
{
    KConfig config("kipirc");
    config.setGroup("Metadata Edit Settings");
    showPage(config.readNumEntry("IPTC Edit Page", 0));
    resize(configDialogSize(config, QString("IPTC Edit Dialog")));
}

void IPTCEditDialog::saveSettings()
{
    KConfig config("kipirc");
    config.setGroup("Metadata Edit Settings");
    config.writeEntry("IPTC Edit Page", activePageIndex());
    saveDialogSize(config, QString("IPTC Edit Dialog"));
    config.sync();
}

void IPTCEditDialog::slotItemChanged()
{
    KIPIPlugins::Exiv2Iface exiv2Iface;
    exiv2Iface.load((*d->currItem).path());
    d->iptcData = exiv2Iface.getIptc();
    d->captionPage->readMetadata(d->iptcData);
    d->subjectsPage->readMetadata(d->iptcData);
    d->keywordsPage->readMetadata(d->iptcData);
    d->categoriesPage->readMetadata(d->iptcData);
    d->creditsPage->readMetadata(d->iptcData);
    d->statusPage->readMetadata(d->iptcData);
    d->originPage->readMetadata(d->iptcData);
    d->datetimePage->readMetadata(d->iptcData);

    d->isReadOnly = KIPIPlugins::Exiv2Iface::isReadOnly((*d->currItem).path()); 
    d->page_caption->setEnabled(!d->isReadOnly);
    d->page_subjects->setEnabled(!d->isReadOnly);
    d->page_keywords->setEnabled(!d->isReadOnly);
    d->page_categories->setEnabled(!d->isReadOnly);
    d->page_credits->setEnabled(!d->isReadOnly);
    d->page_status->setEnabled(!d->isReadOnly);
    d->page_origin->setEnabled(!d->isReadOnly);
    d->page_datetime->setEnabled(!d->isReadOnly);
    enableButton(Apply, !d->isReadOnly);
    
    setCaption(i18n("%1 (%2/%3) - Edit IPTC Metadata%4")
               .arg((*d->currItem).filename())
               .arg(d->urls.findIndex(*(d->currItem))+1)
               .arg(d->urls.count())
               .arg(d->isReadOnly ? i18n(" - (read only)") : QString::null));
    enableButton(User1, *(d->currItem) != d->urls.last());
    enableButton(User2, *(d->currItem) != d->urls.first());
    enableButton(Apply, false);
}

void IPTCEditDialog::slotApply()
{
    if (d->modified && !d->isReadOnly) 
    {
        d->captionPage->applyMetadata(d->iptcData);
        d->subjectsPage->applyMetadata(d->iptcData);
        d->keywordsPage->applyMetadata(d->iptcData);
        d->categoriesPage->applyMetadata(d->iptcData);
        d->creditsPage->applyMetadata(d->iptcData);
        d->statusPage->applyMetadata(d->iptcData);
        d->originPage->applyMetadata(d->iptcData);
        d->datetimePage->applyMetadata(d->iptcData);
        KIPIPlugins::Exiv2Iface exiv2Iface;
        exiv2Iface.load((*d->currItem).path());
        exiv2Iface.setExif(d->iptcData);
        exiv2Iface.save((*d->currItem).path());
        d->modified = false;
    }
}

void IPTCEditDialog::slotUser1()
{
    slotApply();
    d->currItem++;
    slotItemChanged();
}

void IPTCEditDialog::slotUser2()
{
    slotApply();
    d->currItem--;
    slotItemChanged();
}

void IPTCEditDialog::slotModified()
{
    if (!d->isReadOnly)
    {
        enableButton(Apply, true);
        d->modified = true;
    }
}

void IPTCEditDialog::slotOk()
{
    saveSettings();
    accept();
}

bool IPTCEditDialog::eventFilter(QObject *, QEvent *e)
{
    if ( e->type() == QEvent::KeyPress )
    {
        QKeyEvent *k = (QKeyEvent *)e;

        if (k->state() == Qt::ControlButton &&
            (k->key() == Qt::Key_Enter || k->key() == Qt::Key_Return))
        {
            slotApply();

            if (actionButton(User1)->isEnabled())
                slotUser1();

            return true;
        }
        else if (k->state() == Qt::ShiftButton &&
                 (k->key() == Qt::Key_Enter || k->key() == Qt::Key_Return))
        {
            slotApply();

            if (actionButton(User2)->isEnabled())
                slotUser2();

            return true;
        }

        return false;
    }

    return false;
}

}  // namespace KIPIMetadataEditPlugin
