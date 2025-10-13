#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    setWindowTitle("登录");
    ui->nameEdit->setPlaceholderText("输入用户名");

    // 绑定按钮
    QDialogButtonBox *buttonBox = ui->LoginbuttonBox;

    // 确认按钮，尝试登录
    connect(buttonBox, &QDialogButtonBox::accepted, this, &Dialog::on_pushButton_clicked);

    // 取消按钮，直接退出
    connect(buttonBox, &QDialogButtonBox::rejected, this,
    [=](){
        this->close();
    });
}

void Dialog::on_pushButton_clicked()
{
    QString username = ui->nameEdit->text().trimmed();
    if (username.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入用户名！");
        this->show();
        return;
    }

    try {
        // 尝试发送消息（触发连接）
        Client::getInstance().SendMessage(username.toStdString());
        
        // 成功后打开主窗口
        emit openMainWindow(username);
        ui->nameEdit->clear();
        this->close();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "操作失败", QString("错误原因：%1").arg(e.what()));
        this->show();
    }
}
    

Dialog::~Dialog()
{
    delete ui;
}
