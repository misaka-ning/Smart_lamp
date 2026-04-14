#ifndef IMAGE_LOG_H
#define IMAGE_LOG_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace Ui {
class Image_log;
}

class Image_log : public QWidget
{
    Q_OBJECT

public:
    explicit Image_log(QWidget *parent = nullptr);
    ~Image_log();

private slots:
    void onImage1Clicked();
    void onImage2Clicked();

private:
    Ui::Image_log *ui;

    // 手动添加的控件
    QPushButton *buttonImage1;
    QPushButton *buttonImage2;
    QLabel *imageLabel;

    // 布局
    QVBoxLayout *buttonLayout;
    QHBoxLayout *mainLayout;
};

#endif // IMAGE_LOG_H
