#pragma once

#include <QSplitter>

class CustomSplitter : public QSplitter
{
    Q_OBJECT
public:
    explicit CustomSplitter(QWidget *parent = nullptr);

    int position() const;
    int normalPosition() const;
    int max() const;

public slots:
    void setPosition(int pos);
    void setNormalPosition(int pos);

signals:
    void positionChanged(int pos);
    void entered();

private:
    int normalPos;
};
