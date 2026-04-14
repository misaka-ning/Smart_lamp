#include "smart_lamp.h"
#include <QSlider>
#include <QDialog>
#include <QVBoxLayout>
#include "./ui_smart_lamp.h"

#include <QFile>
#include <QKeyEvent>
#include <QApplication>
#include <QMouseEvent>
#include <QDebug>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QInputDialog>
#include <QMessageBox>
#include <QSerialPortInfo>

Smart_lamp::Smart_lamp(QWidget *parent)
    : QLabel(parent)
    , ui(new Ui::Smart_lamp)
    , is_dragging(false)
    , isLightOn(Wait_connection)    // 添加灯光状态初始化
    , Else_window(nullptr)
    , set_time(nullptr)
    , Seialpart(nullptr)            // 初始化为nullptr
    , drinkTimer(new QTimer(this))  // 初始化饮水计时器
    , eyesTimer(new QTimer(this))   // 初始化久坐计时器
{
    // 初始化窗口指针
    ui->setupUi(this);
    Else_window = new else_window;

    set_time = new Set_time(this);

    serialPort = new QSerialPort(this);

    image_log = new Image_log;

    // 设置窗口关闭后自动释放
    // Else_window->setAttribute(Qt::WA_DeleteOnClose);
    // set_time->setAttribute(Qt::WA_DeleteOnClose);

    // 初始化按钮文本
    ui->openButton->setText("连接台灯");

    // 设置应用程序图标
    setWindowIcon(QIcon(":/assets/misaka.png"));

    connect(ui->openButton , &QPushButton::clicked, this, &Smart_lamp::toggleLight);       // 关联开关灯按钮
    connect(ui->closeButton, &QPushButton::clicked, this, &Smart_lamp::closeWindow);       // 关联关闭按钮
    connect(ui->minimizeButton, &QPushButton::clicked, this, &Smart_lamp::minimizeWindow); // 关联最小化按钮
    connect(ui->elseButton, &QPushButton::clicked, this, &Smart_lamp::else_window_open);   // 关联其他窗口按钮
    connect(ui->setTimeButton, &QPushButton::clicked, this, &Smart_lamp::set_time_window_open);   // 关联设置时间按钮
    connect(ui->brightnessButton, &QPushButton::clicked, this, &Smart_lamp::on_brightnessButton_clicked); // 关联设置亮度按钮
    connect(ui->todoButton, &QPushButton::clicked, this, &Smart_lamp::on_todoButton_clicked);
    connect(ui->FocusButton, &QPushButton::clicked, this, &Smart_lamp::Focus_window_open);
    // connect(ui->logButton, &QPushButton::clicked, this, &Smart_lamp::serial_window_open);
    connect(ui->logButton, &QPushButton::clicked, this, &Smart_lamp::image_log_window_open);
    connect(serialPort, &QSerialPort::readyRead, this, &Smart_lamp::serilaPortReadyRead_Slot);
    connect(drinkTimer, &QTimer::timeout, this, &Smart_lamp::handleDrinkReminder);
    connect(eyesTimer, &QTimer::timeout, this, &Smart_lamp::handleEyesReminder);

    // 设置无边框窗口
    this->setWindowFlag(Qt::FramelessWindowHint);
    this->installEventFilter(this);

    // ===== 添加托盘系统初始化 =====
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/assets/misaka.png")); // 使用应用图标

    // 创建托盘菜单
    QMenu *trayMenu = new QMenu(this);
    restoreAction = new QAction(tr("恢复窗口"), this);
    quitAction = new QAction(tr("退出程序"), this);

    trayMenu->addAction(restoreAction);
    trayMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayMenu);

    // 连接托盘菜单信号
    connect(trayIcon, &QSystemTrayIcon::activated,this, &Smart_lamp::onTrayIconActivated);
    connect(restoreAction, &QAction::triggered,this, &Smart_lamp::restoreFromTray);
    connect(quitAction, &QAction::triggered,this, &Smart_lamp::quitApplication);

    // 加载样式表
    QFile file(":/assets/Smart_lamp.qss");
    if(file.open(QFile::OpenModeFlag::ReadOnly))
    {
        this->setStyleSheet(file.readAll());
    }

    // 设置窗口大小
    setFixedSize(380, 560);

    setRemindTimes(drink_remind_time, eyes_remind_time);

    updateButtonStates();
}

