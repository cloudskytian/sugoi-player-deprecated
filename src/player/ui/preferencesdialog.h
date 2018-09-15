#pragma once

#include <QDialog>
#include <QPair>
#include <QMutex>

class QTableWidget;

namespace Ui {
class PreferencesDialog;
}

class SugoiEngine;

class PreferencesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PreferencesDialog(SugoiEngine *sugoi, QWidget *parent = nullptr);
    ~PreferencesDialog() override;

    static void showPreferences(SugoiEngine *sugoi, QWidget *parent = nullptr);

protected:
    void PopulateSkinFiles();
    void PopulateLangs();
    void PopulateMsgLvls();
    void PopulateShortcuts();
    void AddRow(const QString& first, const QString& second, const QString& third);
    void ModifyRow(int row, const QString &first, const QString &second, const QString &third);
    void RemoveRow(int row);
    void SelectKey(bool add, QPair<QString, QPair<QString, QString>> init = (QPair<QString, QPair<QString, QString>>()));

private:
    Ui::PreferencesDialog *ui;
    SugoiEngine *sugoi;
    QHash<QString, QPair<QString, QString>> saved;

    QString screenshotDir;
    int numberOfShortcuts;

    class SortLock : public QMutex
    {
    public:
        SortLock(QTableWidget *parent);

        void lock();
        void unlock();
    private:
        QTableWidget *parent;
    } *sortLock;
};
