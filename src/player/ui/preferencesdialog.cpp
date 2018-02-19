#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

#include "sugoiengine.h"
#include "ui/mainwindow.h"
#include "mpvhandler.h"
#include "ui/keydialog.h"
#include "fileassoc.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QLibraryInfo>
#include <QLocale>
#include <QApplication>
#include <QFileInfo>
#include <QFileInfoList>
#include <QDesktopServices>
#include <QUrl>

PreferencesDialog::PreferencesDialog(SugoiEngine *sugoi, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog),
    sugoi(sugoi),
    screenshotDir("")
{
    ui->setupUi(this);

    ui->infoWidget->sortByColumn(0, Qt::AscendingOrder);
    sortLock = new SortLock(ui->infoWidget);

    PopulateLangs();

    PopulateMsgLvls();

    PopulateSkinFiles();

    QString ontop = sugoi->window->getOnTop();
    if(ontop == "never")
        ui->neverRadioButton->setChecked(true);
    else if(ontop == "playing")
        ui->playingRadioButton->setChecked(true);
    else if(ontop == "always")
        ui->alwaysRadioButton->setChecked(true);
    ui->resumeCheckBox->setChecked(sugoi->window->getResume());
    ui->groupBox_2->setChecked(sugoi->window->getTrayIconVisible());
    ui->hidePopupCheckBox->setChecked(sugoi->window->getHidePopup());
    ui->langComboBox->setCurrentIndex(ui->langComboBox->findData(sugoi->window->getLang()));
    int autofit = sugoi->window->getAutoFit();
    ui->autoFitCheckBox->setChecked((bool)autofit);
    ui->comboBox->setCurrentText(QString::number(autofit)+"%");
    int maxRecent= sugoi->window->getMaxRecent();
    ui->recentCheckBox->setChecked(maxRecent > 0);
    ui->recentSpinBox->setValue(maxRecent);
    ui->resumeCheckBox->setChecked(sugoi->window->getResume());
    ui->formatComboBox->setCurrentText(sugoi->mpv->getScreenshotFormat());
    screenshotDir = QDir::toNativeSeparators(sugoi->mpv->getScreenshotDir());
    ui->templateLineEdit->setText(sugoi->mpv->getScreenshotTemplate());
    ui->msgLvlComboBox->setCurrentIndex(ui->msgLvlComboBox->findData(sugoi->mpv->getMsgLevel()));
    ui->groupBox_9->setChecked(sugoi->window->getShowVideoPreview());
    ui->backgroundNotAskCheckBox->setChecked(sugoi->window->getAllowRunInBackground());
    ui->quickStartCheckBox->setChecked(sugoi->window->getQuickStartMode());
    ui->autoUpdatePlayerCheckBox->setChecked(sugoi->window->getAutoUpdatePlayer());
    ui->autoUpdateStreamingSupportCheckBox->setChecked(sugoi->window->getAutoUpdateStreamingSupport());
    ui->styleSheetFilesComboBox->setCurrentIndex(ui->styleSheetFilesComboBox->findText(sugoi->window->getSkinFile()));

    // add shortcuts
    saved = sugoi->input;
    PopulateShortcuts();

    if (sugoi->window->getFileAssocType() == FileAssoc::reg_type::ALL)
    {
        ui->assocVideoCheckBox->setChecked(true);
        ui->assocAudioCheckBox->setChecked(true);
    }
    else if (sugoi->window->getFileAssocType() == FileAssoc::reg_type::VIDEO_ONLY)
    {
        ui->assocVideoCheckBox->setChecked(true);
        ui->assocAudioCheckBox->setChecked(false);
    }
    else if (sugoi->window->getFileAssocType() == FileAssoc::reg_type::AUDIO_ONLY)
    {
        ui->assocVideoCheckBox->setChecked(false);
        ui->assocAudioCheckBox->setChecked(true);
    }
    else
    {
        ui->assocVideoCheckBox->setChecked(false);
        ui->assocAudioCheckBox->setChecked(false);
    }

    ui->alwaysAssocCheckBox->setChecked(sugoi->window->getAlwaysCheckFileAssoc());

    ui->pauseWhenMinimizedCheckBox->setChecked(sugoi->window->getPauseWhenMinimized());

    ui->showFullscreenIndicatorCheckBox->setChecked(sugoi->window->getShowFullscreenIndicator());
    ui->osdShowLocalTimeCheckBox->setChecked(sugoi->window->getOSDShowLocalTime());

    connect(ui->editStyleSheetFileButton, &QPushButton::clicked,
            [=]
            {
                QDesktopServices::openUrl(QUrl::fromLocalFile(ui->styleSheetFilesComboBox->currentData().toString()));
            });

    connect(ui->openStyleSheetFolderButton, &QPushButton::clicked,
            [=]
            {
                QString folderPath = QFileInfo(ui->styleSheetFilesComboBox->currentData().toString()).absolutePath();
                QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath));
            });

    connect(ui->updateAssocButton, &QPushButton::clicked,
            [=]
            {
                FileAssoc::reg_type regType;
                if (ui->assocVideoCheckBox->isChecked() && ui->assocAudioCheckBox->isChecked())
                {
                    regType = FileAssoc::reg_type::ALL;
                }
                else if (ui->assocVideoCheckBox->isChecked() && !ui->assocAudioCheckBox->isChecked())
                {
                    regType = FileAssoc::reg_type::VIDEO_ONLY;
                }
                else if (!ui->assocVideoCheckBox->isChecked() && ui->assocAudioCheckBox->isChecked())
                {
                    regType = FileAssoc::reg_type::AUDIO_ONLY;
                }
                else if (!ui->assocVideoCheckBox->isChecked() && !ui->assocAudioCheckBox->isChecked())
                {
                    regType = FileAssoc::reg_type::NONE;
                }
                sugoi->window->SetFileAssoc(regType, false);
            });

    connect(ui->autoFitCheckBox, &QCheckBox::clicked,
            [=](bool b)
            {
                ui->comboBox->setEnabled(b);
            });

    connect(ui->changeButton, &QPushButton::clicked,
            [=]
            {
                QString dir = QFileDialog::getExistingDirectory(this, tr("Choose screenshot directory"), screenshotDir);
                if(dir != QString())
                    screenshotDir = dir;
            });

    connect(ui->addKeyButton, &QPushButton::clicked,
            [=]
            {
                SelectKey(true);
            });

    connect(ui->editKeyButton, &QPushButton::clicked,
            [=]
            {
                int i = ui->infoWidget->currentRow();
                if(i == -1)
                    return;

                SelectKey(false,
                    {ui->infoWidget->item(i, 0)->text(),
                    {ui->infoWidget->item(i, 1)->text(),
                     ui->infoWidget->item(i, 2)->text()}});
            });

    connect(ui->resetKeyButton, &QPushButton::clicked,
            [=]
            {
                if(QMessageBox::question(this, tr("Reset All Key Bindings?"), tr("Are you sure you want to reset all shortcut keys to its original bindings?")) == QMessageBox::Yes)
                {
                    sugoi->input = sugoi->default_input;
                    while(numberOfShortcuts > 0)
                        RemoveRow(numberOfShortcuts-1);
                    PopulateShortcuts();
                }
            });

    connect(ui->removeKeyButton, &QPushButton::clicked,
            [=]
            {
                int row = ui->infoWidget->currentRow();
                if(row == -1)
                    return;

                sugoi->input[ui->infoWidget->item(row, 0)->text()] = {QString(), QString()};
                RemoveRow(row);
            });

    connect(ui->infoWidget, &QTableWidget::currentCellChanged,
            [=](int r,int,int,int)
            {
                ui->editKeyButton->setEnabled(r != -1);
                ui->removeKeyButton->setEnabled(r != -1);
            });

    connect(ui->infoWidget, &QTableWidget::doubleClicked,
            [=](const QModelIndex &index)
            {
                int i = index.row();
                SelectKey(false,
                    {ui->infoWidget->item(i, 0)->text(),
                    {ui->infoWidget->item(i, 1)->text(),
                     ui->infoWidget->item(i, 2)->text()}});
            });

    connect(ui->recentCheckBox, SIGNAL(toggled(bool)),
            ui->recentSpinBox, SLOT(setEnabled(bool)));

    connect(ui->okButton, SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(ui->cancelButton, SIGNAL(clicked()),
            this, SLOT(reject()));
}

