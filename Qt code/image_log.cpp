#include "image_log.h"
#include "ui_image_log.h"

Image_log::Image_log(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Image_log)
{
    ui->setupUi(this);

    // 创建按钮
    buttonImage1 = new QPushButton("捕获图片1", this);
    buttonImage2 = new QPushButton("捕获图片2", this);

    // 创建图片标签
    imageLabel = new QLabel(this);

    // 设置按钮布局（垂直）
    buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget(buttonImage1);
    buttonLayout->addWidget(buttonImage2);
    buttonLayout->addStretch(); // 添加弹性空间使按钮居上

    // 设置主布局（水平）
    mainLayout = new QHBoxLayout(this);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(imageLabel);
    mainLayout->setStretchFactor(buttonLayout, 1);
    mainLayout->setStretchFactor(imageLabel, 3);

    // 连接信号槽
    connect(buttonImage1, &QPushButton::clicked, this, &Image_log::onImage1Clicked);
    connect(buttonImage2, &QPushButton::clicked, this, &Image_log::onImage2Clicked);

    // 设置窗口属性
    setWindowTitle("查看捕获图片");
    resize(400, 300);
}

Image_log::~Image_log()
{
    delete ui;
}

void Image_log::onImage1Clicked()
{
    QPixmap pixmap(":/assets/buhuo1.png");
    if(!pixmap.isNull()) {imageLabel->setPixmap(pixmap.scaled(imageLabel->width(), imageLabel->height(),Qt::KeepAspectRatio, Qt::SmoothTransformation));}
}

void Image_log::onImage2Clicked()
{
    QPixmap pixmap(":/assets/buhuo2.png");
    if(!pixmap.isNull()) {imageLabel->setPixmap(pixmap.scaled(imageLabel->width(), imageLabel->height(),Qt::KeepAspectRatio, Qt::SmoothTransformation));}
}
