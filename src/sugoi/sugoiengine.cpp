#include "sugoiengine.h"

#include <QMessageBox>
#include <QDir>

#include "ui/mainwindow.h"
#include "ui_mainwindow.h"
#include "mpvwidget.h"
#include "overlayhandler.h"
#include "widgets/dimdialog.h"
#include "util.h"

SugoiEngine::SugoiEngine(QObject *parent):
    QObject(parent),
    window(static_cast<MainWindow*>(parent)),
    mpv(window->ui->mpvFrame),
    overlay(new OverlayHandler(this)),
    sysTrayIcon(new QSystemTrayIcon(QIcon(":/images/player.svg"), this)),
    translator(nullptr)
{
    if(Util::DimLightsSupported())
        dimDialog = new DimDialog(window, nullptr);
    else
    {
        dimDialog = nullptr;
        window->ui->action_Dim_Lights->setEnabled(false);
    }

    connect(mpv, &MpvWidget::messageSignal,
            [=](QString msg)
            {
                Print(msg, "mpv");
            });
}

SugoiEngine::~SugoiEngine()
{
    if(translator != nullptr)
        delete translator;
    if(dimDialog != nullptr)
        delete dimDialog;
    delete sysTrayIcon;
    delete overlay;
    //delete mpv;
}

void SugoiEngine::Command(QString command)
{
    if(command == QString())
        return;
    QStringList args = command.split(" ");
    if(!args.empty())
    {
        if(args.front() == "sugoi") // implicitly understood
            args.pop_front();

        if(!args.empty())
        {
            auto iter = SugoiCommandMap.find(args.front());
            if(iter != SugoiCommandMap.end())
            {
                args.pop_front();
                (this->*(iter->first))(args); // execute command
            }
            else
                InvalidCommand(args.join(' '));
        }
        else
            RequiresParameters("sugoi");
    }
    else
        InvalidCommand(args.join(' '));
}

void SugoiEngine::Print(QString what, QString who)
{
    QString out = QString("[%0]: %1").arg(who, what);
    (qStdout() << out).flush();
    window->ui->outputTextEdit->moveCursor(QTextCursor::End);
    window->ui->outputTextEdit->insertPlainText(out);
}

void SugoiEngine::PrintLn(QString what, QString who)
{
    Print(what+"\n", who);
}

void SugoiEngine::InvalidCommand(QString command)
{
    PrintLn(tr("invalid command '%0'").arg(command));
}

void SugoiEngine::InvalidParameter(QString parameter)
{
    PrintLn(tr("invalid parameter '%0'").arg(parameter));
}

void SugoiEngine::RequiresParameters(QString what)
{
    PrintLn(tr("'%0' requires parameters").arg(what));
}
