/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2006-10-12
 * Description : IPTC status settings page.
 *
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// QT includes.

#include <QLayout>
#include <QLabel>
#include <QValidator>
#include <QComboBox>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <klineedit.h>

// LibKExiv2 includes. 

#include <libkexiv2/kexiv2.h>

// Local includes.

#include "squeezedcombobox.h"
#include "metadatacheckbox.h"
#include "pluginsversion.h"
#include "iptcstatus.h"
#include "iptcstatus.moc"

namespace KIPIMetadataEditPlugin
{

class IPTCStatusPriv
{
public:

    IPTCStatusPriv()
    {
        priorityCB          = 0;
        objectCycleCB       = 0;
        objectTypeCB        = 0;
        statusEdit          = 0;
        JobIDEdit           = 0;
        priorityCheck       = 0;
        objectCycleCheck    = 0;
        objectTypeCheck     = 0;
        statusCheck         = 0;
        JobIDCheck          = 0;
    }

    QCheckBox                     *statusCheck;
    QCheckBox                     *JobIDCheck;

    QComboBox                     *priorityCB;
    QComboBox                     *objectCycleCB;
    QComboBox                     *objectTypeCB;

    KLineEdit                     *statusEdit;
    KLineEdit                     *objectTypeDescEdit;
    KLineEdit                     *objectAttributeDescEdit;
    KLineEdit                     *JobIDEdit;

    MetadataCheckBox              *priorityCheck;
    MetadataCheckBox              *objectCycleCheck;
    MetadataCheckBox              *objectTypeCheck;
    MetadataCheckBox              *objectAttributeCheck;

