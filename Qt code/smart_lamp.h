#ifndef SMART_LAMP_H
#define SMART_LAMP_H

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include "else_window.h"
#include "set_time.h"
#include "seialpart.h"
#include <QSerialPort>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QCloseEvent>
#include "imageviewerwindow.h"
#include "image_log.h"

#define qss_debug 1

QT_BEGIN_NAMESPACE
namespace Ui {
class Smart_lamp;
}
QT_END_NAMESPACE

enum light_open_state{
    Wait_connection = 0,
    light_open,
    light_off
};

class Smart_lamp : public QLabel
{
    Q_OBJECT

private:
    bool is_dragging;
    QPoint drag_offset;
    light_open_state isLightOn;
    int m_brightness = 50;
    bool focus_flag = false;

    QStringList m_todoList;

    void showConnectionDialog();

    QString serial_buf;

    QVector<QPair<uint8_t, QString>> m_todoItems; // 存储所有待办事项
    uint8_t m_nextTodoId = 1; // 下一个待办事项的ID（从1开始）

    QTimer *drinkTimer;  // 饮水提醒计时器
    QTimer *eyesTimer;   // 久坐提醒计时器

    QSystemTrayIcon *trayIcon;  // 系统托盘图标
    QAction *restoreAction;     // 恢复动作
    QAction *quitAction;        // 退出动作


public slots:
    void handleDrinkReminder();  // 饮水提醒处理
    void handleEyesReminder();   // 久坐提醒处理

    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);  // 托盘激活
    void restoreFromTray();  // 从托盘恢复
    void quitApplication();  // 退出程序

public:
    Smart_lamp(QWidget *parent = nullptr);
    ~Smart_lamp();

    QSerialPort *serialPort;

    void setRemindTimes(int drinkTime, int eyesTime);  // 设置提醒时间
    int drink_remind_time = 60;
    int eyes_remind_time = 120;

protected:
    void closeEvent(QCloseEvent *event) override;  // 重写关闭事件

#if(qss_debug)
    void keyPressEvent(QKeyEvent* ev) override;
#endif

    bool eventFilter(QObject *watched, QEvent *event) override;

    void closeWindow();

    void minimizeWindow();

    void toggleLight();

    void else_window_open();

    void on_brightnessButton_clicked();

    void on_todoButton_clicked();

    void set_time_window_open();

    void Focus_window_open();

    void serial_window_open();

    void serilaPortReadyRead_Slot();

    void clearSerialBuffer();

    void sendSerialCommand(const QString& command);

    void image_log_window_open();

    void updateButtonStates();

signals:
    void serialDataReceived(const QString &data); // 添加信号
    void timesUpdated(int drinkTime, int eyesTime);

private:
    Ui::Smart_lamp *ui;
    else_window *Else_window;
    Set_time *set_time;
    seialpart *Seialpart;
    Image_log *image_log;
};
#endif // SMART_LAMP_H