PreferencesDialog::~PreferencesDialog()
{
    if (result() == QDialog::Accepted)
    {
        sugoi->window->setSkinFile(ui->styleSheetFilesComboBox->currentText());
        sugoi->window->setAutoUpdatePlayer(ui->autoUpdatePlayerCheckBox->isChecked());
        sugoi->window->setAutoUpdateStreamingSupport(ui->autoUpdateStreamingSupportCheckBox->isChecked());
        sugoi->window->setQuickStartMode(ui->quickStartCheckBox->isChecked());
        sugoi->window->setAllowRunInBackground(ui->backgroundNotAskCheckBox->isChecked());
        sugoi->window->setPauseWhenMinimized(ui->pauseWhenMinimizedCheckBox->isChecked());
        sugoi->window->setShowFullscreenIndicator(ui->showFullscreenIndicatorCheckBox->isChecked());
        sugoi->window->setOSDShowLocalTime(ui->osdShowLocalTimeCheckBox->isChecked());
        sugoi->window->setAlwaysCheckFileAssoc(ui->alwaysAssocCheckBox->isChecked());
        sugoi->window->setResume(ui->resumeCheckBox->isChecked());
        if(ui->neverRadioButton->isChecked())
            sugoi->window->setOnTop("never");
        else if(ui->playingRadioButton->isChecked())
            sugoi->window->setOnTop("playing");
        else if(ui->alwaysRadioButton->isChecked())
            sugoi->window->setOnTop("always");
        sugoi->window->setTrayIconVisible(ui->groupBox_2->isChecked());
        sugoi->window->setHidePopup(ui->hidePopupCheckBox->isChecked());
        sugoi->window->setLang(ui->langComboBox->currentData().toString());
        sugoi->window->setShowVideoPreview(ui->groupBox_9->isChecked());
        if(ui->autoFitCheckBox->isChecked())
            sugoi->window->setAutoFit(ui->comboBox->currentText().left(ui->comboBox->currentText().length()-1).toInt());
        else
            sugoi->window->setAutoFit(0);
        sugoi->window->setMaxRecent(ui->recentCheckBox->isChecked() ? ui->recentSpinBox->value() : 0);
        sugoi->window->setResume(ui->resumeCheckBox->isChecked());
        sugoi->mpv->ScreenshotFormat(ui->formatComboBox->currentText());
        sugoi->mpv->ScreenshotDirectory(screenshotDir);
        sugoi->mpv->ScreenshotTemplate(ui->templateLineEdit->text());
        sugoi->mpv->MsgLevel(ui->msgLvlComboBox->currentData().toString());
        sugoi->window->MapShortcuts();
        FileAssoc::reg_state regState;
        if (ui->assocVideoCheckBox->isChecked() && ui->assocAudioCheckBox->isChecked())
        {
            regState = FileAssoc::reg_state::ALL_REGISTERED;
        }
        else if (!ui->assocVideoCheckBox->isChecked() && !ui->assocAudioCheckBox->isChecked())
        {
            regState = FileAssoc::reg_state::NOT_REGISTERED;
        }
        else
        {
            regState = FileAssoc::reg_state::SOME_REGISTERED;
        }
        FileAssoc fileAssoc;
        if (regState != fileAssoc.getMediaFilesRegisterState())
        {
            ui->updateAssocButton->clicked();
        }
    }
    else
    {
        sugoi->input = saved;
    }
    sugoi->SaveSettings();
    delete sortLock;
    delete ui;
}

