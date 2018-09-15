#pragma once

#include <utility>

#include <QString>

struct Recent
{
    Recent(QString s = QString(), QString t = QString(), int p = 0):
        path(std::move(s)), title(std::move(t)), time(p) {}

    operator QString() const
    {
        return path;
    }

    bool operator==(const Recent &recent) const
    {
        return (path == recent.path);
    }

    QString path, title;
    int time;
};
