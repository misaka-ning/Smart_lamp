#ifndef SERIALPART_H
#define SERIALPART_H

#include <QWidget>
#include <QSerialPort>

namespace Ui {
class seialpart;
}

class seialpart : public QWidget
{
    Q_OBJECT

public:
    explicit seialpart(QSerialPort *serialPort, QWidget *parent = nullptr);
    ~seialpart();

    void appendReceivedData(const QString &data);
    void clearReceivedData(); // 清空方法

signals:
    void clearBufferRequested(); // 清空信号

private slots:
    void on_sendButton_clicked();
    void on_cleanButton_clicked(); // 清除按钮槽函数

private:
    Ui::seialpart *ui;
    QSerialPort *m_serialPort;
};

#endif // SERIALPART_H
