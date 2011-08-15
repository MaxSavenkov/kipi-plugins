/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2011-05-23
 * Description : a plugin to create panorama by fusion of several images.
 *
 * Copyright (C) 2011 by Benjamin Girault <benjamin dot girault at gmail dot com>
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

#include "previewpage.moc"

// Qt includes

#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QCheckBox>

// KDE includes

#include <kstandarddirs.h>
#include <kapplication.h>
#include <kvbox.h>
#include <klocale.h>
#include <kdialog.h>
#include <kdebug.h>
#include <klineedit.h>

// Local includes

#include "outputdialog.h"
#include "batchprogressdialog.h"
#include "savesettingswidget.h"
#include "manager.h"
#include "previewmanager.h"

namespace KIPIPanoramaPlugin
{

struct PreviewPage::PreviewPagePriv
{
    PreviewPagePriv(Manager *m)
        : title(0), previewWidget(0), progressDlg(0), saveSettingsGroupBox(0),
          fileTemplateKLineEdit(0), savePtoCheckBox(0), mngr(m)
    {}

    QLabel*                 title;
    QUrl                    previewUrl;
    PreviewManager*         previewWidget;
    BatchProgressDialog*    progressDlg;
    int                     curProgress, totalProgress;
    QGroupBox*              saveSettingsGroupBox;
    KLineEdit*              fileTemplateKLineEdit;
    QCheckBox*              savePtoCheckBox;

    QString                 output;

