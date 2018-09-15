#pragma once

#include <QDialog>

class MainWindow;

class DimDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DimDialog(MainWindow *window, QWidget *parent = nullptr);

    void show();
    bool close();

protected:
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void visbilityChanged(bool dim);

private:
    MainWindow *window;
};
