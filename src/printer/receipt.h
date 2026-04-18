#pragma once

#include <QString>
#include <QSharedPointer>

// ==================== Receipt Structure ====================

struct ReceiptFinishing {
    QString name;
    int qty;
    double cost = 0.0;
};

struct ReceiptItem {
    QString description;
    double quantity = 1.0;
    double unitPrice = 0.0;
    double totalPrice = 0.0;
    QString unit = "pcs";
    QList<ReceiptFinishing> finishings;
};

struct ReceiptPayment {
    QString paymentNumber;      // PAY-20260320-00001
    double  amount       = 0.0;
    double  cashReceived = 0.0; // Hanya terisi jika akunTipe == "cash"
    double  cashChange   = 0.0; // Hanya terisi jika akunTipe == "cash"
    QString date;               // "dd/MM/yyyy HH:mm"
    QString notes;

    // Dari JOIN ke akun_transaksi
    QString akunNama;           // e.g. "Kas Admin", "Bank BRI"
    QString akunTipe;           // "cash" | "bank" | "ewallet" | "lainnya"
};

struct Receipt {
    // Company Info
    QString companyName;
    QString companyAddress;
    QString companyPhone;
    QString companyPhone2;
    // Header Info
    QString invoiceNo;
    QString date;
    QString time;
    QString cashierName;

    // Customer Info
    QString customerName;
    QString customerPhone;
    QString customerAddress;

    // Items
    QList<ReceiptItem>     items;

    // Totals
    double subtotal   = 0.0;
    double discount   = 0.0;
    double taxRate    = 0.11;  // Default PPN 11%
    double tax        = 0.0;
    double grandTotal = 0.0;

    // Payment — satu invoice bisa memiliki banyak payment (cicilan/partial)
    QList<ReceiptPayment> payments;

    // Agregat dari tabel invoices (sudah dihitung di DB)
    double paidAmount = 0.0;    // Total yang sudah dibayar
    double remaining  = 0.0;    // Sisa tagihan (grandTotal - paidAmount)

    // Kembalian tunai dari payment terakhir yang cash (0 jika tidak ada)
    double change = 0.0;

    // Kolom lama — dipertahankan untuk kompatibilitas mundur dengan kode pemanggil
    // yang belum diupdate. Diisi sama dengan paidAmount.
    double amountPaid = 0.0;

    // Status
    QString status;      // "unpaid", "partial", "paid", "refunded"
    QString notes;
    QString pickupDate;
};

using ReceiptPtr = QSharedPointer<Receipt>;

Q_DECLARE_METATYPE(ReceiptPtr);