void PreferencesDialog::showPreferences(SugoiEngine *sugoi, QWidget *parent)
{
    PreferencesDialog dialog(sugoi, parent);
    dialog.exec();
}

void PreferencesDialog::PopulateSkinFiles()
{
    QString dirPath = QApplication::applicationDirPath() + QDir::separator() + QString::fromLatin1("stylesheets");
    QDir dir(dirPath);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    dir.setSorting(QDir::Name);
    QFileInfoList fileList = dir.entryInfoList();
    int fileCount = fileList.count();
    if (fileCount < 1)
    {
        return;
    }
    for (int i = 0; i < fileCount; ++i)
    {
        QFileInfo fi = fileList.at(i);
        ui->styleSheetFilesComboBox->addItem(fi.completeBaseName(), QDir::toNativeSeparators(fi.absoluteFilePath()));
    }
}

void PreferencesDialog::PopulateLangs()
{
    // open the language directory
    QString langPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    QDir root(langPath);
    // get files in the directory with .qm extension
    QFileInfoList flist;
    flist = root.entryInfoList({"*.qm"}, QDir::Files);
    // add the languages to the combo box
    ui->langComboBox->addItem(tr("auto"), QVariant(QString::fromLatin1("auto")));
    for(auto &i : flist)
    {
        QString lang = i.fileName().mid(i.fileName().indexOf("_") + 1); // sugoi_....
        lang.chop(3); // -  .qm
        lang = lang.replace('-', '_');
        QLocale locale(lang);
        ui->langComboBox->addItem(locale.nativeLanguageName(), lang);
    }
}

