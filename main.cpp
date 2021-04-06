#include "widget.h"
#include <QApplication>
#include <QTextCodec>
#include <QDesktopWidget>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

bool opendatabase();
void initDatabase();

int main(int argc, char *argv[])
{
    opendatabase();
    initDatabase();
    QApplication a(argc, argv);
    a.setFont(QFont("Microsoft Yahei", 9));

#if (QT_VERSION <= QT_VERSION_CHECK(5,0,0))
#if _MSC_VER
    QTextCodec *codec = QTextCodec::codecForName("gbk");
#else
    QTextCodec *codec = QTextCodec::codecForName("utf-8");
#endif
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForTr(codec);
#else
    QTextCodec *codec = QTextCodec::codecForName("utf-8");
    QTextCodec::setCodecForLocale(codec);
#endif

    Widget w;
    w.setFixedSize(600, 500); // 窗口大小固定
    w.show();

    return a.exec();
}

bool opendatabase()
{
    qDebug() << QSqlDatabase::drivers();
    // QSqlDatabase mydb=QSqlDatabase::addDatabase("QSQLITE");
    QSqlDatabase mydb=QSqlDatabase::addDatabase("SQLITECIPHER");    // 利用插件加密
    //mydb.setDatabaseName("D:\\sqlite\\student.db");//平时debug正常用
    mydb.setDatabaseName("./account.db");//release用
    mydb.setUserName("admin");
    mydb.setPassword("admin");
    //mydb.setConnectOptions("QSQLITE_CREATE_KEY"); // 连接设置：创建密码

    if(mydb.open())
    {
        qDebug()<<"open success";
        return true;
    }
    else
    {
        qDebug()<<"open failed";
        return false;
    }
}

void initDatabase()
{
    QSqlQuery query;
    query.exec(QString("SELECT count(*) FROM sqlite_master WHERE type='table' AND name='user'"));    //关键的判断
    if (query.next())
    {
        if(query.value(0).toInt()==0)
        {
            // 表不存在
            qDebug() << "table does not exist, creating...";

            if (!query.exec("CREATE TABLE user ("
                            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "name VARCHAR,"
                            "password VARCHAR)")) {
                QMessageBox::critical(0, QObject::tr("Database Error"),
                                      query.lastError().text());
            }
        } else {
            //表存在
            qDebug() << "table exists";
        }

    }

}
