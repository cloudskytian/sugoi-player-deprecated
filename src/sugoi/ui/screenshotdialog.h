#ifndef SCREENSHOTDIALOG_H
#define SCREENSHOTDIALOG_H

#include <QDialog>

class MpvWidget;

namespace Ui {
class ScreenshotDialog;
}

class ScreenshotDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScreenshotDialog(bool &always, bool &screenshot, MpvWidget *mpv, QWidget *parent = nullptr);
    ~ScreenshotDialog() override;

    static int showScreenshotDialog(bool &always, bool &screenshot, MpvWidget *mpv, QWidget *parent = nullptr);
private:
    Ui::ScreenshotDialog *ui;
    bool &always,
         &screenshot;
};

#endif // SCREENSHOTDIALOG_H
