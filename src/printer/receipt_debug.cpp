#include <QDebug>
#include "receipt.h"

// Helper untuk mencetak ReceiptFinishing
QDebug operator<<(QDebug debug, const ReceiptFinishing &f) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "Finishing(" << f.name << ", qty: " << f.qty << ", cost: " << f.cost << ")";
    return debug;
}

// Helper untuk mencetak ReceiptItem
QDebug operator<<(QDebug debug, const ReceiptItem &item) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "\n    Item: " << item.description 
                    << " [" << item.quantity << " " << item.unit << " @ " << item.unitPrice << " = " << item.totalPrice << "]";
    if (!item.finishings.isEmpty()) {
        debug << "\n      Finishings:" << item.finishings;
    }
    return debug;
}

// Helper untuk mencetak ReceiptPayment
QDebug operator<<(QDebug debug, const ReceiptPayment &p) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "Payment(" << p.paymentNumber << ", Amount: " << p.amount 
                    << ", Via: " << p.akunNama << " [" << p.akunKode << "])";
    return debug;
}

// Helper utama untuk Receipt
QDebug operator<<(QDebug debug, const Receipt &r) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "--- RECEIPT DEBUG ---\n"
                    << "Invoice No : " << r.invoiceNo << " [" << r.status << "]\n"
                    << "Date/Time  : " << r.date << " " << r.time << "\n"
                    << "Customer   : " << r.customerName << " (" << r.customerPhone << ")\n"
                    << "Cashier    : " << r.cashierName << "\n"
                    << "---------------------\n"
                    << "Items      : " << r.items << "\n"
                    << "---------------------\n"
                    << "Subtotal   : " << r.subtotal << "\n"
                    << "Tax (PPN)  : " << r.tax << " (" << (r.taxRate * 100) << "%)\n"
                    << "Grand Total: " << r.grandTotal << "\n"
                    << "Paid       : " << r.paidAmount << "\n"
                    << "Remaining  : " << r.remaining << "\n"
                    << "Payments   : " << r.payments << "\n"
                    << "Notes      : " << r.notes << "\n"
                    << "---------------------";
    return debug;
}
