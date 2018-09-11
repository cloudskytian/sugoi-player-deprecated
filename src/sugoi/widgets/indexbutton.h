#ifndef INDEXBUTTON_H
#define INDEXBUTTON_H

#include <QPushButton>

class IndexButton : public QPushButton
{
    Q_OBJECT
public:
    explicit IndexButton(QWidget *parent = nullptr);

    int getIndex() const;

public slots:
    void setIndex(int index);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    int index;
};

#endif // INDEXBUTTON_H
