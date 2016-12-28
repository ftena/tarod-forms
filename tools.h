#ifndef TOOLS
#define TOOLS

#include <QMessageBox>

inline void showInfo(const QString &info)
{
    QMessageBox::information(nullptr,
                             "Info",
                             info);
}


#endif // TOOLS