// 饮水提醒处理
void Smart_lamp::handleDrinkReminder()
{
    serialPort->write("@Open_Drink_Reminder&");
    serialPort->flush();  // 确保数据立即发送
    QMessageBox::information(this,tr("饮水提醒"),tr("您已经连续工作%1分钟，请起来喝杯水休息一下！").arg(drink_remind_time));

    // 重新开始计时
    drinkTimer->start(drink_remind_time * 60 * 1000);
}

// 久坐提醒处理
void Smart_lamp::handleEyesReminder()
{
    serialPort->write("@Open_Eys_Reminder&");
    serialPort->flush();  // 确保数据立即发送
    QMessageBox::information(this,tr("用眼提醒"),tr("您已经连续使用电脑%1分钟，请休息一下眼睛！").arg(eyes_remind_time));

    // 重新开始计时
    eyesTimer->start(eyes_remind_time * 60 * 1000);
}

// 设置提醒时间
void Smart_lamp::setRemindTimes(int drinkTime, int eyesTime)
{
    // 更新成员变量
    drink_remind_time = drinkTime;
    eyes_remind_time = eyesTime;

    drinkTimer->stop();
    eyesTimer->stop();

    if (drinkTime > 0) {
        drinkTimer->start(drinkTime * 60 * 1000); // 分钟转毫秒
    }

    if (eyesTime > 0) {
        eyesTimer->start(eyesTime * 60 * 1000);
    }

    qDebug() << "提醒时间已更新 - 喝水:" << drinkTime << "分钟, 休息:" << eyesTime << "分钟";
}

Smart_lamp::~Smart_lamp()
{
    delete ui;
    if (Seialpart) {
        delete Seialpart;
    }
    // 确保退出时隐藏托盘图标
    if (trayIcon) {
        trayIcon->hide();
    }
}

#if(qss_debug)
// 调试用，刷新样式表
void Smart_lamp::keyPressEvent(QKeyEvent *ev)
{
#if(0)
    if(ev->key() == Qt::Key_F5)
    {
        qDebug() << qApp->applicationDirPath() + "/../../assets/Smart_lamp.qss";
        QFile file(qApp->applicationDirPath() + "/../../assets/Smart_lamp.qss");
        if(file.open(QFile::OpenModeFlag::ReadOnly))
        {
            this->setStyleSheet(file.readAll());
        }
    }

    if(ev->key() == Qt::Key_4)
    {
        qDebug() << drink_remind_time << " " << eyes_remind_time;
    }
#endif
    return QLabel::keyPressEvent(ev);
}
#endif

// 重写移动窗口逻辑
bool Smart_lamp::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == this)
    {
        // 检查鼠标焦点
        if(event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseMove ||
            event->type() == QEvent::MouseButtonRelease)
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

            if(mouseEvent->type() == QEvent::MouseButtonPress &&
                mouseEvent->button() == Qt::LeftButton)
            {
                QPoint localPos = mouseEvent->pos();
                // 检查选中部分是否是标题栏部分
                if(localPos.y() <= 40 && !ui->closeButton->geometry().contains(localPos))
                {
                    is_dragging = true;
                    drag_offset = mouseEvent->globalPosition().toPoint() - this->pos();
                    return true;
                }
            }
            else if(mouseEvent->type() == QEvent::MouseMove && is_dragging)
            {
                this->move(mouseEvent->globalPosition().toPoint() - drag_offset);
                return true;
            }
            else if(mouseEvent->type() == QEvent::MouseButtonRelease &&
                     mouseEvent->button() == Qt::LeftButton)
            {
                is_dragging = false;
            }
        }
    }
    return QLabel::eventFilter(watched, event);
}

// 通过关闭按钮关闭窗口
void Smart_lamp::closeWindow()
{
    // 改为隐藏到托盘而不是关闭程序
    this->hide();
    trayIcon->show();
    trayIcon->showMessage(tr("智能台灯"),tr("程序已最小化到托盘"),QSystemTrayIcon::Information,2000);
}

// 通过最小化按钮最小化窗口
void Smart_lamp::minimizeWindow()
{
    this->showMinimized();
}

