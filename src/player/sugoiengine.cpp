#include "sugoiengine.h"

#include <QMessageBox>
#include <QDir>
#include <QTranslator>

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
    overlay(new OverlayHandler(this))
{
    dimDialog = new DimDialog(window, nullptr);

    connect(mpv, &MpvWidget::messageSignal,
            [=](const QString& msg)
            {
                Print(msg, QStringLiteral("mpv"));
            });
}

SugoiEngine::~SugoiEngine()
{
    delete translator;
    delete qtTranslator;
    delete dimDialog;
    delete overlay;
}

void SugoiEngine::Command(const QString& command)
{
    if(command == QString())
        return;
    QStringList args = command.split(QStringLiteral(" "));
    if(!args.empty())
    {
        if(args.front() == QLatin1String("sugoi")) // implicitly understood
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
            RequiresParameters(QStringLiteral("sugoi"));
    }
    else
        InvalidCommand(args.join(' '));
}

void SugoiEngine::Print(const QString& what, const QString& who)
{
    QString out = QStringLiteral("[%0]: %1").arg(who, what);
    QTextStream r{stdout};
    (r << out).flush();
    window->ui->outputTextEdit->moveCursor(QTextCursor::End);
    window->ui->outputTextEdit->insertPlainText(out);
}

void SugoiEngine::PrintLn(const QString& what, const QString& who)
{
    Print(what+"\n", who);
}

void SugoiEngine::InvalidCommand(const QString& command)
{
    PrintLn(tr("invalid command '%0'").arg(command));
}

void SugoiEngine::InvalidParameter(const QString& parameter)
{
    PrintLn(tr("invalid parameter '%0'").arg(parameter));
}

void SugoiEngine::RequiresParameters(const QString& what)
{
    PrintLn(tr("'%0' requires parameters").arg(what));
}
