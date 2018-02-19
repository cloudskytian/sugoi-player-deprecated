#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "splayer-version.h"
#include "winsparkle-version.h"

#include <QFile>
#include <QTextStream>
#include <QSysInfo>
#include <QCoreApplication>

AboutDialog::AboutDialog(QString lang, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    ui->compilerText->setText(compilerText_HTML());
    ui->aboutText->setText(aboutText_HTML());
    ui->updateLogText->setText(updateLogText_HTML(lang));
    ui->creditsText->setText(creditsText_HTML());
    ui->licenseText->setText(licenseText_HTML(lang));

    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->aboutQtButton, SIGNAL(clicked()), qApp, SLOT(aboutQt()));
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::about(QString lang, QWidget *parent)
{
    AboutDialog dialog(lang, parent);
    dialog.exec();
}

QString AboutDialog::compilerText_HTML()
{
#ifndef CI
    static QString text = QString::fromLatin1("<p><b>%1</b>: %2<br /><b>%3</b>: %4<br /><b>%5</b>: %6<br /><b>%7</b>: %8<br /><b>%9</b>: %10<br /><b>%11</b>: %12</p>")
                     .arg(tr("Version")).arg(QString::fromStdWString(SPLAYER_VERSION_STR))
                     .arg(tr("Architecture")).arg(QSysInfo::buildCpuArchitecture())
                     .arg(tr("libmpv version")).arg(QString::fromStdWString(LIBMPV_VERSION_STR))
                     .arg(tr("Qt version")).arg(QString::fromLatin1(QT_VERSION_STR))
                     .arg(tr("WinSparkle version")).arg(QString::fromLatin1(WIN_SPARKLE_VERSION_STRING))
                     .arg(tr("Compiler")).arg(QString::fromLatin1("MSVC ") + QString::number(_MSC_FULL_VER));
#else
    static QString text = QString::fromLatin1("<p><b>%1</b>: %2<br /><b>%3</b>: %4<br /><b>%5</b>: %6<br /><b>%7</b>: %8<br /><b>%9</b>: %10<br /><b>%11</b>: %12<br /><b>%13</b>: %14<br /><b>%15</b>: %16<br /><b>%17</b>: %18<br /><b>%19</b>: %20<br /><b>%21</b>: %22</p>")
                     .arg(tr("Version")).arg(QString::fromStdWString(SPLAYER_VERSION_STR))
                     .arg(tr("Commit ID")).arg(QString::fromStdWString(SPLAYER_COMMIT_ID_STR))
                     .arg(tr("Commit author")).arg(QString::fromStdWString(SPLAYER_COMMIT_AUTHOR_STR))
                     .arg(tr("Commit author e-mail")).arg(QString::fromStdWString(SPLAYER_COMMIT_AUTHOR_EMAIL_STR))
                     .arg(tr("Commit time")).arg(QString::fromStdWString(SPLAYER_COMMIT_TIMESTAMP_STR))
                     .arg(tr("Commit message")).arg(QString::fromStdWString(SPLAYER_COMMIT_MESSAGE_STR))
                     .arg(tr("Architecture")).arg(QSysInfo::buildCpuArchitecture())
                     .arg(tr("libmpv version")).arg(QString::fromStdWString(LIBMPV_VERSION_STR))
                     .arg(tr("Qt version")).arg(QString::fromLatin1(QT_VERSION_STR))
                     .arg(tr("WinSparkle version")).arg(QString::fromLatin1(WIN_SPARKLE_VERSION_STRING))
                     .arg(tr("Compiler")).arg(QString::fromLatin1("MSVC ") + QString::number(_MSC_FULL_VER));
#endif
    return text;
}

QString AboutDialog::compilerText_PlainText()
{
    return compilerText_HTML().remove(QRegExp(QStringLiteral("<[^>]*>")));
}

QString AboutDialog::aboutText_HTML()
{
    static QString text = tr("<p><b>SPlayer</b> is a multimedia player based on "
                          "<b>libmpv</b> and <b>Qt</b> for Microsoft Windows 7+.</p>\n"
                          "<p>People should know that <b>SPlayer</b> is a fork of "
                          "<a href='https://github.com/u8sand/Baka-MPlayer'><b>Baka MPlayer</b></a>. "
                          "<b>Baka MPlayer</b> is a free and open source, cross-platform, <b>libmpv</b> "
                          "based multimedia player. Its simple design reflects the idea for an uncluttered, "
                          "simple, and enjoyable environment for watching tv shows. "
                          "Thanks to the great work of <b>Baka MPlayer</b>'s original developers. "
                          "Without their hard work, there won't be <b>SPlayer</b> anymore. "
                          "I really appreciate <b>godly-devotion</b> (Creator/UX Designer/Programmer), "
                          "<a href='https://github.com/u8sand'><b>u8sand</b></a> (Lead Programmer/Website Host)"
                          " and their team.</p>\n<p>Home Page: <a href='%1'>%2</a></p>")
                          .arg(QString::fromStdWString(SPLAYER_COMPANY_URL_STR))
                          .arg(QString::fromStdWString(SPLAYER_COMPANY_URL_STR));
    return text;
}

QString AboutDialog::aboutText_PlainText()
{
    return aboutText_HTML().remove(QRegExp(QStringLiteral("<[^>]*>")));
}