// 打开其他窗口
void Smart_lamp::else_window_open()
{
    Else_window->show();
}

// 打开显示捕获图片窗口
void Smart_lamp::image_log_window_open()
{
    image_log->show();
}

// 打开设置时间窗口
void Smart_lamp::set_time_window_open()
{
    if (!set_time) {
        set_time = new Set_time(this);
        set_time->setAttribute(Qt::WA_DeleteOnClose);
    }
    // 每次打开都重新连接信号
    connect(set_time, &Set_time::timesUpdated, this, &Smart_lamp::setRemindTimes);
    set_time->show();
}

// 开关灯函数实现
// 亮度调节对话框实现
void Smart_lamp::on_brightnessButton_clicked() {
    // 创建对话框
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("亮度调节");
    dialog->setAttribute(Qt::WA_DeleteOnClose); // 关闭时自动删除

    QVBoxLayout *layout = new QVBoxLayout();

    // 亮度显示标签
    QLabel *valueLabel = new QLabel(
        QString::number(m_brightness) + "%"  // 直接拼接字符串
        );
    valueLabel->setAlignment(Qt::AlignCenter);

    // 亮度滑块
    QSlider *slider = new QSlider(Qt::Horizontal);
    slider->setRange(0, 100);
    slider->setValue(m_brightness);

    // 实时更新标签文本
    connect(slider, &QSlider::valueChanged, [=](int val){
        valueLabel->setText(QString::number(val) + "%");
    });

    // 确定按钮
    QPushButton *btnOk = new QPushButton("确定");

    // 连接确定按钮的点击信号 - 修改点1：添加保存功能
    connect(btnOk, &QPushButton::clicked, [=]() {
        // 获取当前滑块值
        int selectedBrightness = slider->value();

        m_brightness = selectedBrightness;

        // 或者保存到配置文件
        // QSettings settings("MyCompany", "SmartLamp");
        // settings.setValue("brightness", selectedBrightness);

        // 调用更新亮度的槽函数
        // updateBrightness(selectedBrightness);

        // 关闭对话框（触发accept）
        dialog->accept();
    });

    // 布局组装
    layout->addWidget(valueLabel);
    layout->addWidget(slider);
    layout->addWidget(btnOk);
    dialog->setLayout(layout);

    // 显示模态对话框
    dialog->exec();
}

void Smart_lamp::on_todoButton_clicked()
{
    // 创建对话框
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("待办事项");
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setMinimumSize(300, 400);

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);

    // 待办事项列表
    QListWidget *todoListWidget = new QListWidget(dialog);

    // 使用成员变量初始化列表
    for (const auto& item : m_todoItems) {
        todoListWidget->addItem(QString("%1: %2").arg(item.first).arg(item.second));
    }

    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    // 添加按钮
    QPushButton *btnAdd = new QPushButton("添加", dialog);
    connect(btnAdd, &QPushButton::clicked, [=, this]() { // 捕获 this 指针
        bool ok;
        QString text = QInputDialog::getText(
            dialog,
            "添加待办事项",
            "请输入待办事项:",
            QLineEdit::Normal,
            "",
            &ok
            );

        if (ok && !text.isEmpty()) {
            // 使用下一个可用ID（从1开始）
            uint8_t newId = m_nextTodoId++;

            // 添加到数据结构
            m_todoItems.append(qMakePair(newId, text));

            // 显示在列表控件中
            todoListWidget->addItem(QString("%1: %2").arg(newId).arg(text));
        }
    });

    // 删除按钮
    QPushButton *btnRemove = new QPushButton("删除选中", dialog);
    connect(btnRemove, &QPushButton::clicked, [=, this]() { // 捕获 this 指针
        QList<QListWidgetItem*> selectedItems = todoListWidget->selectedItems();

        // 从后往前删除以避免索引问题
        for (int i = selectedItems.size() - 1; i >= 0; i--) {
            QListWidgetItem *item = selectedItems[i];
            int row = todoListWidget->row(item);

            // 从数据结构中删除
            if (row >= 0 && row < m_todoItems.size()) {
                m_todoItems.remove(row);
            }

            // 从界面中删除
            delete todoListWidget->takeItem(row);
        }
    });

    // 清空按钮
    QPushButton *btnClear = new QPushButton("清空所有", dialog);
    connect(btnClear, &QPushButton::clicked, [=, this]() { // 捕获 this 指针
        todoListWidget->clear();
        m_todoItems.clear();
        m_nextTodoId = 1; // 重置ID计数器
    });

    // 确定按钮 - 添加串口发送功能
    QPushButton *btnOk = new QPushButton("确定", dialog);
    connect(btnOk, &QPushButton::clicked, [=, this]() { // 捕获 this 指针
        // 1. 发送清空链表命令
        sendSerialCommand("Clear_Todo_list");

        // 2. 发送所有待办事项
        for (const auto& item : m_todoItems) {
            uint8_t id = item.first;
            QString task = item.second;

            // 构造命令字符串 (ID + 逗号 + 任务内容)
            QString command = QString("Add_New_Todo %1,%2").arg(id).arg(task);
            sendSerialCommand(command);
        }

        dialog->accept();
    });

    // 添加到按钮布局
    buttonLayout->addWidget(btnAdd);
    buttonLayout->addWidget(btnRemove);
    buttonLayout->addWidget(btnClear);
    buttonLayout->addWidget(btnOk);

    // 添加到主布局
    mainLayout->addWidget(todoListWidget);
    mainLayout->addLayout(buttonLayout);

    // 显示模态对话框
    dialog->exec();
}

