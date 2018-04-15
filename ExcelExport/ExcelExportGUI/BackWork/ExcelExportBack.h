#ifndef EXCEL_EXPORT_BACK_H
#define EXCEL_EXPORT_BACK_H
#include <QThread>
#include <QList>
#include <QMap>
#include <memory>
#include <future>
namespace back{

    struct CellData{
        QString Time;
        QString shicefengsu;
        QString shicegonglv;
        QString shicejiangjujiao;
    };

    using DayMap = QMap<int, QList<CellData>>;
    using MonthMap = QMap<int, DayMap>;

    class ExcelExportBack :public QThread{
        Q_OBJECT

    public:
        explicit ExcelExportBack(int year,const QString& gen, const QString& datapth, QObject *parent = Q_NULLPTR);

        virtual ~ExcelExportBack();

        void setMonthPath(const QString& path){
            monthpath = path;
        }

        const QString getMonthPath()const{
            return monthpath;
        }

        void setGenName(const QString& gen){
            genname = gen;
        }

        const QString getGenName() const{
            return genname;
        }

        void resetData(){
            monthdata.clear();
        }

    Q_SIGNALS:
        void EmitBackLog(const QString& log);

    protected:
        virtual void run();

    private:
        void DoEachMonth(int month);
        std::unique_ptr<std::future<bool>> CreateMonthFuture(int month);

        void insertMonthData(int month, const DayMap& map);

    private:
        Q_DISABLE_COPY(ExcelExportBack);
        QString monthpath;
        QString genname;
        MonthMap monthdata;
        int processyear;
    };
}

#endif