#include "addorderwindow.h"
#include "ui_addorderwindow.h"

AddOrderWindow::AddOrderWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddOrderWindow)
{
    ui->setupUi(this);
}

AddOrderWindow::~AddOrderWindow()
{
    delete ui;
}

void AddOrderWindow::init(std::shared_ptr<QSqlRelationalTableModel> model)
{
    model_ = model;

}
