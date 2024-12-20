#ifndef HelperH
#define HelperH

#include <QMessageBox>

namespace MessageHelper {
    int question(QWidget* parent,
                         const QString& title,
                         const QString& message);
                                         
    int information(QWidget* parent,
                             const QString& title,
                             const QString& message);
    
    int warning(QWidget* parent,
                const QString& title,
                const QString& message);
    
}

namespace StringHelper {
    QString currentDateTimeString();
}

namespace InputHelper {
    double getDouble(QWidget* parent,
                     const QString& title,
                     const QString& label,
                     double value = 0,
                     double minValue = -2147483647,
                     double maxValue = 2147483647,
                     int decimals = 0,
                     bool *ok=nullptr,
                     Qt::WindowFlags flags= Qt::WindowFlags());
}

namespace ToolTipHelper {
    void showTip(const QPoint&, const QString& tips);
};
#endif