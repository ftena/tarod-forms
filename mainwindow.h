#ifndef BOOKWINDOW_H
#define BOOKWINDOW_H

#include <QtWidgets>
#include <QtSql>

#include "ui_mainwindow.h"

class MainWindow: public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();

private:    
    void createMenuBar();
    void showError(const QSqlError &err);

private slots:
    void about();
    void addOrder();
    void notificationHandler(const QString &name);

private:
    Ui::MainWindow ui;
    QSqlRelationalTableModel *model;
    int supplierIdx, productIdx;
};

#endif
