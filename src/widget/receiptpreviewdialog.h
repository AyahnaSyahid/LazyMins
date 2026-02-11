#ifndef RECEIPTPREVIEWDIALOG_H
#define RECEIPTPREVIEWDIALOG_H

#include <QDialog>
#include <QGraphicsView>
#include <QGraphicsScene>
#include "../invoicedatatype.h"   // mengandung struct PrintInvoiceParams

class ReceiptPreviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReceiptPreviewDialog(const PrintInvoiceParams &params,
                                  QWidget *parent = nullptr);
    ~ReceiptPreviewDialog();

private:
    void setupUi();
    void populateScene();

    // Konfigurasi ukuran & tampilan
    void drawHeader();
    void drawItems();
    void drawPayments();
    void drawFooter();
    void drawDashedLine(double y);

    // Helper untuk penempatan teks
    QGraphicsTextItem* addCenteredText(const QString &text, const QFont &font, double y);
    QGraphicsTextItem* addLeftText(const QString &text, const QFont &font, double y, double margin = 10.0);
    QGraphicsTextItem* addRightText(const QString &text, const QFont &font, double y, double margin = 10.0);

private:
    PrintInvoiceParams m_params;

    QGraphicsScene    *m_scene   = nullptr;
    QGraphicsView     *m_view    = nullptr;

    // Konfigurasi receipt (disesuaikan untuk thermal 76 mm ~ 203 dpi)
    static constexpr double DPI             = 203.0;
    static constexpr double PAPER_WIDTH_MM  = 76.0;
    static constexpr double PAPER_WIDTH_PX  = PAPER_WIDTH_MM * DPI / 25.4;  // ≈ 595–600 px
    static constexpr double LEFT_MARGIN     = 12.0;
    static constexpr double RIGHT_MARGIN    = 12.0;
    static constexpr double LINE_SPACING    = 4.0;

    double m_currentY = 10.0;
};

#endif // RECEIPTPREVIEWDIALOG_H