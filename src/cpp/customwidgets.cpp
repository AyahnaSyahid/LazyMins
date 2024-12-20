#include "customwidgets.h"

CustomWidgets::SpinBoxUang::SpinBoxUang(QWidget* parent)
  : QSpinBox(parent)
{
    setMaximum(2147483647);
    setButtonSymbols(QAbstractSpinBox::NoButtons);
    setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    setSingleStep(1000);
    setGroupSeparatorShown(true);
}