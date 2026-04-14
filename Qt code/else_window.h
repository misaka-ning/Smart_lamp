#ifndef ELSE_WINDOW_H
#define ELSE_WINDOW_H

#include <QWidget>

namespace Ui {
class else_window;
}

class else_window : public QWidget
{
    Q_OBJECT

public:
    explicit else_window(QWidget *parent = nullptr);
    ~else_window();

private:
    Ui::else_window *ui;
};

#endif // ELSE_WINDOW_H
