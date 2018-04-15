#include "ExcelExportBack.h"
#include <QDir>
#include <QVector>
#include <QDebug>
#include <QtXlsx/xlsxdocument.h>
#include <QDateTime>

namespace back{
    
    

    ExcelExportBack::ExcelExportBack(int year, const QString& gen, const QString& datapth, QObject *parent)
        :QThread(parent), genname(gen), monthpath(datapth),processyear(year){

    }

    ExcelExportBack::~ExcelExportBack(){

    }

    std::unique_ptr<std::future<bool>> ExcelExportBack::CreateMonthFuture(int month){
        return std::make_unique<std::future<bool>>(std::async(std::launch::async, [=]()->bool{
            DoEachMonth(month);

            return true;
        }));
    }

    void ExcelExportBack::run(){
        emit EmitBackLog(QString::fromLocal8Bit("开始处理机组%1的数据").arg(genname));

        std::unique_ptr<std::future<bool>> month1future=nullptr;
        std::unique_ptr<std::future<bool>> month2future=nullptr;
        std::unique_ptr<std::future<bool>> month3future = nullptr;
        std::unique_ptr<std::future<bool>> month4future = nullptr;
        std::unique_ptr<std::future<bool>> month5future = nullptr;
        std::unique_ptr<std::future<bool>> month6future = nullptr;
        std::unique_ptr<std::future<bool>> month7future = nullptr;
        std::unique_ptr<std::future<bool>> month8future = nullptr;
        std::unique_ptr<std::future<bool>> month9future = nullptr;
        std::unique_ptr<std::future<bool>> month10future = nullptr;
        std::unique_ptr<std::future<bool>> month11future = nullptr;
        std::unique_ptr<std::future<bool>> month12future = nullptr;

        month1future = CreateMonthFuture(1);
        month2future = CreateMonthFuture(2);
        month3future = CreateMonthFuture(3);
        month4future = CreateMonthFuture(4);
        month5future = CreateMonthFuture(5);
        month6future = CreateMonthFuture(6);
        month7future = CreateMonthFuture(7);
        month8future = CreateMonthFuture(8);
        month9future = CreateMonthFuture(9);
        month10future = CreateMonthFuture(10);
        month11future = CreateMonthFuture(11);
        month12future = CreateMonthFuture(12);

        while (true){
            if (month1future->_Is_ready() &&
                month2future->_Is_ready() &&
                month3future->_Is_ready() &&
                month4future->_Is_ready() &&
                month5future->_Is_ready() &&
                month6future->_Is_ready() &&
                month7future->_Is_ready() &&
                month8future->_Is_ready() &&
                month9future->_Is_ready() &&
                month10future->_Is_ready() &&
                month11future->_Is_ready() &&
                month12future->_Is_ready()){
                break;
            }
            else{

                emit EmitBackLog(QString::fromLocal8Bit("机组%1的数据正在读取中...").arg(genname));
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }

        //按月读取完毕输出
        emit EmitBackLog(QString::fromLocal8Bit("机组%1的数据读取结束,开始输入到文件").arg(genname));
        QFile file(monthpath+"/"+QString("%1.csv").arg(genname));
        if (file.open(QIODevice::WriteOnly)){
            QTextStream stream(&file);
            stream << 
                QString::fromLocal8Bit("时间") << "," 
                << QString::fromLocal8Bit("实测风速") << "," 
                << QString::fromLocal8Bit("实测功率") << "," 
                << QString::fromLocal8Bit("实测浆距角") << "\r\n";

            for (int month = 1; month <= 12; month++){
                DayMap daydata = monthdata.value(month);
                emit EmitBackLog(QString::fromLocal8Bit("输出机组%1%2月的数据").arg(genname).arg(month));
                for (int day = 1; day <= daydata.size(); day++){
                    if (!daydata.contains(day)){
                        throw std::runtime_error("逻辑有问题啊，不然怎么会没有呢");
                    }
                    QList<CellData> hourdata = daydata.value(day);
                    for (const CellData& cur : hourdata){
                        stream << cur.Time << ","
                            << cur.shicefengsu << ","
                            << cur.shicegonglv << ","
                            << cur.shicejiangjujiao << "\r\n";
                    }
                }
            }
        }
        else{
            throw std::runtime_error("不能输出到文件");
        }
        emit EmitBackLog(QString::fromLocal8Bit("输出机组%1数据整理结束").arg(genname));
    }

    namespace jgg{
        static std::mutex xlsxmutex;
        std::unique_ptr<QXlsx::Document> ReadXlsxDocument(const QString& path){
            std::lock_guard<std::mutex> xlsxlock(xlsxmutex);

            std::unique_ptr<QXlsx::Document> xlsx = std::make_unique<QXlsx::Document>(path);
            return std::move(xlsx);
        }

        bool lostData(const QString& path){
            QDir dir(path);
            QStringList files = dir.entryList(QDir::Filters(QDir::Files));
            for (const QString& file : files){
                if (file.contains(QString::fromLocal8Bit("缺失"))){
                    return true;
                }
            }
            return false;
        }
    }

    void ExcelExportBack::DoEachMonth(int month){
        emit EmitBackLog(QString::fromLocal8Bit("开始处理机组%1在%2月的数据").arg(genname).arg(month));
        QDir dir(monthpath + QString::fromLocal8Bit("/%1月").arg(month));

        qDebug() << dir.absolutePath();

        DayMap dayexport;
        QDate curdate(processyear, month, 1);
        if (jgg::lostData(dir.absolutePath()))
        {
            for (int day = 1; day <= curdate.daysInMonth(); day++){
                emit EmitBackLog(QString::fromLocal8Bit("机组%1在%2月在%3天的数据没有...全补").arg(genname).arg(month).arg(day));
                // 补充数据
                QTime curtime(0, 0);
                QList<CellData> hourdata;
                for (int i = 1; i <= 144; i++){
                    CellData cell;
                    QDateTime datetime(curdate, curtime);
                    cell.Time = datetime.toString(QStringLiteral("yyyy/MM/dd hh:mm"));
                    cell.shicefengsu = QStringLiteral("N/A");
                    cell.shicegonglv = QStringLiteral("N/A");
                    cell.shicejiangjujiao = QStringLiteral("N/A");
                    curtime = curtime.addSecs(600);
                    hourdata.append(cell);
                }
                curdate = curdate.addDays(1);
                dayexport.insert(day, hourdata);
            }
            insertMonthData(month, dayexport);
            return;
        }

        for (int day = 1; day <= curdate.daysInMonth(); day++){//一个月有这么多天
            QString xlsxpath = dir.absolutePath() + "/" + QString("%1.xlsx").arg(day);
            if (!QFile::exists(xlsxpath)){
                emit EmitBackLog(QString::fromLocal8Bit("机组%1在%2月在%3天的数据没有...全补").arg(genname).arg(month).arg(day));
                // 补充数据
                QTime curtime(0, 0);
                QList<CellData> hourdata;
                for (int i = 1; i <= 144; i++){
                    CellData cell;
                    QDateTime datetime(curdate, curtime);
                    cell.Time = datetime.toString(QStringLiteral("yyyy/MM/dd hh:mm"));
                    cell.shicefengsu = QStringLiteral("N/A");
                    cell.shicegonglv = QStringLiteral("N/A");
                    cell.shicejiangjujiao = QStringLiteral("N/A");
                    curtime = curtime.addSecs(600);
                    hourdata.append(cell);
                }
                curdate=curdate.addDays(1);
                dayexport.insert(day, hourdata);
                continue;
            }
            else{
                emit EmitBackLog(QString::fromLocal8Bit("读取到机组%1在%2月在%3天的数据").arg(genname).arg(month).arg(day));
                
                std::unique_ptr<QXlsx::Document> dayxlsx = jgg::ReadXlsxDocument(xlsxpath);
                if (dayxlsx->cellAt(1, 1)->value().toString() != QString::fromLocal8Bit("变量")){//第一格一定是变量
                    throw std::runtime_error("该文件格式解析不了");
                }
                
                int rowcount = dayxlsx->dimension().rowCount();
                bool findspped = false;
                bool findpower = false;
                bool findcorner = false;
                int rowofspped = 0;
                int rowofpower = 0;
                int rowofcorner = 0;
                for (int row = 1; row <= rowcount; row++){
                    QString firstcol = dayxlsx->cellAt(row, 1)->value().toString();
                    if (firstcol == QString::fromLocal8Bit("风速 - AVE [米/秒]")){
                        rowofspped = row;
                        findspped = true;
                    }
                    else if (firstcol == QString::fromLocal8Bit("电网有功功率 - AVE [千瓦]")){
                        // find 实测功率 || "电网有功功率 - AVE [千瓦]"
                        rowofpower = row;
                        findpower = true;
                    }
                    else if (firstcol == QString::fromLocal8Bit("叶片1角度 - AVE [度]")){
                        // find 实测浆距角 || "叶片1角度 - AVE [度]"
                        rowofcorner = row;
                        findcorner = true;

                    }
                    if (findspped && findpower && findcorner){
                        emit EmitBackLog(QString::fromLocal8Bit("机组%1在%2月在%3天的数据在行风速%4功率%5角度%6")
                            .arg(genname).arg(month).arg(day)
                            .arg(rowofspped).arg(rowofpower).arg(rowofcorner));
                        break;
                    }
                }

                QTime curtime(0, 0);
                QList<CellData> hourdata;
                int colcount = dayxlsx->dimension().columnCount();
                for (int i = 1; i <= 144; i++){
                    //开始按时间读取
                    CellData cell;
                    QDateTime datetime(curdate, curtime);
                    cell.Time = datetime.toString(QStringLiteral("yyyy/MM/dd hh:mm"));
                    //先查找有没有这个时间的数据
                    bool isfindcol = false;
                    int findcol = 0;
                    for (int col = 2; col <= colcount; col++){
                        QString strcurtime = curtime.toString("hh:mm");
                        if (dayxlsx->cellAt(1, col)->value().toString().contains(QString("%1:00-").arg(strcurtime))){
                            // 这一行找到了
                            isfindcol = true;
                            findcol = col;
                            break;
                        }
                    }
                    cell.shicefengsu = QStringLiteral("N/A");
                    cell.shicegonglv = QStringLiteral("N/A");
                    cell.shicejiangjujiao = QStringLiteral("N/A");
                    // 如果找到有该时间这一列
                    if (isfindcol){
                        if (findspped){
                            QString speedvalue = dayxlsx->cellAt(rowofspped, findcol)->value().toString().trimmed();
                            if (!speedvalue.isEmpty())
                                cell.shicefengsu = speedvalue;
                        }

                        if (findpower){
                            QString powervalue = dayxlsx->cellAt(rowofpower, findcol)->value().toString().trimmed();
                            if (!powervalue.isEmpty())
                                cell.shicegonglv = powervalue;
                        }

                        if (findcorner){
                            QString cornervalue = dayxlsx->cellAt(rowofcorner, findcol)->value().toString().trimmed();
                            if (!cornervalue.isEmpty())
                                cell.shicejiangjujiao = cornervalue;
                        }
                    }
                    curtime = curtime.addSecs(600);
                    hourdata.append(cell);
                }
                curdate=curdate.addDays(1);
                dayexport.insert(day, hourdata);
            }
        }

        insertMonthData(month, dayexport);
    }

    static std::mutex mutex;

    void ExcelExportBack::insertMonthData(int month, const DayMap& map){
        std::lock_guard<std::mutex> lck(mutex);
        if (monthdata.contains(month)){
            throw std::runtime_error("逻辑有错误，不然怎么会有数据呢");
        }
        monthdata.insert(month, map);
    }
}