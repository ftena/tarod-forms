#include <QtSql>
#include <QTableView>
#include "addorderwindow.h"
#include "ui_addorderwindow.h"
#include "tools.h"

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

void AddOrderWindow::init(std::shared_ptr<QSqlRelationalTableModel> model,
                          QTableView *tableView)
{
    //TODO: don't use a magical number,
    //use  model_->fieldIndex("supplier") instead.
    //but... http://stackoverflow.com/questions/41354212
    int supplierIdx = 2;
    int productIdx = 3;

    model_ = model;    
    ui->supplierCombo->setModel(model_->relationModel(supplierIdx));
    ui->supplierCombo->setModelColumn(model_->relationModel(supplierIdx)->fieldIndex("name"));
    ui->productsView->setModel(model_->relationModel(productIdx));
    ui->productsView->setModelColumn(model_->relationModel(productIdx)->fieldIndex("name"));
    tableView_ = tableView;
}

void AddOrderWindow::on_buttonBox_accepted()
{
    //TODO: don't use a magical number,
    //use  model_->fieldIndex("supplier") instead.
    //but... http://stackoverflow.com/questions/41354212
    int productIdx = 3;

    QSqlRecord record = model_->record();

    /*
     * id serial, name varchar, supplier integer, product integer, year integer, rating integer
     */
    QSqlField f0("id", QVariant::Int);
    QSqlField f1("name", QVariant::String);
    QSqlField f2("supplier", QVariant::Int);
    QSqlField f3("product", QVariant::Int);
    QSqlField f4("year", QVariant::Int);
    QSqlField f5("rating", QVariant::Int);

    // get next value for the id sequence
    QSqlQuery q;
    if(!q.exec("SELECT nextval(pg_get_serial_sequence('orders', 'id'))"))
    {
        showInfo(q.lastError().text());
        return;
    } else {
        if (q.next()) {
            int lastId = q.value(0).toInt();
            f0.setValue(lastId);
        }
        else {
            showInfo(q.lastError().text());
            return;
        }
    }

    f1.setValue(QVariant(ui->nameEdit->text()));

    //Get the underlying index (not the visible text) for the combobox
    int row = ui->supplierCombo->currentIndex();
    QModelIndex idx = ui->supplierCombo->model()->index(row, 0); // first combobox column (= the id)
    QVariant data = ui->supplierCombo->model()->data(idx);
    f2.setValue(QVariant(data.toInt()));


    //Get the underlying index (not the visible text) for the listview
    /*Same way than before...
    row = ui->productsView->currentIndex().row();
    idx = ui->productsView->model()->index(row, 0); // first listview column (= the id)
    data = ui->productsView->model()->data(idx);
    f3.setValue(QVariant(data.toInt()));
    */

    //Or...
    QSqlRecord r = model_->relationModel(productIdx)->record(ui->productsView->currentIndex().row());
    f3.setValue(QVariant(r.value("id").toInt()));

    f4.setValue(QVariant(ui->yearSpinBox->value()));
    f5.setValue(QVariant(ui->ratingSpinBox->value()));

    if (ui->productsView->selectionModel()->selectedIndexes().empty()) {
        showInfo("Please, select a product.");
        return;
    }

    record.append(f0);
    record.append(f1);
    record.append(f2);
    record.append(f3);
    record.append(f4);
    record.append(f5);

    if (model_->insertRecord(-1, record))
    {
        qDebug() << Q_FUNC_INFO << " OK ";

        int lastRow = model_->rowCount() - 1;
        tableView_->selectRow(lastRow);
        tableView_->scrollToBottom();
    } else {
        qDebug() << Q_FUNC_INFO << " NO OK " << model_->lastError().text();
    }

    close();
}

void AddOrderWindow::on_buttonBox_rejected()
{
    close();
}
