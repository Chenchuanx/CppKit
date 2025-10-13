#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , msgModel(new QStandardItemModel(this))
    , m_name()
{
    ui->setupUi(this);
    setWindowTitle("Epoll Chat");

    // 输入框提示
    ui->messageEdit->setPlaceholderText("输入消息, 按Ctrl+Enter发送");

    // 绑定消息列表
    ui->listView->setModel(msgModel);

    // 设置消息列表样式     
    ui->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);   // 不可编辑
    ui->listView->setSelectionMode(QAbstractItemView::NoSelection);     // 不可选中?

    // 发送按钮绑定
    connect(ui->sendButton, &QPushButton::clicked, this, &MainWindow::sendMessage);
}

// 定时检查新消息（由QTimer触发）
void MainWindow::checkNewMessage() {
    // 调用非阻塞接收函数
    std::string msg = Client::getInstance().RecvMessage();
    if (!msg.empty()) {
        // 收到他人消息，创建左对齐的项
        QStandardItem *otherItem = new QStandardItem(QString::fromStdString(msg));
        otherItem->setTextAlignment(Qt::AlignLeft | Qt::AlignTop); // 左对齐
        otherItem->setForeground(QBrush(QColor(255, 255, 255))); // 白色文本

        m_mutex.lock();
        msgModel->appendRow(otherItem); // 添加到模型
        ui->listView->scrollToBottom();
        m_mutex.unlock();
    }
}

// 发送消息
void MainWindow::sendMessage() {
    QString text = ui->messageEdit->toPlainText().trimmed();
    if (text.isEmpty()) return;

    // 创建自己的消息项（右对齐）
    QStandardItem *myItem = new QStandardItem(text);
    myItem->setTextAlignment(Qt::AlignRight | Qt::AlignTop); // 右对齐
    myItem->setForeground(QBrush(QColor(255, 255, 255))); // 白色文本

    m_mutex.lock();
    msgModel->appendRow(myItem);    // 添加到模型
    ui->listView->scrollToBottom();
    m_mutex.unlock();

    ui->messageEdit->clear();
    // 发送到服务器
    Client::getInstance().SendMessage(text.toStdString());
}
    
void MainWindow::mainShow(QString name) {
    // 初始化定时器, 每隔100ms检查一次是否有新消息
    QTimer *recvTimer = new QTimer(this);
    // 定时器触发时，调用接收消息的函数
    connect(recvTimer, &QTimer::timeout, this, &MainWindow::checkNewMessage);
    recvTimer->start(100); // 启动定时器（间隔100ms）
    
    m_name = name;
    this->show();
}

// Ctrl+Enter发送消息
void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Return && (event->modifiers() & Qt::ControlModifier)) {
        sendMessage();
        return;
    }
    QWidget::keyPressEvent(event);
}

MainWindow::~MainWindow() {
    delete ui;
}
