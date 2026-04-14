#include "else_window.h"
#include "ui_else_window.h"

else_window::else_window(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::else_window)
{
    ui->setupUi(this);
    ui->competelabel->setPixmap(QPixmap(":/assets/compete.png").scaled(ui->competelabel->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

else_window::~else_window()
{
    delete ui;
}
