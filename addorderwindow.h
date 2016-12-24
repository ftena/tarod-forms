#ifndef ADDORDERWINDOW_H
#define ADDORDERWINDOW_H

#include <memory>
#include <QWidget>

class QSqlRelationalTableModel;

namespace Ui {
class AddOrderWindow;
}

class AddOrderWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AddOrderWindow(QWidget *parent = 0);
    ~AddOrderWindow();
    void init(std::shared_ptr<QSqlRelationalTableModel> model);

private:
    Ui::AddOrderWindow *ui;
    std::shared_ptr<QSqlRelationalTableModel> model_;
};

#endif // ADDORDERWINDOW_H
