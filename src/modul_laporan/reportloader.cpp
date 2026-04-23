#include "reportloader.h"

// ============================================================
//  PUBLIC
// ============================================================

DailySalesReport ReportLoader::loadDailySales(const QDate& date)
{
    m_error.clear();
    const QString dateStr = date.toString("yyyy-MM-dd");

    DailySalesReport r;
    r.company       = loadCompanyInfo();
    r.meta          = buildMeta(date, "RPT-H");
    r.summary       = loadSalesSummary(dateStr);
    r.orders        = loadOrders(dateStr);
    r.paymentMethods= loadPaymentMethods(dateStr);
    r.topProducts   = loadTopProducts(dateStr);
    r.notes         = "Laporan ini mencakup semua order selain 'cancelled'. "
                      "Waktu menggunakan zona WIB (UTC+7). "
                      "Data piutang belum termasuk order dari hari sebelumnya.";
    return r;
}

DailyExpenseReport ReportLoader::loadDailyExpense(const QDate& date)
{
    m_error.clear();
    const QString dateStr = date.toString("yyyy-MM-dd");

    DailyExpenseReport r;
    r.company    = loadCompanyInfo();
    r.meta       = buildMeta(date, "RPT-B");
    r.summary    = loadExpenseSummary(dateStr);
    r.expenses   = loadExpenses(dateStr);
    r.byCategory = loadByCategory(dateStr);
    r.byAccount  = loadByAccount(dateStr);
    r.notes      = "Hanya mencakup transaksi bertipe 'pengeluaran' yang dicatat hari ini. "
                   "Waktu menggunakan zona WIB (UTC+7).";
    return r;
}

// ============================================================
//  SHARED
// ============================================================

CompanyInfo ReportLoader::loadCompanyInfo()
{
    // Ambil beberapa key sekaligus dengan satu query
    QSqlQuery q(m_db);
    q.prepare(R"(
        SELECT setting_key, setting_value
        FROM   app_settings
        WHERE  setting_key IN (
                   'company_name', 'company_address',
                   'company_phone','company_email')
    )");

    CompanyInfo info;
    if (!exec(q, "loadCompanyInfo")) return info;

    while (q.next()) {
        const QString key = q.value("setting_key").toString();
        const QString val = q.value("setting_value").toString();
        if      (key == "company_name")    info.name    = val;
        else if (key == "company_address") info.address = val;
        else if (key == "company_phone")   info.phone   = val;
        else if (key == "company_email")   info.email   = val;
    }
    return info;
}

ReportMeta ReportLoader::buildMeta(const QDate& date, const QString& prefix)
{
    ReportMeta m;
    m.periodDate  = date;
    m.printedAt   = QDateTime::currentDateTimeUtc();

    // nomor dokumen: PREFIX-YYYYMMDD-NNNNN
    // hitung berapa laporan sudah dicetak hari ini (dari activity_logs atau
    // cukup gunakan timestamp — di sini kita buat sekuensial sederhana)
    m.documentNumber = QString("%1-%2-001")
                           .arg(prefix)
                           .arg(date.toString("yyyyMMdd"));

    // ambil nama admin yang sedang login (asumsi disimpan di sesi aplikasi)
    // jika tidak ada mekanisme sesi, bisa diisi dari luar setelah load
    m.printedBy = "System";
    return m;
}

// ============================================================
//  SALES — helpers
// ============================================================

