#pragma once

#include <QtWidgets/QMainWindow>
#include <QMap>
#include "ui_ExcelExportGUI.h"
#include "BackWork/ExcelExportBack.h"
class ExcelExportGUI : public QMainWindow
{
    Q_OBJECT

public:
    ExcelExportGUI(QWidget *parent = Q_NULLPTR);


    private slots:

    void OnPathClicked();
    void OnWorkClicked();
    void OnBackLogEmit(const QString& log);

private:
    void AppendLog(const QString& log);

private:
    Ui::ExcelExportGUIClass ui;
    bool workstart = false;
    QMap<QString, back::ExcelExportBack*> backworks;
    int year = 2013;
};
