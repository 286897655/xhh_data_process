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
        emit EmitBackLog(QString::fromLocal8Bit("��ʼ�������%1������").arg(genname));

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

                emit EmitBackLog(QString::fromLocal8Bit("����%1���������ڶ�ȡ��...").arg(genname));
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }

        //���¶�ȡ������
        emit EmitBackLog(QString::fromLocal8Bit("����%1�����ݶ�ȡ����,��ʼ���뵽�ļ�").arg(genname));
        QFile file(monthpath+"/"+QString("%1.csv").arg(genname));
        if (file.open(QIODevice::WriteOnly)){
            QTextStream stream(&file);
            stream << 
                QString::fromLocal8Bit("ʱ��") << "," 
                << QString::fromLocal8Bit("ʵ�����") << "," 
                << QString::fromLocal8Bit("ʵ�⹦��") << "," 
                << QString::fromLocal8Bit("ʵ�⽬���") << "\r\n";

            for (int month = 1; month <= 12; month++){
                DayMap daydata = monthdata.value(month);
                emit EmitBackLog(QString::fromLocal8Bit("�������%1%2�µ�����").arg(genname).arg(month));
                for (int day = 1; day <= daydata.size(); day++){
                    if (!daydata.contains(day)){
                        throw std::runtime_error("�߼������Ⱑ����Ȼ��ô��û����");
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
            throw std::runtime_error("����������ļ�");
        }
        emit EmitBackLog(QString::fromLocal8Bit("�������%1�����������").arg(genname));
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
                if (file.contains(QString::fromLocal8Bit("ȱʧ"))){
                    return true;
                }
            }
            return false;
        }
    }

    void ExcelExportBack::DoEachMonth(int month){
        emit EmitBackLog(QString::fromLocal8Bit("��ʼ�������%1��%2�µ�����").arg(genname).arg(month));
        QDir dir(monthpath + QString::fromLocal8Bit("/%1��").arg(month));

        qDebug() << dir.absolutePath();

        DayMap dayexport;
        QDate curdate(processyear, month, 1);
        if (jgg::lostData(dir.absolutePath()))
        {
            for (int day = 1; day <= curdate.daysInMonth(); day++){
                emit EmitBackLog(QString::fromLocal8Bit("����%1��%2����%3�������û��...ȫ��").arg(genname).arg(month).arg(day));
                // ��������
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

        for (int day = 1; day <= curdate.daysInMonth(); day++){//һ��������ô����
            QString xlsxpath = dir.absolutePath() + "/" + QString("%1.xlsx").arg(day);
            if (!QFile::exists(xlsxpath)){
                emit EmitBackLog(QString::fromLocal8Bit("����%1��%2����%3�������û��...ȫ��").arg(genname).arg(month).arg(day));
                // ��������
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
                emit EmitBackLog(QString::fromLocal8Bit("��ȡ������%1��%2����%3�������").arg(genname).arg(month).arg(day));
                
                std::unique_ptr<QXlsx::Document> dayxlsx = jgg::ReadXlsxDocument(xlsxpath);
                if (dayxlsx->cellAt(1, 1)->value().toString() != QString::fromLocal8Bit("����")){//��һ��һ���Ǳ���
                    throw std::runtime_error("���ļ���ʽ��������");
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
                    if (firstcol == QString::fromLocal8Bit("���� - AVE [��/��]")){
                        rowofspped = row;
                        findspped = true;
                    }
                    else if (firstcol == QString::fromLocal8Bit("�����й����� - AVE [ǧ��]")){
                        // find ʵ�⹦�� || "�����й����� - AVE [ǧ��]"
                        rowofpower = row;
                        findpower = true;
                    }
                    else if (firstcol == QString::fromLocal8Bit("ҶƬ1�Ƕ� - AVE [��]")){
                        // find ʵ�⽬��� || "ҶƬ1�Ƕ� - AVE [��]"
                        rowofcorner = row;
                        findcorner = true;

                    }
                    if (findspped && findpower && findcorner){
                        emit EmitBackLog(QString::fromLocal8Bit("����%1��%2����%3����������з���%4����%5�Ƕ�%6")
                            .arg(genname).arg(month).arg(day)
                            .arg(rowofspped).arg(rowofpower).arg(rowofcorner));
                        break;
                    }
                }

                QTime curtime(0, 0);
                QList<CellData> hourdata;
                int colcount = dayxlsx->dimension().columnCount();
                for (int i = 1; i <= 144; i++){
                    //��ʼ��ʱ���ȡ
                    CellData cell;
                    QDateTime datetime(curdate, curtime);
                    cell.Time = datetime.toString(QStringLiteral("yyyy/MM/dd hh:mm"));
                    //�Ȳ�����û�����ʱ�������
                    bool isfindcol = false;
                    int findcol = 0;
                    for (int col = 2; col <= colcount; col++){
                        QString strcurtime = curtime.toString("hh:mm");
                        if (dayxlsx->cellAt(1, col)->value().toString().contains(QString("%1:00-").arg(strcurtime))){
                            // ��һ���ҵ���
                            isfindcol = true;
                            findcol = col;
                            break;
                        }
                    }
                    cell.shicefengsu = QStringLiteral("N/A");
                    cell.shicegonglv = QStringLiteral("N/A");
                    cell.shicejiangjujiao = QStringLiteral("N/A");
                    // ����ҵ��и�ʱ����һ��
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
            throw std::runtime_error("�߼��д��󣬲�Ȼ��ô����������");
        }
        monthdata.insert(month, map);
    }
}