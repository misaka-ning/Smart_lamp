#ifndef IMAGEVIEWERWINDOW_H
#define IMAGEVIEWERWINDOW_H

#include <QDialog>
#include <QListWidget>
#include <QLabel>
#include <QDir>
#include <QResizeEvent>

class ImageViewerWindow : public QDialog
{
    Q_OBJECT
public:
    explicit ImageViewerWindow(const QString& folderPath, QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override; // 添加重写声明

private:
    void loadImagesFromFolder(const QString& folderPath);
    void displayImage(int index);

    QListWidget *imageListWidget;
    QLabel *imageLabel;
    QStringList imagePaths;
};

#endif // IMAGEVIEWERWINDOW_H
