#include "imageviewerwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDir>
#include <QDebug>
#include <QPixmap>
#include <QApplication>
#include <QScreen>
#include <QResizeEvent>
#include <QFileInfo> // 添加包含

ImageViewerWindow::ImageViewerWindow(const QString& folderPath, QWidget *parent)
    : QDialog(parent)
{
    // 设置窗口标题和大小
    setWindowTitle("图片查看器");
    resize(800, 600);

    // 创建主布局
    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    // 创建左侧图片列表
    imageListWidget = new QListWidget(this);
    imageListWidget->setFixedWidth(200);
    mainLayout->addWidget(imageListWidget);

    // 创建右侧图片显示区域
    imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet("background-color: #f0f0f0; border: 1px solid #cccccc;");
    mainLayout->addWidget(imageLabel, 1); // 设置伸缩因子为1

    // 加载图片
    loadImagesFromFolder(folderPath);

    // 连接列表项点击信号
    connect(imageListWidget, &QListWidget::currentRowChanged,
            this, &ImageViewerWindow::displayImage);

    // 默认选择第一张图片
    if (!imagePaths.isEmpty()) {
        imageListWidget->setCurrentRow(0);
    }
}

void ImageViewerWindow::loadImagesFromFolder(const QString& folderPath)
{
    QDir directory(folderPath);
    if (!directory.exists()) {
        qWarning() << "文件夹不存在:" << folderPath;
        return;
    }

    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.gif";

    // 获取文件列表
    QStringList fileNames = directory.entryList(filters, QDir::Files);

    // 添加完整路径
    for (const QString &fileName : fileNames) {
        QString filePath = directory.filePath(fileName);
        imagePaths.append(filePath);
        imageListWidget->addItem(fileName); // 只显示文件名，不显示完整路径
    }

    qDebug() << "Loaded" << imagePaths.size() << "images from" << folderPath;
}

void ImageViewerWindow::displayImage(int index)
{
    if (index < 0 || index >= imagePaths.size()) {
        imageLabel->setText("没有可显示的图片");
        return;
    }

    // 使用QFileInfo确保路径正确
    QString imagePath = imagePaths[index];
    QPixmap pixmap(imagePath);

    if (pixmap.isNull()) {
        imageLabel->setText("无法加载图片: " + imagePath);
        qWarning() << "无法加载图片:" << imagePath;
        return;
    }

    // 调整图片大小以适应标签
    QSize labelSize = imageLabel->size();
    QPixmap scaledPixmap = pixmap.scaled(
        labelSize,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        );

    imageLabel->setPixmap(scaledPixmap);
}

// 正确实现 resizeEvent
void ImageViewerWindow::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    // 只在有图片时重新显示
    if (!imagePaths.isEmpty()) {
        displayImage(imageListWidget->currentRow());
    }
}
