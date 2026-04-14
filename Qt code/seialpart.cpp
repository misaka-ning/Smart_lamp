#include "seialpart.h"
#include "ui_seialpart.h"
#include <QDebug>

seialpart::seialpart(QSerialPort *serialPort, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::seialpart)
    , m_serialPort(serialPort) // 初始化串口指针
{
    ui->setupUi(this);

    // 连接发送按钮的点击信号
    connect(ui->sendButton, &QPushButton::clicked, this, &seialpart::on_sendButton_clicked);
    connect(ui->cleanButton, &QPushButton::clicked, this, &seialpart::on_cleanButton_clicked);
}

seialpart::~seialpart()
{
    delete ui;
}

// 添加接收数据到文本框
void seialpart::appendReceivedData(const QString &data)
{
    ui->recvEdit->appendPlainText(data); // 使用append会添加新行
}

// 清空接收数据
void seialpart::clearReceivedData()
{
    ui->recvEdit->clear();
}

// 实现发送按钮点击事件
void seialpart::on_sendButton_clicked()
{
    if (!m_serialPort || !m_serialPort->isOpen()) {
        ui->recvEdit->appendPlainText("错误: 串口未连接");
        return;
    }

    QString text = ui->sendEdit->text();
    if (!text.isEmpty()) {
        // 发送数据
        // QByteArray data = text.toUtf8();
        m_serialPort->write(ui->sendEdit->text().toLocal8Bit().data());

        // 在接收框中显示发送的数据（可选）
        ui->recvEdit->appendPlainText("发送: " + text);

        // 清空发送框
        ui->sendEdit->clear();
    }
}

// 实现清除按钮点击事件
void seialpart::on_cleanButton_clicked()
{
    // 清空接收框
    clearReceivedData();

    // 发出清空缓冲区的信号
    emit clearBufferRequested();
}
