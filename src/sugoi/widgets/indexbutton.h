#ifndef INDEXBUTTON_H
#define INDEXBUTTON_H

#include <QPushButton>

class IndexButton : public QPushButton
{
    Q_OBJECT
public:
    explicit IndexButton(QWidget *parent = 0);

    int getIndex() const;

public slots:
    void setIndex(int index);

protected:
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent* event);

private:
    int index;
};

#endif // INDEXBUTTON_H