    Manager*                mngr;
};

PreviewPage::PreviewPage(Manager* mngr, KAssistantDialog* dlg)
        : KIPIPlugins::WizardPage(dlg, i18n("Preview")),
          d(new PreviewPagePriv(mngr))
{
    KConfig config("kipirc");
    KConfigGroup group              = config.group(QString("Panorama Settings"));

    KVBox *vbox                 = new KVBox(this);
    d->title                    = new QLabel(i18n("<qt>"
                                                  "<p><h1>Panorama Preview</h1></p>"
                                                  "<p>Pressing the <i>Next</i> button launches the final "
                                                  "stitching process.</p>"
                                                  "</qt>"),
                                             vbox);
    d->title->setOpenExternalLinks(true);
    d->title->setWordWrap(true);

    d->previewWidget            = new PreviewManager(vbox);
    d->previewWidget->setButtonText(i18n("Details..."));
    d->previewWidget->show();

    QLabel* space               = new QLabel(vbox);

    QVBoxLayout *formatVBox     = new QVBoxLayout();
    d->saveSettingsGroupBox     = new QGroupBox(i18n("Save Settings"), vbox);
    d->saveSettingsGroupBox->setLayout(formatVBox);
    formatVBox->addStretch(1);

    QLabel *fileTemplateLabel   = new QLabel(i18n("File Name Template: "), d->saveSettingsGroupBox);
    formatVBox->addWidget(fileTemplateLabel);
    d->fileTemplateKLineEdit    = new KLineEdit(d->saveSettingsGroupBox);
    formatVBox->addWidget(d->fileTemplateKLineEdit);
    d->savePtoCheckBox          = new QCheckBox(i18n("Save Project File"), d->saveSettingsGroupBox);
    d->savePtoCheckBox->setChecked(group.readEntry("Save PTO", false));
    formatVBox->addWidget(d->savePtoCheckBox);

    vbox->setStretchFactor(space, 2);
    vbox->setSpacing(KDialog::spacingHint());
    vbox->setMargin(KDialog::spacingHint());

    setPageWidget(vbox);

    //QPixmap leftPix = KStandardDirs::locate("data", "kipiplugin_panorama/pics/assistant-xxx.png");
    //setLeftBottomPix(leftPix.scaledToWidth(128, Qt::SmoothTransformation));

    connect(d->mngr->thread(), SIGNAL(starting(KIPIPanoramaPlugin::ActionData)),
            this, SLOT(slotAction(KIPIPanoramaPlugin::ActionData)));
}

PreviewPage::~PreviewPage()
{
    KConfig config("kipirc");
    KConfigGroup group = config.group(QString("Panorama Settings"));
    group.writeEntry("Save PTO", d->savePtoCheckBox->isChecked());
    config.sync();

    if (d->progressDlg)
        delete d->progressDlg;
    delete d;
}

void PreviewPage::cancel()
{
    disconnect(d->mngr->thread(), SIGNAL(finished(KIPIPanoramaPlugin::ActionData)),
               this, SLOT(slotAction(KIPIPanoramaPlugin::ActionData)));

    d->mngr->thread()->cancel();
    d->previewWidget->setBusy(false);
}

void PreviewPage::computePreview()
{
    d->previewWidget->setBusy(true, i18n("Processing Panorama Preview..."));

    connect(d->mngr->thread(), SIGNAL(finished(KIPIPanoramaPlugin::ActionData)),
            this, SLOT(slotAction(KIPIPanoramaPlugin::ActionData)));

    emit signalPreviewGenerating();
    d->mngr->thread()->generatePanoramaPreview(d->mngr->autoOptimiseUrl(), d->mngr->preProcessedMap());
}

void PreviewPage::startStitching()
{
    d->progressDlg = new BatchProgressDialog(this, QString("Stitching is under progress..."));
    d->curProgress = 0;
    d->totalProgress = d->mngr->preProcessedMap().size() + 1;
    d->progressDlg->setTotal(d->totalProgress);
    d->progressDlg->show();

    connect(d->mngr->thread(), SIGNAL(finished(KIPIPanoramaPlugin::ActionData)),
            this, SLOT(slotAction(KIPIPanoramaPlugin::ActionData)));

    d->mngr->thread()->compileProject(d->mngr->autoOptimiseUrl(),
                                      d->mngr->preProcessedMap(),
                                      d->mngr->format(),
                                      d->fileTemplateKLineEdit->text(),
                                      d->savePtoCheckBox->isChecked());
}

void PreviewPage::resetPage()
{
    computePreview();
}

void PreviewPage::slotAction(const KIPIPanoramaPlugin::ActionData& ad)
{
    QString text;

    if (!ad.starting)           // Something is complete...
    {
        if (!ad.success)        // Something is failed...
        {
            switch (ad.action)
            {
                case PREVIEW :
                {
                    disconnect(d->mngr->thread(), SIGNAL(finished(KIPIPanoramaPlugin::ActionData)),
                               this, SLOT(slotAction(KIPIPanoramaPlugin::ActionData)));

                    d->output = ad.message;
                    d->previewWidget->setBusy(false);
                    d->previewWidget->setText(ad.message);
                    kDebug() << "Preview compilation failed";
                    emit signalPreviewGenerated(KUrl());
                    break;
                }
                case STITCH:
                {
                    disconnect(d->mngr->thread(), SIGNAL(finished(KIPIPanoramaPlugin::ActionData)),
                               this, SLOT(slotAction(KIPIPanoramaPlugin::ActionData)));

                    d->progressDlg->addedAction(QString("Panorama compilation: ") + ad.message, ErrorMessage);
                    kDebug() << "Enblend call failed";
                    break;
                }
                case NONAFILE:
                {
                    QStringList message;
                    message << "Processing file ";
                    message << QString::number(ad.id + 1);
                    message << " / ";
                    message << QString::number(d->totalProgress - 1);
                    message << ": ";
                    message << ad.message;
                    kDebug() << "Nona call failed for file #" << ad.id;
                    d->progressDlg->addedAction(message.join(""), ErrorMessage);
                    break;
                }
                default:
                {
                    kWarning() << "Unknown action";
                    break;
                }
            }
        }
        else                    // Something is done...
        {
            switch (ad.action)
            {
                case PREVIEW:
                {
                    disconnect(d->mngr->thread(), SIGNAL(finished(KIPIPanoramaPlugin::ActionData)),
                               this, SLOT(slotAction(KIPIPanoramaPlugin::ActionData)));

                    d->previewUrl = ad.outUrl;
                    d->mngr->setPreviewUrl(ad.outUrl);
                    d->previewWidget->load(ad.outUrl.toLocalFile(), true);
                    kDebug() << "Preview URL: " << ad.outUrl.toLocalFile();

                    emit signalPreviewGenerated(ad.outUrl);
                    break;
                }
                case STITCH:
                {
                    disconnect(d->mngr->thread(), SIGNAL(finished(KIPIPanoramaPlugin::ActionData)),
                               this, SLOT(slotAction(KIPIPanoramaPlugin::ActionData)));

                    d->progressDlg->addedAction(QString("Panorama compilation"), SuccessMessage);
                    d->curProgress++;
                    d->progressDlg->setProgress(d->curProgress, d->totalProgress);
                    d->progressDlg->hide();
                    kDebug() << "Panorama URL: " << ad.outUrl.toLocalFile();
                    emit signalStitchingFinished();
                    break;
                }
                case NONAFILE:
                {
                    QStringList message;
                    message << "Processing file ";
                    message << QString::number(ad.id + 1);
                    message << " / ";
                    message << QString::number(d->totalProgress - 1);
                    d->progressDlg->addedAction(message.join(""), SuccessMessage);
                    d->curProgress++;
                    d->progressDlg->setProgress(d->curProgress, d->totalProgress);
                    kDebug() << "Nona URL #" << ad.id << " URL: " << ad.outUrl.toLocalFile();
                    break;
                }
                default:
                {
                    kWarning() << "Unknown action";
                    break;
                }
            }
        }
    }
    else
    {
        switch (ad.action)
        {
            case STITCH:
            {
                d->progressDlg->addedAction(QString("Panorama compilation"), StartingMessage);
                break;
            }
            case NONAFILE:
            {
                QStringList message;
                message << "Processing file ";
                message << QString::number(ad.id + 1);
                message << " / ";
                message << QString::number(d->totalProgress - 1);
                d->progressDlg->addedAction(message.join(""), StartingMessage);
                break;
            }
            default:
            {
                kWarning() << "Unknown starting action";
                break;
            }
        }
    }
}

}   // namespace KIPIPanoramaPlugin
