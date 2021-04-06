#ifndef XLSXDEAL_H
#define XLSXDEAL_H

#include <QAbstractItemModel>
#include <QSpinBox>
#include <QLineEdit>
#include <QDir>
#include <QPushButton>
#include <QListWidget>
#include <QMessageBox>
//#include <xlsxdocument.h>
#include <QDebug>

class xlsxDeal :public QObject
{
    Q_OBJECT

public:
    xlsxDeal();
    QStringList GetFileNameList(const QString &strDirpath);
    void showFileName();
    void dealXlsx();

public:
    QSpinBox* cols;
    QLineEdit* col[4];
    QPushButton* renovateListButton;
    QListWidget * fileName;

public slots:
    void onTotalLineSpinBoxValueChanged();
    void onRenovateListButtonClicked();
    void onFileItemDoubleClicked(QListWidgetItem* item);

private:

};

#endif // XLSXDEAL_H