void Smart_lamp::sendSerialCommand(const QString& command)
{
    if (!serialPort || !serialPort->isOpen()) {
        qWarning() << "Serial port not open!";
        return;
    }

    // 根据单片机协议构造完整命令
    // 格式: 帧头(0xAA) + 命令 + 帧尾(0x55)
    QByteArray data;
    data.append('@'); // HEADER
    data.append(command.toUtf8());
    data.append('&'); // FOOTER

    // 发送数据
    serialPort->write(data);
    serialPort->waitForBytesWritten(1000);

    qDebug() << "Sent command:" << command;
}

// 实现串口连接对话框
void Smart_lamp::showConnectionDialog()
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("连接台灯");
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *layout = new QVBoxLayout(dialog);

    // 串口选择下拉框
    QComboBox *comboBox = new QComboBox(dialog);
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        comboBox->addItem(info.portName());
    }

    // 连接按钮
    QPushButton *connectButton = new QPushButton("连接", dialog);

    layout->addWidget(new QLabel("选择串口:"));
    layout->addWidget(comboBox);
    layout->addWidget(connectButton);

    // 连接按钮点击事件
    connect(connectButton, &QPushButton::clicked, [=]() {
        QString portName = comboBox->currentText();
        serialPort->setPortName(portName);
        serialPort->setBaudRate(QSerialPort::Baud115200);
        serialPort->setDataBits(QSerialPort::Data8);
        serialPort->setParity(QSerialPort::NoParity);
        serialPort->setStopBits(QSerialPort::OneStop);
        serialPort->setFlowControl(QSerialPort::NoFlowControl);

        if (serialPort->open(QIODevice::ReadWrite)) {
            // 新增时间发送代码 - 开始
            QTime currentTime = QTime::currentTime();
            // 格式化为两位数的小时、分钟、秒
            QString timeStr = QString("@Set_Time %1:%2:%3&")
                                  .arg(currentTime.hour(), 2, 10, QLatin1Char('0'))
                                  .arg(currentTime.minute(), 2, 10, QLatin1Char('0'))
                                  .arg(currentTime.second(), 2, 10, QLatin1Char('0'));

            // 通过串口发送时间指令
            qDebug() << timeStr;
            serialPort->write(timeStr.toUtf8());
            serialPort->flush();  // 确保数据立即发送
            // 新增时间发送代码 - 结束

            QMessageBox::information(this, "连接成功", "已成功连接到台灯");
            serialPort->write("@Open_Ui_Clock&");
            serialPort->flush();  // 确保数据立即发送
            isLightOn = light_open; // 更新状态为开灯
            ui->openButton->setText("开灯");
            ui->connectLabel->setText("当前状态：已连接");

            updateButtonStates();

            dialog->accept();
        }
        else {
            QMessageBox::critical(this, "连接失败", "无法连接到台灯: " + serialPort->errorString());
            dialog->exec();
        }
    });

    dialog->exec();
}

