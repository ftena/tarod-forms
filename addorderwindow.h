#ifndef ADDORDERWINDOW_H
#define ADDORDERWINDOW_H

#include <memory>
#include <QWidget>

class QSqlRelationalTableModel;
class QTableView;

namespace Ui {
class AddOrderWindow;
}

class AddOrderWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AddOrderWindow(QWidget *parent = 0);
    ~AddOrderWindow();
    void init(std::shared_ptr<QSqlRelationalTableModel> model, QTableView *tableView);

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::AddOrderWindow *ui;
    std::shared_ptr<QSqlRelationalTableModel> model_;
    QTableView *tableView_;
};

#endif // ADDORDERWINDOW_H
