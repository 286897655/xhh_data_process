#include "ExcelExportGUI.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

ExcelExportGUI::ExcelExportGUI(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    ui.txtPath->setPlaceholderText(QString::fromLocal8Bit("ѡ����Ŀ¼Ӧ����ʾ������..."));
    ui.txtYear->setPlaceholderText(QString::fromLocal8Bit("�����봦����� Ĭ��2013��"));

#if _DEBUG
    ui.txtPath->setText("F:/Git/xhh_data_process/ExcelExport");
#endif

    connect(ui.btnPath, SIGNAL(clicked()), this, SLOT(OnPathClicked()));

    connect(ui.btnWork, SIGNAL(clicked()), this, SLOT(OnWorkClicked()));
}


void ExcelExportGUI::OnPathClicked(){
    QString path = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("Ŀ¼ѡ��"), qApp->applicationDirPath(), QFileDialog::ShowDirsOnly
        | QFileDialog::DontResolveSymlinks);

    if (path.isEmpty())
        return;

    ui.txtPath->setText(path);
}

void ExcelExportGUI::OnWorkClicked(){
    if (workstart){
        // �Ѿ��ڿ�ʼ��������
        QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("�Ѿ���ʼ�������ݣ���ȴ�..."), QMessageBox::Ok);
        return;
    }

    bool backwork = false;
    QMapIterator<QString, back::ExcelExportBack*> iterator(backworks);
    if (iterator.hasNext()){
        iterator.next();
        backwork = iterator.value()->isRunning() && backwork;
    }
    
    if (backwork){
        // �Ѿ��ڿ�ʼ��������
        QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("�Ѿ���ʼ�������ݣ���ȴ�..."), QMessageBox::Ok);
        return;
    }

    AppendLog(QString::fromLocal8Bit("��ʼ���Ŀ¼�µ��ļ�"));
    QString path = ui.txtPath->text().trimmed();
    if (path.isEmpty()){
        QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("ûѡ��Ŀ¼"), QMessageBox::Ok);
        return;
    }
    QString processyear = ui.txtYear->text().trimmed();
    if (!processyear.isEmpty()){
        year = processyear.toInt();
    }
    workstart = true;
    QDir dir(path);
    QStringList entrys = dir.entryList(QDir::Filters(QDir::Dirs));

    QRegExp rx("[F]+\\d+");
    for (const QString& entry : entrys){
        if (rx.indexIn(entry) < 0)
            continue;
        AppendLog(QString::fromLocal8Bit("��⵽����%1").arg(entry));

        if (!dir.cd(entry)){
            AppendLog(QString::fromLocal8Bit("���ܽ���Ŀ¼"));
            throw std::runtime_error("��ô������Ŀ¼��");
        }
        else{
            QStringList months = dir.entryList(QDir::Filters(QDir::Dirs));
            
            Q_ASSERT(months.size() == 14);

            for (const QString& month : months){
                if (!month.endsWith(QString::fromLocal8Bit("��")))
                    continue;

                AppendLog(QString::fromLocal8Bit("��⵽����%1���·�%2����").arg(entry).arg(month));
            }

            QString childpath = dir.absolutePath();
            dir.cd("..");

            if (!backworks.contains(entry)){
                back::ExcelExportBack* work = new back::ExcelExportBack(year, entry, childpath, this);
                connect(work, &back::ExcelExportBack::EmitBackLog, this, &ExcelExportGUI::OnBackLogEmit);
                backworks.insert(entry, work);
                work->start();
            }
            else{
                back::ExcelExportBack* oldwork = backworks.value(entry);
                oldwork->setMonthPath(childpath);
                oldwork->start();
            }
        }
    }
    workstart = false;
}

void ExcelExportGUI::OnBackLogEmit(const QString& log){
    AppendLog(log);
}

void ExcelExportGUI::AppendLog(const QString& log){
    ui.plainTextEdit->appendPlainText(log);
}