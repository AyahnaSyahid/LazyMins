#pragma once

#include <QDialog>
#include <QVariantMap>

namespace Ui {
  class ProductEditorDialog;
}

class ProductEditorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ProductEditorDialog(QWidget *parent = nullptr);
    ~ProductEditorDialog();
  
    bool setProductId(int pid);

private slots:
    void on_simpanButton_clicked();
    void reject() override;

private:
    Ui::ProductEditorDialog *ui;
    int m_productId;

    // Cache data asli untuk dirty checking
    QString m_origName;
    QString m_origSku;
    int m_origCatId;
    int m_origActive;
    QString m_origUnit;
    QString m_origDesc;
    int m_origCalcArea;

    bool isDirty() const;
    bool commit();
};