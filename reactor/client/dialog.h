#ifndef DIALOG_H
#define DIALOG_H

#include "client.h"
#include <QDialog>
#include <QMessageBox>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();

signals:
    void openMainWindow(QString name);

private:
    void on_pushButton_clicked();
    Ui::Dialog *ui;
};

#endif // DIALOG_H
