#include "ExcelExportGUI.h"
#include <QtWidgets/QApplication>
#include <QDateTime>
#include <QDebug>

#include <QtXlsx/xlsxdocument.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ExcelExportGUI w;
    w.show();
    return a.exec();
}
