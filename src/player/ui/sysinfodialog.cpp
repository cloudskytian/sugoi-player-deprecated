#include "sysinfodialog.h"
#include "ui_sysinfodialog.h"

#include <QSysInfo>
#include <QClipboard>

SysInfoDialog::SysInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SysInfoDialog)
{
    ui->setupUi(this);
    ui->sysInfoText->setText(sysInfoText_HTML());

    connect(ui->OKButton, &QPushButton::clicked, this, &SysInfoDialog::close);
    connect(ui->copyButton, &QPushButton::clicked,
            [=]
            {
                QClipboard *clipboard = qApp->clipboard();
                clipboard->setText(sysInfoText_PlainText());
            });
}

SysInfoDialog::~SysInfoDialog()
{
    delete ui;
}

QString SysInfoDialog::sysInfoText_HTML()
{
    static QString text = QString::fromLatin1(
                "<p><b>%1</b>: %2<br /><b>%3</b>: %4<br /><b>%5</b>: %6<br />"
                "<b>%7</b>: %8<br /><b>%9</b>: %10<br /><b>%11</b>: %12<br />"
                "<b>%13</b>: %14<br /><b>%15</b>: %16<br /><b>%17</b>: %18<br />"
                "<b>%19</b>: %20</p>")
                .arg(tr("Build ABI")).arg(QSysInfo::buildAbi())
                .arg(tr("Build CPU Architecture")).arg(QSysInfo::buildCpuArchitecture())
                .arg(tr("Current CPU Architecture")).arg(QSysInfo::currentCpuArchitecture())
                .arg(tr("Kernel Type")).arg(QSysInfo::kernelType())
                .arg(tr("Kernel Version")).arg(QSysInfo::kernelVersion())
                .arg(tr("Machine Host Name")).arg(QSysInfo::machineHostName())
                .arg(tr("Pretty Product Name")).arg(QSysInfo::prettyProductName())
                .arg(tr("Product Type")).arg(QSysInfo::productType())
                .arg(tr("Product Version")).arg(QSysInfo::productVersion())
                .arg(tr("Windows Version")).arg(QSysInfo::windowsVersion());
    return text;
}

QString SysInfoDialog::sysInfoText_PlainText()
{
    return sysInfoText_HTML().remove(QRegExp(QStringLiteral("<[^>]*>")));
}
