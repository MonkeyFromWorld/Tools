#include "common.h"
#include "login.h"
#include <QPushButton>
#include <QFile>
#include <QTextStream>
#include <QSqlQueryModel>
#include <QDebug>
#include <QMessageBox>
#include <QPainter>
#include "ui_login.h"

Login::Login(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);
    initForm();

    // on_pushButton_login_clicked 这种写法的函数名可以不写connect, 否则会触发俩次;
    //connect(ui->pushButton_login, &QPushButton::clicked, this, &Login::on_pushButton_login_clicked);
    //connect(ui->pushButton_regist, &QPushButton::clicked, this, &Login::on_pushButton_regist_clicked);
}

Login::~Login()
{
    delete ui;
}

void Login::on_pushButton_login_clicked()
{
    QString sql1,sql2;
    QString sql3;

    QString inputName=ui->lineEdit_userName->text();
    QString inputPassword=ui->lineEdit_password->text();
    QSqlQueryModel *model1=new QSqlQueryModel;
    QSqlQueryModel *model2=new QSqlQueryModel;
    sql1="select count (*) from user where name='"+inputName+"'";
    model1->setQuery(sql1);
    QModelIndex index1=model1->index(0,0);
    sql2="select password from user where name ='"+inputName+"'";
    model2->setQuery(sql2);
    QModelIndex index2=model2->index(0,0);

    qDebug() << "1:" << index1.data();
    qDebug() << "2:" << index2.data();
    if(index1.data()!=0 && inputPassword==index2.data())
    {
        //QMessageBox::about(NULL,"提示","用户登录成功");
        //sql3="insert into user(name,password) values('"+inputName+"','"+inputPassword+"')";
        //QSqlQueryModel *model=new QSqlQueryModel;
        //model->setQuery(sql3);
        emit display(NAVIGATION_PAGE);
    }
    else
    {
        QMessageBox::about(NULL, "提示", "用户名或密码错误");
    }

}

void Login::on_pushButton_regist_clicked()
{
    QString registCode = ui->lineEdit_registCode->text();
    if (registCode != "1221") {
        QMessageBox::about(NULL,"提示","注册码错误");
    } else if (ui->lineEdit_userName->text().length() == 0) {
        QMessageBox::about(NULL,"提示","用户名为空");
    } else if (ui->lineEdit_password->text().length() == 0) {
        QMessageBox::about(NULL,"提示","密码为空");
    } else {

        QString sql1,sql2;
        QString sql3;

        inputName=ui->lineEdit_userName->text();
        QString inputPassword=ui->lineEdit_password->text();
        QSqlQueryModel *model1=new QSqlQueryModel;
        QSqlQueryModel *model2=new QSqlQueryModel;
        sql1="select count (*) from user where name='"+inputName+"'";
        model1->setQuery(sql1);
        QModelIndex index1=model1->index(0,0);
        //sql2="select password from user where name ='"+inputName+"'";
        //model2->setQuery(sql2);
        //QModelIndex index2=model2->index(0,0);
        qDebug() << index1.data();
        if(index1.data()==0)
        {
            QMessageBox::about(NULL,"提示","用户注册成功");
            sql3="insert into user(name,password) values('"+inputName+"','"+inputPassword+"')";
            QSqlQueryModel *model=new QSqlQueryModel;
            model->setQuery(sql3);
        }
        else if(index1.data()!=0)
        {
            QMessageBox::about(NULL,"提示","用户已经存在");
        }
    }
}

void Login::initForm()
{
    this->initStyle();

    this->setAutoFillBackground(true);
    QPalette palette;
    QPixmap pixmap = QPixmap(":/images/moon.png").scaled(this->size());
    palette.setBrush(QPalette::Background, QBrush(pixmap));
    this->setPalette(palette);
}

void Login::initStyle()
{
    //加载样式表
    QString qss;
    QFile file(":/qss/login.qss");

    if (file.open(QFile::ReadOnly)) {
#if 1
        //用QTextStream读取样式文件不用区分文件编码 带bom也行
        QStringList list;
        QTextStream in(&file);
        //in.setCodec("utf-8");
        while (!in.atEnd()) {
            QString line;
            in >> line;
            list << line;
        }

        qss = list.join("\n");
#else
        //用readAll读取默认支持的是ANSI格式,如果不小心用creator打开编辑过了很可能打不开
        qss = QLatin1String(file.readAll());
#endif
        QString paletteColor = qss.mid(20, 7);
        this->setPalette(QPalette(QColor(paletteColor)));

        this->setStyleSheet(qss);   // 用qApp设置全局，可能会被其他全局覆盖，用this设置当前类
        file.close();
    }
}
