#include "helper.h"
#include <QDateTime>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFont>
#include <QToolTip>

int MessageHelper::information(QWidget* parent, const QString& title, const QString& message) {
    QMessageBox box(parent);
    box.setIcon(QMessageBox::Information);
    box.setWindowTitle(title);
    box.setText(message);
    box.setStandardButtons(QMessageBox::Yes);
    box.setButtonText(QMessageBox::Yes, "Ya");
    return box.exec();
};

int MessageHelper::question(QWidget* parent, const QString& title, const QString& message) {
    QMessageBox box(parent);
    box.setIcon(QMessageBox::Question);
    box.setWindowTitle(title);
    box.setText(message);
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setButtonText(QMessageBox::Yes, "Ya");
    box.setButtonText(QMessageBox::No, "Tidak");
    box.setDefaultButton(QMessageBox::No);
    return box.exec();
};

int MessageHelper::warning(QWidget* parent, const QString& title, const QString& message) {
    QMessageBox box(parent);
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle(title);
    box.setText(message);
    box.setStandardButtons(QMessageBox::Yes);
    box.setButtonText(QMessageBox::Yes, "Ya");
    return box.exec();
};

QString StringHelper::currentDateTimeString() {
    return QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
};

double InputHelper::getDouble(QWidget* parent,
                              const QString& title,
                              const QString& label,
                              double val,
                              double minVal,
                              double maxVal,
                              int decimals,
                              bool *ok,
                              Qt::WindowFlags flags)
{
    QDialog dg(parent, flags);
    QDialogButtonBox bbox(&dg);
    QDoubleSpinBox ds(&dg);
    QLabel lb(&dg);
    auto font = lb.font();
    font.setPointSize(font.pointSize() + 4);
    lb.setFont(font);
    lb.setText(label);
    QVBoxLayout ly;
    ly.addWidget(&lb, 1, Qt::AlignLeft | Qt::AlignTop);
    ly.addWidget(&ds);
    ly.addWidget(&bbox);
    auto bok = bbox.addButton(QDialogButtonBox::Yes);
    bok->setText("Ya");
    bok->setAutoDefault(false);
    auto bno = bbox.addButton(QDialogButtonBox::No);
    bno->setText("Batal");
    bno->setDefault(true);
    
    dg.connect(&bbox, SIGNAL(accepted()), &dg, SLOT(accept()));
    dg.connect(&bbox, SIGNAL(rejected()), &dg, SLOT(reject()));
    
    dg.setLayout(&ly);
    dg.setWindowTitle(title);
    ds.setRange(minVal, maxVal);
    ds.setValue(val);
    ds.setAlignment(Qt::AlignRight);
    ds.setGroupSeparatorShown(true);
    ds.setButtonSymbols(QAbstractSpinBox::NoButtons);
    ds.setSingleStep(1000);
    ds.setFont(font);
    dg.adjustSize();
    dg.exec();
    return ds.value();
}

void ToolTipHelper::showTip(const QPoint& pt, const QString& tips) {
    QToolTip::showText(pt, tips, nullptr, QRect(), 1500);
}