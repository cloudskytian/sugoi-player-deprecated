#include "aboutdialog.h"
#include "ui_aboutdialog.h"

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
    return QStringLiteral("<p><b>%1</b>: %2<br /><b>%3</b>: %4<br /><b>%5</b>: %6<br /><b>%7</b>: %8 %9</p>")
            .arg(tr("Version")).arg(QStringLiteral(SUGOI_VERSION))
            .arg(tr("Architecture")).arg(QSysInfo::buildCpuArchitecture())
            .arg(tr("Qt version")).arg(QStringLiteral(QT_VERSION_STR))
            .arg(tr("Build time")).arg(QStringLiteral(__DATE__)).arg(QStringLiteral(__TIME__));
}

QString AboutDialog::compilerText_PlainText()
{
    return compilerText_HTML().remove(QRegExp(QStringLiteral("<[^>]*>")));
}

QString AboutDialog::aboutText_HTML()
{
    return tr("<p><b>Sugoi Player</b> is a free multimedia player based on "
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
              " and their team.</p>");
}

QString AboutDialog::aboutText_PlainText()
{
    return aboutText_HTML().remove(QRegExp(QStringLiteral("<[^>]*>")));
}
