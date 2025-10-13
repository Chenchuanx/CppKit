#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QKeyEvent>
#include <QTimer>
#include <mutex>
#include "client.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void mainShow(QString name);

private:
    void keyPressEvent(QKeyEvent *event);
    void checkNewMessage();
    void sendMessage();

    Ui::MainWindow *ui;
    QStandardItemModel *msgModel;
    QString m_name;

    std::mutex m_mutex; // 保护msgModel, 防止同时修改
};
#endif // MAINWINDOW_H
