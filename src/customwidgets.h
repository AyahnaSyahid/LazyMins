#ifndef CustomWidgetsH
#define CustomWidgetsH

#include <QSpinBox>

namespace CustomWidgets {
    class SpinBoxUang;
}

class CustomWidgets::SpinBoxUang : public QSpinBox
{
  public:
    SpinBoxUang(QWidget* =nullptr);
};

#endif //CustomWidgetsH