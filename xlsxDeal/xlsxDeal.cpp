#include "xlsxDeal.h"
#include "xlsxdocument.h"
#include <QtCore/qmath.h>

xlsxDeal::xlsxDeal()
{

}

//获取目录下的文件名
QStringList xlsxDeal::GetFileNameList(const QString &strDirpath)
{
    QDir dir(strDirpath);
    QFileInfoList Info_list = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    QListIterator<QFileInfo> Iterator(Info_list);
    QStringList strFileNameList;
    QFileInfo Info;
    while (Iterator.hasNext())
    {
        Info = Iterator.next();
        if (Info.isFile())
        {
            strFileNameList << Info.fileName();
        }
    }
    return strFileNameList;
}

void xlsxDeal::showFileName()
{
    QStringList strFileNameList;
    // 获取当前目录下的文件
    this->fileName->clear();
    strFileNameList = GetFileNameList("./");
    //this->fileName->addItems(strFileNameList);
    int itemCount = strFileNameList.count();
    // 将带.xlsx的文件名字添加到QListWidget
    for (int row = 0; row < itemCount; row++) {
        QString tmp = strFileNameList.at(row);
        if (tmp.contains(".xlsx",Qt::CaseSensitive))
            this->fileName->addItem(tmp);
    }
}

void xlsxDeal::onRenovateListButtonClicked()
{
    this->showFileName();
}

void xlsxDeal::onTotalLineSpinBoxValueChanged()
{
    int lineCounts;

    lineCounts = cols->value();
    for (int i=0; i<4; i++) {
        col[i]->setEnabled(i<lineCounts);
    }
}

void xlsxDeal::onFileItemDoubleClicked(QListWidgetItem* item)
{
    QString strItemName = item->text(); // 获取双击的文件的名字

    // 判断文件名中有.xlsx
    if (!strItemName.contains(".xlsx",Qt::CaseSensitive)) return;

    int readColsCount = this->cols->value(); // 获取要读的总列

    // 获取输入的列号
    int readCol[4];
    readCol[0] = this->col[0]->text().toInt();
    readCol[1] = this->col[1]->text().toInt();
    readCol[2] = this->col[2]->text().toInt();
    readCol[3] = this->col[3]->text().toInt();

    int readRow = 4; // 从第三行开始读取

    double lastValueOfCol1 = 0;
    QVariant currentReadValue[4];
    double currentReadValueToDoule[4] = {0};
    double recordValue = 0;
    int sameValueCount = 0;
    int writeCount = 0;

    QXlsx::Document dataXlsx(strItemName); // 打开原始表格
    QXlsx::Document newXlsx;
    newXlsx.addSheet("数据");

    while (!dataXlsx.read(readRow, readCol[0]).isNull()) {
        // 读取第一列数据，转为double类型
        currentReadValue[0] = dataXlsx.read(readRow, readCol[0]);

        if (currentReadValue[0].type() != QVariant::Type::Double
                && currentReadValue[0].type() != QVariant::Type::Int
                && currentReadValue[0].type() != QVariant::Type::Bool)
            return;

        currentReadValueToDoule[0] = currentReadValue[0].toDouble();
        // 判断当前数组较上一次记录的数值变化超过100
        if (fabs(currentReadValueToDoule[0] - lastValueOfCol1) > 100) {
            // 判断相同数值的个数
            if (fabs(currentReadValueToDoule[0] - recordValue) < 3) sameValueCount++;
            else recordValue = currentReadValueToDoule[0];
        }
        // 若相同数值次数超过5，认为数据平缓，开始写入表格
        if (sameValueCount > 5) {
            for (int i=0; i<readColsCount; i++) {
                currentReadValue[i] = dataXlsx.read(readRow, readCol[i]);
                currentReadValueToDoule[i] = currentReadValue[i].toDouble();
                newXlsx.write(writeCount+1, i+1, currentReadValueToDoule[i]);
            }
            writeCount++;
            lastValueOfCol1 = currentReadValueToDoule[0];
            sameValueCount = 0;
        }

        // 读取下一行
        ++readRow;
    }

    newXlsx.saveAs("new_" + strItemName);
    this->showFileName();
}

