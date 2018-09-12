#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "sugoi-player-version.h"

#include <QFile>
#include <QTextStream>
#include <QSysInfo>
#include <QCoreApplication>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    ui->compilerText->setText(compilerText_HTML());
    ui->aboutText->setText(aboutText_HTML());

    connect(ui->closeButton_About, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->aboutQtButton, SIGNAL(clicked()), qApp, SLOT(aboutQt()));
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::about(QWidget *parent)
{
    AboutDialog dialog(parent);
    dialog.exec();
}

QString AboutDialog::compilerText_HTML()
{
#ifndef CI
    static QString text = QStringLiteral("<p><b>%1</b>: %2<br /><b>%3</b>: %4<br /><b>%5</b>: %6<br /><b>%7</b>: %8<br /><b>%9</b>: %10<br /><b>%11</b>: %12</p>")
                     .arg(tr("Version")).arg(QString::fromStdWString(SUGOI_VERSION_STR))
                     .arg(tr("Architecture")).arg(QSysInfo::buildCpuArchitecture())
                     .arg(tr("libmpv version")).arg(QString::fromStdWString(LIBMPV_VERSION_STR))
                     .arg(tr("Qt version")).arg(QString::fromLatin1(QT_VERSION_STR));
#else
    static QString text = QStringLiteral("<p><b>%1</b>: %2<br /><b>%3</b>: %4<br /><b>%5</b>: %6<br /><b>%7</b>: %8<br /><b>%9</b>: %10<br /><b>%11</b>: %12<br /><b>%13</b>: %14<br /><b>%15</b>: %16<br /><b>%17</b>: %18<br /><b>%19</b>: %20<br /><b>%21</b>: %22</p>")
                     .arg(tr("Version")).arg(QString::fromStdWString(SUGOI_VERSION_STR))
                     .arg(tr("Commit ID")).arg(QString::fromStdWString(SUGOI_COMMIT_ID_STR))
                     .arg(tr("Commit author")).arg(QString::fromStdWString(SUGOI_COMMIT_AUTHOR_STR))
                     .arg(tr("Commit author e-mail")).arg(QString::fromStdWString(SUGOI_COMMIT_AUTHOR_EMAIL_STR))
                     .arg(tr("Commit time")).arg(QString::fromStdWString(SUGOI_COMMIT_TIMESTAMP_STR))
                     .arg(tr("Commit message")).arg(QString::fromStdWString(SUGOI_COMMIT_MESSAGE_STR))
                     .arg(tr("Architecture")).arg(QSysInfo::buildCpuArchitecture())
                     .arg(tr("libmpv version")).arg(QString::fromStdWString(LIBMPV_VERSION_STR))
                     .arg(tr("Qt version")).arg(QString::fromLatin1(QT_VERSION_STR));
#endif
    return text;
}

QString AboutDialog::compilerText_PlainText()
{
    return compilerText_HTML().remove(QRegExp(QStringLiteral("<[^>]*>")));
}

QString AboutDialog::aboutText_HTML()
{
    static QString text = tr("<p><b>Sugoi Player</b> is a multimedia player based on "
                          "<b>libmpv</b> and <b>Qt</b> for Microsoft Windows 7+.</p>\n"
                          "<p>People should know that <b>Sugoi Player</b> is a fork of "
                          "<a href='https://github.com/u8sand/Baka-MPlayer'><b>Baka MPlayer</b></a>. "
                          "<b>Baka MPlayer</b> is a free and open source, cross-platform, <b>libmpv</b> "
                          "based multimedia player. Its simple design reflects the idea for an uncluttered, "
                          "simple, and enjoyable environment for watching tv shows. "
                          "Thanks to the great work of <b>Baka MPlayer</b>'s original developers. "
                          "Without their hard work, there won't be <b>Sugoi Player</b> anymore. "
                          "I really appreciate <b>godly-devotion</b> (Creator/UX Designer/Programmer), "
                          "<a href='https://github.com/u8sand'><b>u8sand</b></a> (Lead Programmer/Website Host)"
                          " and their team.</p>\n<p>Home Page: <a href='%1'>%2</a></p>")
                          .arg(QString::fromStdWString(SUGOI_COMPANY_URL_STR))
                          .arg(QString::fromStdWString(SUGOI_COMPANY_URL_STR));
    return text;
}

QString AboutDialog::aboutText_PlainText()
{
    return aboutText_HTML().remove(QRegExp(QStringLiteral("<[^>]*>")));
}