void Smart_lamp::toggleLight()
{
    if(isLightOn == Wait_connection)
    {
        ui->openButton->setText("连接台灯");
        ui->lampPictureLabel->setPixmap(QPixmap(":/assets/lamp_off.png").scaled(ui->lampPictureLabel->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        showConnectionDialog();
        return;
    }
    else if(isLightOn == light_open)
    {
        ui->openButton->setText("关灯");
        ui->lampPictureLabel->setPixmap(QPixmap(":/assets/lamp_open.png").scaled(ui->lampPictureLabel->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        isLightOn = light_off;
        return;
    }
    else if(isLightOn == light_off)
    {
        ui->openButton->setText("开灯");
        ui->lampPictureLabel->setPixmap(QPixmap(":/assets/lamp_off.png").scaled(ui->lampPictureLabel->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        isLightOn = light_open;
        return;
    }
    updateButtonStates();
}

// 开关专注模式
void Smart_lamp::Focus_window_open()
{
    if(focus_flag)
    {
        QMessageBox::information(this, tr("专注模式"), tr("已关闭专注模式"));
        focus_flag = !focus_flag;
        serialPort->write("@Open_Ui_Clock&");
        serialPort->flush();  // 确保数据立即发送

    }
    else
    {
        QMessageBox::information(this, tr("专注模式"), tr("已打开专注模式"));
        focus_flag = !focus_flag;
        serialPort->write("@Open_Ui_Todo&");
        serialPort->flush();  // 确保数据立即发送
    }
}

// 修改串口数据接收函数
void Smart_lamp::serilaPortReadyRead_Slot()
{
    QByteArray data = serialPort->readAll();
    serial_buf.append(QString(data)); // 使用 append 而不是直接赋值

    // 发出信号通知数据已接收
    emit serialDataReceived(QString(data));
}


// 添加清空缓冲区实现
void Smart_lamp::clearSerialBuffer()
{
    serial_buf.clear();
}

// 串口调试窗口打开函数
void Smart_lamp::serial_window_open()
{
    if (!Seialpart) {
        // 创建窗口时传入串口指针
        Seialpart = new seialpart(serialPort, nullptr);
        Seialpart->setWindowTitle("串口调试");

        // 设置窗口位置在主窗口右侧
        QPoint newPos = this->pos() + QPoint(this->width() + 10, 0);
        Seialpart->move(newPos);

        // 连接清空信号
        connect(Seialpart, &seialpart::clearBufferRequested, this, &Smart_lamp::clearSerialBuffer);
    }

    // 连接数据接收信号
    connect(this, &Smart_lamp::serialDataReceived, Seialpart, &seialpart::appendReceivedData);

    // 显示当前已保存的数据
    Seialpart->appendReceivedData(serial_buf);

    // 确保窗口显示在最前面
    Seialpart->show();
    Seialpart->raise();
    Seialpart->activateWindow();
}

// 托盘图标激活处理
void Smart_lamp::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        restoreFromTray();
    }
}

// 从托盘恢复窗口
void Smart_lamp::restoreFromTray()
{
    this->show();
    this->activateWindow(); // 激活窗口
    trayIcon->hide(); // 可选：隐藏托盘图标
}

// 退出应用程序
void Smart_lamp::quitApplication()
{
    // 清理资源
    drinkTimer->stop();
    eyesTimer->stop();
    if (serialPort && serialPort->isOpen()) {
        serialPort->close();
    }

    trayIcon->hide(); // 隐藏托盘图标
    qApp->quit();    // 退出程序
}

// 重写关闭事件
void Smart_lamp::closeEvent(QCloseEvent *event)
{
    // 拦截关闭事件，改为最小化到托盘
    event->ignore();
    this->hide();
    trayIcon->show();
    trayIcon->showMessage(tr("智能台灯"),tr("程序已最小化到托盘"), QSystemTrayIcon::Information, 2000);
}

void Smart_lamp::updateButtonStates()
{
    bool isConnected = (isLightOn != Wait_connection);

    ui->brightnessButton->setEnabled(isConnected);
    ui->todoButton->setEnabled(isConnected);
    ui->FocusButton->setEnabled(isConnected);
    ui->logButton->setEnabled(isConnected);
}