void PreferencesDialog::PopulateMsgLvls()
{
    ui->msgLvlComboBox->clear();
    ui->msgLvlComboBox->addItem(tr("No"), QString::fromLatin1("no"));
    ui->msgLvlComboBox->addItem(tr("Fatal"), QString::fromLatin1("fatal"));
    ui->msgLvlComboBox->addItem(tr("Error"), QString::fromLatin1("error"));
    ui->msgLvlComboBox->addItem(tr("Warn"), QString::fromLatin1("warn"));
    ui->msgLvlComboBox->addItem(tr("Info"), QString::fromLatin1("info"));
    ui->msgLvlComboBox->addItem(tr("Status"), QString::fromLatin1("status"));
    ui->msgLvlComboBox->addItem(tr("V"), QString::fromLatin1("v"));
    ui->msgLvlComboBox->addItem(tr("Debug"), QString::fromLatin1("debug"));
    ui->msgLvlComboBox->addItem(tr("Trace"), QString::fromLatin1("trace"));
    ui->msgLvlComboBox->setCurrentIndex(5);
}

void PreferencesDialog::PopulateShortcuts()
{
    sortLock->lock();
    numberOfShortcuts = 0;
    for(auto iter = sugoi->input.begin(); iter != sugoi->input.end(); ++iter)
    {
        QPair<QString, QString> p = iter.value();
        if(p.first == QString() || p.second == QString())
            continue;
        AddRow(iter.key(), p.first, p.second);
    }
    sortLock->unlock();
}

void PreferencesDialog::AddRow(QString first, QString second, QString third)
{
    bool locked = sortLock->tryLock();
    ui->infoWidget->insertRow(numberOfShortcuts);
    ui->infoWidget->setItem(numberOfShortcuts, 0, new QTableWidgetItem(first));
    ui->infoWidget->setItem(numberOfShortcuts, 1, new QTableWidgetItem(second));
    ui->infoWidget->setItem(numberOfShortcuts, 2, new QTableWidgetItem(third));
    ++numberOfShortcuts;
    if(locked)
        sortLock->unlock();
}

void PreferencesDialog::ModifyRow(int row, QString first, QString second, QString third)
{
    bool locked = sortLock->tryLock();
    ui->infoWidget->item(row, 0)->setText(first);
    ui->infoWidget->item(row, 1)->setText(second);
    ui->infoWidget->item(row, 2)->setText(third);
    if(locked)
        sortLock->unlock();
}

void PreferencesDialog::RemoveRow(int row)
{
    bool locked = sortLock->tryLock();
    ui->infoWidget->removeCellWidget(row, 0);
    ui->infoWidget->removeCellWidget(row, 1);
    ui->infoWidget->removeCellWidget(row, 2);
    ui->infoWidget->removeRow(row);
    --numberOfShortcuts;
    if(locked)
        sortLock->unlock();
}

void PreferencesDialog::SelectKey(bool add, QPair<QString, QPair<QString, QString>> init)
{
    sortLock->lock();
    KeyDialog dialog(this);
    int status = 0;
    while(status != 2)
    {
        QPair<QString, QPair<QString, QString>> result = dialog.SelectKey(add, init);
        if(result == QPair<QString, QPair<QString, QString>>()) // cancel
            break;
        for(int i = 0; i < numberOfShortcuts; ++i)
        {
            if(!add && i == ui->infoWidget->currentRow()) // don't compare selected row if we're changing
                continue;
            if(ui->infoWidget->item(i, 0)->text() == result.first)
            {
                if(QMessageBox::question(this,
                       tr("Existing keybinding"),
                       tr("%0 is already being used. Would you like to change its function?").arg(
                           result.first)) == QMessageBox::Yes)
                {
                    sugoi->input[ui->infoWidget->item(i, 0)->text()] = {QString(), QString()};
                    RemoveRow(i);
                    status = 0;
                }
                else
                {
                    init = result;
                    status = 1;
                }
                break;
            }
        }
        if(status == 0)
        {
            if(add) // add
                AddRow(result.first, result.second.first, result.second.second);
            else // change
            {
                if(result.first != init.first)
                    sugoi->input[init.first] = {QString(), QString()};
                ModifyRow(ui->infoWidget->currentRow(), result.first, result.second.first, result.second.second);
            }
            sugoi->input[result.first] = result.second;
            status = 2;
        }
        else
            status = 0;
    }
    sortLock->unlock();
}

PreferencesDialog::SortLock::SortLock(QTableWidget *parent):parent(parent) {}

void PreferencesDialog::SortLock::lock()
{
    parent->setSortingEnabled(false);
    QMutex::lock();
}

void PreferencesDialog::SortLock::unlock()
{
    QMutex::unlock();
    parent->setSortingEnabled(true);
}
