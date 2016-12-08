#ifndef ADDORDERWINDOW_H
#define ADDORDERWINDOW_H

#include <QWidget>

namespace Ui {
class AddOrderWindow;
}

class AddOrderWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AddOrderWindow(QWidget *parent = 0);
    ~AddOrderWindow();

private:
    Ui::AddOrderWindow *ui;
};

#endif // ADDORDERWINDOW_H
