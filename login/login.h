#ifndef LOGIN_H
#define LOGIN_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class Login;
}
QT_END_NAMESPACE

class Login : public QMainWindow
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = 0);
    ~Login();

signals:
    void display(int number);

private slots:
    void on_pushButton_regist_clicked();
    void on_pushButton_login_clicked();

private slots:
    void initForm();
    void initStyle();

private:
    Ui::Login * ui;
    QString inputName;
};

#endif // LOGIN_H
