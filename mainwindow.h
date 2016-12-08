#ifndef BOOKWINDOW_H
#define BOOKWINDOW_H

#include <memory>
#include <QtWidgets>
#include <QtSql>

#include "ui_mainwindow.h"

class AddOrderWindow;

class MainWindow: public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow();

private:    
    void createMenuBar();
    void showError(const QSqlError &err);

private slots:
    void about();
    void addOrder();
    void notificationHandler(const QString &name);

private:
    Ui::MainWindow ui;
    std::unique_ptr<AddOrderWindow> addOrderWindow;
    QSqlRelationalTableModel *model;
    int supplierIdx, productIdx;
};

#endif
