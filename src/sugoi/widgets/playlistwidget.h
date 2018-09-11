#ifndef PLAYLISTWIDGET_H
#define PLAYLISTWIDGET_H

#include <QListWidget>

class SugoiEngine;

class PlaylistWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit PlaylistWidget(QWidget *parent = nullptr);

    void AttachEngine(SugoiEngine *sugoi);

public slots:
    void Populate();
    void RefreshPlaylist();

    void SelectItem(const QString &item);
    QString CurrentItem();
    int CurrentIndex(); // index of the current playing file
    void SelectIndex(int index, bool relative = false); // relative to current selection
    void PlayIndex(int index, bool relative = false); // relative to current playing file
    void RemoveIndex(int index); // remove the selected item

    void Search(const QString&);
    void ShowAll(bool);
    void Shuffle();

protected slots:
    void BoldText(const QString &f, bool state);
    void RemoveFromPlaylist(QListWidgetItem *item);
    void DeleteFromDisk(QListWidgetItem *item);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    SugoiEngine *sugoi = nullptr;

    QStringList playlist;
    QString file, suffix;
    bool newPlaylist, refresh, showAll;
};

#endif // PLAYLISTWIDGET_H