SalesSummary ReportLoader::loadSalesSummary(const QString& dateStr)
{
    SalesSummary s;

    // total order & omzet dari orders
    {
        QSqlQuery q(m_db);
        q.prepare(QString(R"(
            SELECT COUNT(*)              AS total_orders,
                   COALESCE(SUM(total_amount), 0) AS total_revenue
            FROM   orders
            WHERE  staging_status != 'cancelled'
              AND  %1
        )").arg(wibFilter().arg("order_date")));
        q.bindValue(":date", dateStr);

        if (exec(q, "salesSummary::orders") && q.next()) {
            s.totalOrders  = q.value("total_orders").toInt();
            s.totalRevenue = toInt(q.value("total_revenue"));
        }
    }

    // paid & unpaid dari invoices (lebih akurat untuk status pembayaran)
    {
        QSqlQuery q(m_db);
        q.prepare(QString(R"(
            SELECT COALESCE(SUM(paid_amount),      0) AS paid,
                   COALESCE(SUM(remaining_amount), 0) AS remaining
            FROM   invoices
            WHERE  staging_status != 'cancelled'
              AND  is_active = 1
              AND  %1
        )").arg(wibFilter().arg("issue_date")));
        q.bindValue(":date", dateStr);

        if (exec(q, "salesSummary::invoices") && q.next()) {
            s.paidAmount   = toInt(q.value("paid"));
            s.unpaidAmount = toInt(q.value("remaining"));
        }
    }

    return s;
}

QList<OrderRow> ReportLoader::loadOrders(const QString& dateStr)
{
    QList<OrderRow> rows;

    QSqlQuery q(m_db);
    q.prepare(QString(R"(
        SELECT o.order_number,
               o.customer_name,
               o.staging_status,
               o.total_amount,
               COALESCE(i.settlement_status, 'unpaid') AS settlement_status,
               -- ringkasan item: nama produk pertama + jumlah item lain
               (
                   SELECT oi.product_name
                   FROM   order_items oi
                   WHERE  oi.order_id = o.id
                   ORDER  BY oi.id
                   LIMIT  1
               ) AS first_product,
               (
                   SELECT COUNT(*)
                   FROM   order_items oi
                   WHERE  oi.order_id = o.id
               ) AS item_count
        FROM   orders o
        LEFT   JOIN invoices i ON i.id = o.invoice_id
        WHERE  o.staging_status != 'cancelled'
          AND  %1
        ORDER  BY o.order_date
    )").arg(wibFilter().arg("o.order_date")));
    q.bindValue(":date", dateStr);

    if (!exec(q, "loadOrders")) return rows;

    while (q.next()) {
        OrderRow row;
        row.orderNumber      = q.value("order_number").toString();
        row.customerName     = q.value("customer_name").toString();
        row.totalAmount      = toInt(q.value("total_amount"));
        row.productionStatus = q.value("staging_status").toString();
        row.paymentStatus    = q.value("settlement_status").toString();

        // ringkasan produk: "Banner Flexy (+2 item lain)"
        const QString first = q.value("first_product").toString();
        const int     count = q.value("item_count").toInt();
        row.productSummary  = (count > 1)
            ? QString("%1 (+%2 lainnya)").arg(first).arg(count - 1)
            : first;

        rows.append(row);
    }
    return rows;
}

QList<PaymentMethodRow> ReportLoader::loadPaymentMethods(const QString& dateStr)
{
    QList<PaymentMethodRow> rows;

    QSqlQuery q(m_db);
    q.prepare(QString(R"(
        SELECT at.nama                      AS method_name,
               COUNT(p.id)                 AS tx_count,
               COALESCE(SUM(p.amount), 0)  AS total_amount
        FROM   payments p
        JOIN   akun_transaksi at ON at.id = p.akun_transaksi_id
        WHERE  p.verification_status = 'verified'
          AND  %1
        GROUP  BY at.id, at.nama
        ORDER  BY total_amount DESC
    )").arg(wibFilter().arg("p.payment_date")));
    q.bindValue(":date", dateStr);

    if (!exec(q, "loadPaymentMethods")) return rows;

    while (q.next()) {
        rows.append({
            q.value("method_name").toString(),
            q.value("tx_count").toInt(),
            toInt(q.value("total_amount"))
        });
    }
    return rows;
}

QList<TopProductRow> ReportLoader::loadTopProducts(const QString& dateStr)
{
    QList<TopProductRow> rows;

    QSqlQuery q(m_db);
    q.prepare(QString(R"(
        SELECT p.name                                                   AS product_name,
               pc.category_name,
               p.unit,
               p.use_area,
               CASE WHEN p.use_area = 0
                    THEN CAST(SUM(oi.quantity) AS TEXT)
                    ELSE CAST(ROUND(SUM(oi.quantity *
                                       oi.size_width *
                                       oi.size_height), 2) AS TEXT)
               END                                                      AS qty_sold,
               COALESCE(SUM(oi.total), 0)                              AS revenue
        FROM   order_items oi
        JOIN   products p        ON p.id = oi.product_id
        JOIN   product_categories pc ON pc.id = p.category_id
        JOIN   orders o          ON o.id = oi.order_id
        WHERE  o.staging_status != 'cancelled'
          AND  %1
        GROUP  BY p.id, p.name, pc.category_name, p.unit, p.use_area
        ORDER  BY revenue DESC
        LIMIT  10
    )").arg(wibFilter().arg("o.order_date")));
    q.bindValue(":date", dateStr);

    if (!exec(q, "loadTopProducts")) return rows;

    while (q.next()) {
        TopProductRow row;
        row.productName  = q.value("product_name").toString();
        row.categoryName = q.value("category_name").toString();
        row.revenue      = toInt(q.value("revenue"));

        // format qty + satuan: "120 lbr" atau "8.5 m²"
        const QString qty  = q.value("qty_sold").toString();
        const QString unit = q.value("unit").toString();
        const bool useArea = q.value("use_area").toBool();
        row.qtySold = useArea
            ? QString("%1 m²").arg(qty)
            : QString("%1 %2").arg(qty, unit);

        rows.append(row);
    }
    return rows;
}

// ============================================================
//  EXPENSE — helpers
// ============================================================

ExpenseSummary ReportLoader::loadExpenseSummary(const QString& dateStr)
{
    ExpenseSummary s;

    // total semua pengeluaran
    {
        QSqlQuery q(m_db);
        q.prepare(QString(R"(
            SELECT COALESCE(SUM(amount), 0) AS total
            FROM   transaksi
            WHERE  tipe = 'pengeluaran'
              AND  %1
        )").arg(wibFilter().arg("created_at")));
        q.bindValue(":date", dateStr);

        if (exec(q, "expenseSummary::total") && q.next())
            s.totalExpense = toInt(q.value("total"));
    }

    // bahan baku vs operasional — dibedakan lewat parent kategori
    {
        QSqlQuery q(m_db);
        q.prepare(QString(R"(
            SELECT kt.nama                          AS kat_nama,
                   COALESCE(SUM(t.amount), 0)       AS subtotal
            FROM   transaksi t
            JOIN   kategori_transaksi kt ON kt.id = t.kategori_id
            WHERE  t.tipe = 'pengeluaran'
              AND  %1
            GROUP  BY kt.id, kt.nama
        )").arg(wibFilter().arg("t.created_at")));
        q.bindValue(":date", dateStr);

        if (exec(q, "expenseSummary::breakdown")) {
            while (q.next()) {
                const QString nama = q.value("kat_nama").toString().toLower();
                const qint64  sub  = toInt(q.value("subtotal"));

                // kategorisasi sederhana berdasarkan nama — sesuaikan
                // dengan data aktual di tabel kategori_transaksi kamu
                if (nama.contains("bahan") || nama.contains("stok") ||
                    nama.contains("material"))
                    s.materialExpense += sub;
                else
                    s.opsExpense += sub;
            }
        }
    }

    return s;
}

QList<ExpenseRow> ReportLoader::loadExpenses(const QString& dateStr)
{
    QList<ExpenseRow> rows;

    QSqlQuery q(m_db);
    q.prepare(QString(R"(
        SELECT t.transaction_number,
               t.deskripsi,
               kt.nama             AS kat_nama,
               kt.parent_id,
               at.nama             AS akun_nama,
               a.nama_lengkap      AS admin_nama,
               t.amount,
               -- pakai parent nama untuk grouping tipe
               COALESCE(ktp.nama, kt.nama) AS parent_nama
        FROM   transaksi t
        JOIN   kategori_transaksi kt  ON kt.id  = t.kategori_id
        LEFT   JOIN kategori_transaksi ktp ON ktp.id = kt.parent_id
        JOIN   akun_transaksi at      ON at.id  = t.akun_id
        JOIN   admins a               ON a.id   = t.admin_id
        WHERE  t.tipe = 'pengeluaran'
          AND  %1
        ORDER  BY
               CASE WHEN LOWER(kt.nama) LIKE '%%bahan%%'
                         OR LOWER(kt.nama) LIKE '%%stok%%'   THEN 0
                    ELSE 1 END,
               t.created_at
    )").arg(wibFilter().arg("t.created_at")));
    q.bindValue(":date", dateStr);

    if (!exec(q, "loadExpenses")) return rows;

    while (q.next()) {
        ExpenseRow row;
        row.txNumber     = q.value("transaction_number").toString();
        row.description  = q.value("deskripsi").toString();
        row.categoryName = q.value("kat_nama").toString();
        row.accountName  = q.value("akun_nama").toString();
        row.recordedBy   = q.value("admin_nama").toString();
        row.amount       = toInt(q.value("amount"));

        // tentukan tipe untuk warna badge
        const QString kat = q.value("kat_nama").toString().toLower();
        if (kat.contains("bahan") || kat.contains("stok") ||
            kat.contains("material"))
            row.categoryType = "material";
        else if (kat.contains("listrik") || kat.contains("gaji") ||
                 kat.contains("sewa")    || kat.contains("operasional"))
            row.categoryType = "ops";
        else
            row.categoryType = "other";

        rows.append(row);
    }
    return rows;
}

QList<ExpenseCategoryRow> ReportLoader::loadByCategory(const QString& dateStr)
{
    QList<ExpenseCategoryRow> rows;

    // hitung total dulu untuk persentase
    qint64 grandTotal = 0;
    {
        QSqlQuery q(m_db);
        q.prepare(QString(R"(
            SELECT COALESCE(SUM(amount), 0) AS total
            FROM   transaksi
            WHERE  tipe = 'pengeluaran'
              AND  %1
        )").arg(wibFilter().arg("created_at")));
        q.bindValue(":date", dateStr);
        if (exec(q, "byCategory::total") && q.next())
            grandTotal = toInt(q.value("total"));
    }

    QSqlQuery q(m_db);
    q.prepare(QString(R"(
        SELECT kt.nama                         AS kat_nama,
               COUNT(t.id)                    AS tx_count,
               COALESCE(SUM(t.amount), 0)     AS subtotal
        FROM   transaksi t
        JOIN   kategori_transaksi kt ON kt.id = t.kategori_id
        WHERE  t.tipe = 'pengeluaran'
          AND  %1
        GROUP  BY kt.id, kt.nama
        ORDER  BY subtotal DESC
    )").arg(wibFilter().arg("t.created_at")));
    q.bindValue(":date", dateStr);

    if (!exec(q, "loadByCategory")) return rows;

    while (q.next()) {
        ExpenseCategoryRow row;
        row.categoryName = q.value("kat_nama").toString();
        row.txCount      = q.value("tx_count").toInt();
        row.amount       = toInt(q.value("subtotal"));
        row.percent      = (grandTotal > 0)
            ? (row.amount * 100.0 / grandTotal)
            : 0.0;
        rows.append(row);
    }
    return rows;
}

QList<ExpenseAccountRow> ReportLoader::loadByAccount(const QString& dateStr)
{
    QList<ExpenseAccountRow> rows;

    QSqlQuery q(m_db);
    q.prepare(QString(R"(
        SELECT at.nama                         AS akun_nama,
               COUNT(t.id)                    AS tx_count,
               COALESCE(SUM(t.amount), 0)     AS subtotal
        FROM   transaksi t
        JOIN   akun_transaksi at ON at.id = t.akun_id
        WHERE  t.tipe = 'pengeluaran'
          AND  %1
        GROUP  BY at.id, at.nama
        ORDER  BY subtotal DESC
    )").arg(wibFilter().arg("t.created_at")));
    q.bindValue(":date", dateStr);

    if (!exec(q, "loadByAccount")) return rows;

    while (q.next()) {
        rows.append({
            q.value("akun_nama").toString(),
            q.value("tx_count").toInt(),
            toInt(q.value("subtotal"))
        });
    }
    return rows;
}
