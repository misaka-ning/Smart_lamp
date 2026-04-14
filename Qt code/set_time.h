#ifndef SET_TIME_H
#define SET_TIME_H

#include <QWidget>

namespace Ui {
class Set_time;
}

class Set_time : public QWidget
{
    Q_OBJECT

public:
    int drink_remind_time; // 新增成员变量声明

    explicit Set_time(SmartLamp *smartLamp, QWidget *parent = nullptr);
    SmartLamp *m_smartLamp;
    ~Set_time();

    void Confirm_time();

private:
    Ui::Set_time *ui;
};

#endif // SET_TIME_H