QString AboutDialog::updateLogText_HTML(QString lang)
{
    static QString text;
    QString l(lang);
    if (lang == QString::fromLatin1("auto") || lang == QString::fromLatin1("en_US")
            || lang == QString::fromLatin1("en_UK") || lang == QString::fromLatin1("C"))
    {
        l = QString::fromLatin1("en");
    }
    QString filePath = QString::fromLatin1(":/updatelogs/updatelog.") + l + QString::fromLatin1(".html");
    QFile licenseFile(filePath);
    if (licenseFile.exists())
    {
        if (licenseFile.open(QFile::ReadOnly | QFile::Text))
        {
            QTextStream ts(&licenseFile);
            text = ts.readAll();
            licenseFile.close();
            return text;
        }
    }
    return updateLogText_HTML(QString::fromLatin1("en"));
}

QString AboutDialog::updateLogText_PlainText()
{
    return updateLogText_HTML().remove(QRegExp(QStringLiteral("<[^>]*>")));
}

QString AboutDialog::creditsText_HTML()
{
    static QString text = tr("<h2>mpv - video player based on MPlayer/mplayer2</h2>\n<h3>Contributors</h3>\n"
                             "<p>pigoz, haasn, kevmitch, rossy, wiiaboo, divVerent, giselher, lachs0r, "
                             "Kovensky, Akemi, Argon-, ChrisK2, ghedo, mathstuf, olifre, xylosper, ubitux, "
                             "atomnuker, qmega, tmm1, bjin, philipl, pavelxdd, shdown, avih, thebombzen, rr-, "
                             "dubhater, rrooij, torque, jeeb, sfan5, igv, grigorig, frau, jon-y, TimothyGu, "
                             "maniak1349, richardpl, Nyx0uf, czarkoff, Coacher, qyot27, Cloudef, linkmauve, "
                             "DanOscarsson, CounterPillow, henry0312, marcan, fhvwy, jaimeMF, SirCmpwn, xantoz, "
                             "rcombs, Nikoli, percontation, AoD314, otommod, ahodesuka, LongChair, CyberShadow, "
                             "michaelforney, sCreami, medhefgo, MadFishTheOne, rozhuk-im, shinchiro, TheAMM, "
                             "Gusar321, elenril, wrl, mixi, Themaister, lu-zero, pa4, markun, maletor, quilloss, "
                             "PombeirP, mikaoP, ricardomv, wsldankers, ion1, chneukirchen, ncopa, agiz, "
                             "viveksjain, hroncok, andre-d, Bilalh, jozzse, elevengu, MoSal, foo86, ryanmjacobs, "
                             "vitorgalvao, Shudouken, zekica, c-14, eworm-de</p>\n"
                             "<h2>Baka MPlayer</h2>\n<h3>Contributors</h3>\n"
                             "<p>u8sand, godly-devotion, amazingfate, epitron, AlfredoRamos, jbeich, rrooij, "
                             "samdx, ErikDavison, muzena, FrankHB, stryaponoff, starks, theChaosCoder, luigino, "
                             "tehcereal, redranamber, wb9688, arabuli, jqs7, Reboare, yiip87, suhr</p>\n"
                             "<h3>Material Design icons</h3>\n<p>Google</p>\n"
                             "<h3>Noto Sans fonts</h3>\n<p>Google</p>\n"
                             "<h3>Retro Cassette image</h3>\n<p>Lukas Troup</p>\n"
                             "<h3>Gesture icons</h3>\n<p>Jeff Portaro</p>\n"
                             "<h3>Download icon</h3>\n<p>Sasha Mescheryakov</p>\n"
                             "<h3>Translations</h3>\n<h4>Chinese</h4>\n<p>amazingfate, Antares95</p>\n"
                             "<h4>Croatian</h4>\n<p>gogo</p>\n<h4>Dutch</h4>\n<p>robin007bond, wb9688</p>\n"
                             "<h4>French</h4>\n<p>chapouvalpin</p>\n<h4>Georgian</h4>\n<p>arabuli</p>\n"
                             "<h4>German</h4>\n<p>yiip87</p>\n<h4>Italian</h4>\n<p>Aloysius</p>\n"
                             "<h4>Korean</h4>\n<p>godly-devotion</p>\n<h4>Portuguese</h4>\n<p>u8sand</p>\n"
                             "<h4>Russian</h4>\n<p>suhr</p>\n<h4>Spanish</h4>\n<p>Alfredo Ramos</p>\n"
                             "<h2>SPlayer</h2>\n<h3>Contributors</h3>\n<p>wangwenx190</p>\n"
                             "<h3>SPlayer icon</h3>\n<p>ninja emotion-icons <a href='https://roundicons.com/'>"
                             "roundicons</a></p>\n<h3>Translations</h3>\n<h4>Simplified Chinese</h4>\n"
                             "<p>amazingfate, Antares95, wangwenx190</p>");
    return text;
}

QString AboutDialog::creditsText_PlainText()
{
    return creditsText_HTML().remove(QRegExp(QStringLiteral("<[^>]*>")));
}

QString AboutDialog::licenseText_HTML(QString lang)
{
    static QString text;
    QString l(lang);
    if (lang == QString::fromLatin1("auto") || lang == QString::fromLatin1("en_US")
            || lang == QString::fromLatin1("en_UK") || lang == QString::fromLatin1("C"))
    {
        l = QString::fromLatin1("en");
    }
    QString filePath = QString::fromLatin1(":/licenses/gpl-3.0.") + l + QString::fromLatin1(".html");
    QFile licenseFile(filePath);
    if (licenseFile.exists())
    {
        if (licenseFile.open(QFile::ReadOnly | QFile::Text))
        {
            QTextStream ts(&licenseFile);
            text = ts.readAll();
            licenseFile.close();
            return text;
        }
    }
    return licenseText_HTML(QString::fromLatin1("en"));
}

QString AboutDialog::licenseText_PlainText()
{
    return licenseText_HTML().remove(QRegExp(QStringLiteral("<[^>]*>")));
}
