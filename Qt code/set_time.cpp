#include "Set_time.h"
#include "ui_Set_time.h"

#include <QMessageBox>

Set_time::Set_time(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Set_time)
{
    ui->setupUi(this);

   connect(ui->ConfirmButton, &QPushButton::clicked, this, &Set_time::Confirm_time);
}

Set_time::~Set_time()
{
    delete ui;
}


void Set_time::Confirm_time()
{
    bool ok;
    int value = ui->drinkEdit->text().toInt(&ok);
    if (ok) {
        drink_remind_time = value;
    } else {
        QMessageBox::warning(this, tr("输入错误"), tr("请输入有效的数字"));
    }
    this->close();
}