    KIPIPlugins::SqueezedComboBox *objectAttributeCB;
};

IPTCStatus::IPTCStatus(QWidget* parent)
          : QWidget(parent)
{
    d = new IPTCStatusPriv;

    QGridLayout* grid = new QGridLayout(this);

    // IPTC only accept printable Ascii char.
    QRegExp asciiRx("[\x20-\x7F]+$");
    QValidator *asciiValidator = new QRegExpValidator(asciiRx, this);

    // --------------------------------------------------------

    d->statusCheck = new QCheckBox(i18n("Edit Status:"), this);
    d->statusEdit  = new KLineEdit(this);
    d->statusEdit->setClearButtonShown(true);
    d->statusEdit->setValidator(asciiValidator);
    d->statusEdit->setMaxLength(64);
    d->statusEdit->setWhatsThis(i18n("<p>Set here the content status. This field is limited "
                                     "to 64 ASCII characters."));

    // --------------------------------------------------------

    d->priorityCheck = new MetadataCheckBox(i18n("Priority:"), this);
    d->priorityCB    = new QComboBox(this);
    d->priorityCB->insertItem(0, i18n("0: None"));
    d->priorityCB->insertItem(1, i18n("1: High"));
    d->priorityCB->insertItem(2, "2");
    d->priorityCB->insertItem(3, "3");
    d->priorityCB->insertItem(4, "4");
    d->priorityCB->insertItem(5, i18n("5: Normal"));
    d->priorityCB->insertItem(6, "6");
    d->priorityCB->insertItem(7, "7");
    d->priorityCB->insertItem(8, i18n("8: Low"));
    d->priorityCB->setWhatsThis(i18n("<p>Select here the editorial urgency of content."));

    // --------------------------------------------------------

    d->objectCycleCheck = new MetadataCheckBox(i18n("Object Cycle:"), this);
    d->objectCycleCB    = new QComboBox(this);
    d->objectCycleCB->insertItem(0, i18n("Morning"));
    d->objectCycleCB->insertItem(1, i18n("Afternoon"));
    d->objectCycleCB->insertItem(2, i18n("Evening"));
    d->objectCycleCB->setWhatsThis(i18n("<p>Select here the editorial cycle of content."));
      
    // --------------------------------------------------------

    d->objectTypeCheck    = new MetadataCheckBox(i18n("Object Type:"), this);
    d->objectTypeCB       = new QComboBox(this);
    d->objectTypeDescEdit = new KLineEdit(this);
    d->objectTypeDescEdit->setClearButtonShown(true);
    d->objectTypeDescEdit->setValidator(asciiValidator);
    d->objectTypeDescEdit->setMaxLength(64);
    d->objectTypeCB->insertItem(0, i18n("News"));
    d->objectTypeCB->insertItem(1, i18n("Data"));
    d->objectTypeCB->insertItem(2, i18n("Advisory"));
    d->objectTypeCB->setWhatsThis(i18n("<p>Select here the editorial type of content."));
    d->objectTypeDescEdit->setWhatsThis(i18n("<p>Set here the editorial type description of content. "
                                             "This field is limited to 64 ASCII characters."));

    // --------------------------------------------------------

    d->objectAttributeCheck    = new MetadataCheckBox(i18n("Object Attribute:"), this);
    d->objectAttributeCB       = new KIPIPlugins::SqueezedComboBox(this);
    d->objectAttributeDescEdit = new KLineEdit(this);
    d->objectAttributeDescEdit->setClearButtonShown(true);
    d->objectAttributeDescEdit->setValidator(asciiValidator);
    d->objectAttributeDescEdit->setMaxLength(64);
    d->objectAttributeCB->addSqueezedItem(i18n("Current"));
    d->objectAttributeCB->addSqueezedItem(i18n("Analysis"));
    d->objectAttributeCB->addSqueezedItem(i18n("Archive material"));
    d->objectAttributeCB->addSqueezedItem(i18n("Background"));
    d->objectAttributeCB->addSqueezedItem(i18n("Feature"));
    d->objectAttributeCB->addSqueezedItem(i18n("Forecast"));
    d->objectAttributeCB->addSqueezedItem(i18n("History"));
    d->objectAttributeCB->addSqueezedItem(i18n("Obituary"));
    d->objectAttributeCB->addSqueezedItem(i18n("Opinion"));
    d->objectAttributeCB->addSqueezedItem(i18n("Polls & Surveys"));
    d->objectAttributeCB->addSqueezedItem(i18n("Profile"));
    d->objectAttributeCB->addSqueezedItem(i18n("Results Listings & Table"));
    d->objectAttributeCB->addSqueezedItem(i18n("Side bar & Supporting information"));
    d->objectAttributeCB->addSqueezedItem(i18n("Summary"));
    d->objectAttributeCB->addSqueezedItem(i18n("Transcript & Verbatim"));
    d->objectAttributeCB->addSqueezedItem(i18n("Interview"));
    d->objectAttributeCB->addSqueezedItem(i18n("From the Scene"));
    d->objectAttributeCB->addSqueezedItem(i18n("Retrospective"));
    d->objectAttributeCB->addSqueezedItem(i18n("Statistics"));
    d->objectAttributeCB->addSqueezedItem(i18n("Update"));
    d->objectAttributeCB->addSqueezedItem(i18n("Wrap-up"));
    d->objectAttributeCB->addSqueezedItem(i18n("Press Release"));
    d->objectAttributeCB->setWhatsThis(i18n("<p>Select here the editorial attribute of content."));
    d->objectAttributeDescEdit->setWhatsThis(i18n("<p>Set here the editorial attribute description of "
                                                  "content. This field is limited to 64 ASCII characters."));

    // --------------------------------------------------------

    d->JobIDCheck = new QCheckBox(i18n("Job Identification:"), this);
    d->JobIDEdit  = new KLineEdit(this);
    d->JobIDEdit->setClearButtonShown(true);
    d->JobIDEdit->setValidator(asciiValidator);
    d->JobIDEdit->setMaxLength(32);
    d->JobIDEdit->setWhatsThis(i18n("<p>Set here the string that identifies content that recurs. "
                                    "This field is limited to 32 ASCII characters."));

    // --------------------------------------------------------

    QLabel *note = new QLabel(i18n("<b>Note: "
                 "<b><a href='http://en.wikipedia.org/wiki/IPTC'>IPTC</a></b> "
                 "text tags only support the printable "
                 "<b><a href='http://en.wikipedia.org/wiki/Ascii'>ASCII</a></b> "
                 "characters set and limit strings size. "
                 "Use contextual help for details.</b>"), this);
    note->setOpenExternalLinks(true);
    note->setWordWrap(true);

    // --------------------------------------------------------

    grid->addWidget(d->statusCheck, 0, 0, 1, 3 );
    grid->addWidget(d->statusEdit, 1, 0, 1, 3 );
    grid->addWidget(d->priorityCheck, 2, 0, 1, 1);
    grid->addWidget(d->priorityCB, 2, 1, 1, 1);
    grid->addWidget(d->objectCycleCheck, 3, 0, 1, 1);
    grid->addWidget(d->objectCycleCB, 3, 1, 1, 1);
    grid->addWidget(d->objectTypeCheck, 4, 0, 1, 1);
    grid->addWidget(d->objectTypeCB, 4, 1, 1, 1);
    grid->addWidget(d->objectTypeDescEdit, 5, 0, 1, 3 );
    grid->addWidget(d->objectAttributeCheck, 6, 0, 1, 1);
    grid->addWidget(d->objectAttributeCB, 6, 1, 1, 2);
    grid->addWidget(d->objectAttributeDescEdit, 7, 0, 1, 3 );
    grid->addWidget(d->JobIDCheck, 8, 0, 1, 3 );
    grid->addWidget(d->JobIDEdit, 9, 0, 1, 3 );
    grid->addWidget(note, 10, 0, 1, 3 );
    grid->setColumnStretch(2, 10);                     
    grid->setRowStretch(11, 10);                     
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    connect(d->priorityCheck, SIGNAL(toggled(bool)),
            d->priorityCB, SLOT(setEnabled(bool)));

    connect(d->objectCycleCheck, SIGNAL(toggled(bool)),
            d->objectCycleCB, SLOT(setEnabled(bool)));

    connect(d->objectTypeCheck, SIGNAL(toggled(bool)),
            d->objectTypeCB, SLOT(setEnabled(bool)));

    connect(d->objectTypeCheck, SIGNAL(toggled(bool)),
            d->objectTypeDescEdit, SLOT(setEnabled(bool)));

    connect(d->objectAttributeCheck, SIGNAL(toggled(bool)),
            d->objectAttributeCB, SLOT(setEnabled(bool)));

    connect(d->objectAttributeCheck, SIGNAL(toggled(bool)),
            d->objectAttributeDescEdit, SLOT(setEnabled(bool)));

    connect(d->statusCheck, SIGNAL(toggled(bool)),
            d->statusEdit, SLOT(setEnabled(bool)));

    connect(d->JobIDCheck, SIGNAL(toggled(bool)),
            d->JobIDEdit, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    connect(d->priorityCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->objectCycleCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->objectTypeCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->objectTypeCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->objectAttributeCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->objectAttributeCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->statusCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->JobIDCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->priorityCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->objectCycleCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->objectTypeCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->objectAttributeCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->statusEdit, SIGNAL(textChanged(const QString &)),
            this, SIGNAL(signalModified()));

    connect(d->objectTypeDescEdit, SIGNAL(textChanged(const QString &)),
            this, SIGNAL(signalModified()));

    connect(d->objectAttributeDescEdit, SIGNAL(textChanged(const QString &)),
            this, SIGNAL(signalModified()));

    connect(d->JobIDEdit, SIGNAL(textChanged(const QString &)),
            this, SIGNAL(signalModified()));
}

IPTCStatus::~IPTCStatus()
{
    delete d;
}

void IPTCStatus::readMetadata(QByteArray& iptcData)
{
    blockSignals(true);
    KExiv2Iface::KExiv2 exiv2Iface;
    exiv2Iface.setIptc(iptcData);
    QString data;
    int     val;

    d->statusEdit->clear();
    d->statusCheck->setChecked(false);
    data = exiv2Iface.getIptcTagString("Iptc.Application2.EditStatus", false);    
    if (!data.isNull())
    {
        d->statusEdit->setText(data);
        d->statusCheck->setChecked(true);
    }
    d->statusEdit->setEnabled(d->statusCheck->isChecked());

    d->priorityCB->setCurrentIndex(0);
    d->priorityCheck->setChecked(false);
    data = exiv2Iface.getIptcTagString("Iptc.Application2.Urgency", false);    
    if (!data.isNull())
    {
        val = data.toInt(); 
        if (val >= 0 && val <= 8)
        {
            d->priorityCB->setCurrentIndex(val);
            d->priorityCheck->setChecked(true);
        }
        else
            d->priorityCheck->setValid(false);
    }
    d->priorityCB->setEnabled(d->priorityCheck->isChecked());

    d->objectCycleCB->setCurrentIndex(0);
    d->objectCycleCheck->setChecked(false);
    data = exiv2Iface.getIptcTagString("Iptc.Application2.ObjectCycle", false);    
    if (!data.isNull())
    {
        if (data == QString("a"))
        {
            d->objectCycleCB->setCurrentIndex(0);
            d->objectCycleCheck->setChecked(true);
        }
        else if (data == QString("b"))
        {
            d->objectCycleCB->setCurrentIndex(1);
            d->objectCycleCheck->setChecked(true);
        }
        else if (data == QString("c"))
        {
            d->objectCycleCB->setCurrentIndex(2);
            d->objectCycleCheck->setChecked(true);
        }
        else 
            d->objectCycleCheck->setValid(false);
    }
    d->objectCycleCB->setEnabled(d->objectCycleCheck->isChecked());

    d->objectTypeCB->setCurrentIndex(0);
    d->objectTypeDescEdit->clear();
    d->objectTypeCheck->setChecked(false);
    data = exiv2Iface.getIptcTagString("Iptc.Application2.ObjectType", false);    
    if (!data.isNull())
    {
        QString typeSec = data.section(":", 0, 0);
        if (!typeSec.isEmpty())
        {
            int type = typeSec.toInt()-1;
            if (type >= 0 && type < 3)
            {
                d->objectTypeCB->setCurrentIndex(type);
                d->objectTypeDescEdit->setText(data.section(":", -1));
                d->objectTypeCheck->setChecked(true);
            }
            else
                d->objectTypeCheck->setValid(false);
        }
    }
    d->objectTypeCB->setEnabled(d->objectTypeCheck->isChecked());
    d->objectTypeDescEdit->setEnabled(d->objectTypeCheck->isChecked());

    d->objectAttributeCB->setCurrentIndex(0);
    d->objectAttributeDescEdit->clear();
    d->objectAttributeCheck->setChecked(false);
    data = exiv2Iface.getIptcTagString("Iptc.Application2.ObjectAttribute", false);    
    if (!data.isNull())
    {
        QString attSec = data.section(":", 0, 0);
        if (!attSec.isEmpty())
        {
            int att = attSec.toInt()-1;
            if (att >= 0 && att < 21)
            {
                d->objectAttributeCB->setCurrentIndex(att);
                d->objectAttributeDescEdit->setText(data.section(":", -1));
                d->objectAttributeCheck->setChecked(true);
            }
            else 
                d->objectAttributeCheck->setValid(false);
        }
    }
    d->objectAttributeCB->setEnabled(d->objectAttributeCheck->isChecked());
    d->objectAttributeDescEdit->setEnabled(d->objectAttributeCheck->isChecked());

    d->JobIDEdit->clear();
    d->JobIDCheck->setChecked(false);
    data = exiv2Iface.getIptcTagString("Iptc.Application2.FixtureId", false);    
    if (!data.isNull())
    {
        d->JobIDEdit->setText(data);
        d->JobIDCheck->setChecked(true);
    }
    d->JobIDEdit->setEnabled(d->JobIDCheck->isChecked());

    blockSignals(false);
}

void IPTCStatus::applyMetadata(QByteArray& iptcData)
{
    KExiv2Iface::KExiv2 exiv2Iface;
    exiv2Iface.setIptc(iptcData);

    if (d->statusCheck->isChecked())
        exiv2Iface.setIptcTagString("Iptc.Application2.EditStatus", d->statusEdit->text());
    else
        exiv2Iface.removeIptcTag("Iptc.Application2.EditStatus");

    if (d->priorityCheck->isChecked())
        exiv2Iface.setIptcTagString("Iptc.Application2.Urgency", QString::number(d->priorityCB->currentIndex()));
    else if (d->priorityCheck->isValid())
        exiv2Iface.removeIptcTag("Iptc.Application2.Urgency");

    if (d->objectCycleCheck->isChecked())
    {
        switch (d->objectCycleCB->currentIndex())
        {
            case(0):
                exiv2Iface.setIptcTagString("Iptc.Application2.ObjectCycle", QString("a"));
                break;
                
            case(1):
                exiv2Iface.setIptcTagString("Iptc.Application2.ObjectCycle", QString("b"));
                break;
        
            case(2):
                exiv2Iface.setIptcTagString("Iptc.Application2.ObjectCycle", QString("c"));
                break;
        }
    }
    else if (d->objectCycleCheck->isValid())
        exiv2Iface.removeIptcTag("Iptc.Application2.ObjectCycle");

    if (d->objectTypeCheck->isChecked())
    {
        QString objectType;
        objectType.sprintf("%2d", d->objectTypeCB->currentIndex()+1);
        objectType.append(QString(":%1").arg(d->objectTypeDescEdit->text()));
        exiv2Iface.setIptcTagString("Iptc.Application2.ObjectType", objectType);
    }
    else if (d->objectTypeCheck->isValid())
        exiv2Iface.removeIptcTag("Iptc.Application2.ObjectType");

    if (d->objectAttributeCheck->isChecked())
    {
        QString objectAttribute;
        objectAttribute.sprintf("%3d", d->objectAttributeCB->currentIndex()+1);
        objectAttribute.append(QString(":%1").arg(d->objectAttributeDescEdit->text()));
        exiv2Iface.setIptcTagString("Iptc.Application2.ObjectAttribute", objectAttribute);
    }
    else if (d->objectAttributeCheck->isValid())
        exiv2Iface.removeIptcTag("Iptc.Application2.ObjectAttribute");

    if (d->JobIDCheck->isChecked())
        exiv2Iface.setIptcTagString("Iptc.Application2.FixtureId", d->JobIDEdit->text());
    else
        exiv2Iface.removeIptcTag("Iptc.Application2.FixtureId");

    exiv2Iface.setImageProgramId(QString("Kipi-plugins"), QString(kipiplugins_version));

    iptcData = exiv2Iface.getIptc();
}

}  // namespace KIPIMetadataEditPlugin
