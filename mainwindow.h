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
    void initProductsView();
    void createMenuBar();
    void showError(const QSqlError &err);    

private slots:
    void about();
    void addOrder();
    void notificationHandler(const QString &name);
    void showOrderItemsDetails(const QModelIndex &index);

private:
    Ui::MainWindow ui;
    std::unique_ptr<AddOrderWindow> addOrderWindow_;
    std::shared_ptr<QSqlRelationalTableModel> orderModel_;
    std::shared_ptr<QSqlRelationalTableModel> orderItemsModel_;
    int orderIdx_, supplierIdx_, productIdx_;
    int ordersIdx_, productsIdx_;
};

#endif
