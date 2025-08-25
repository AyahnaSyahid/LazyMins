#include <QImage>
#include <QPainter>
#include <QString>
#include <vector>

QImage createBarcode(const QString& text, int width_px, int height_px) {
    // Array of widths strings for values 0-106
    const QString widths_str[107] = {
        "212222", "222122", "222221", "121223", "121322", "131222", "122213", "122312", "132212", "221213", // 0-9
        "221312", "231212", "112232", "122132", "122231", "113222", "123122", "123221", "223211", "221132", // 10-19
        "221231", "213212", "223112", "312131", "311222", "321122", "321221", "312212", "322112", "322211", // 20-29
        "212123", "212321", "232121", "111323", "131123", "131321", "112313", "132113", "132311", "211313", // 30-39
        "231113", "231311", "112133", "112331", "132131", "113123", "113321", "133121", "313121", "211331", // 40-49
        "231131", "213113", "213311", "213131", "311123", "311321", "331121", "312113", "312311", "332111", // 50-59
        "314111", "221411", "431111", "111224", "111422", "121124", "121421", "141122", "141221", "112214", // 60-69
        "112412", "122114", "122411", "142112", "142211", "241211", "221114", "413111", "241112", "134111", // 70-79
        "111242", "121142", "121241", "114212", "124112", "124211", "411212", "421112", "421211", "212141", // 80-89
        "214121", "412121", "111143", "111341", "131141", "114113", "114311", "411113", "411311", "113141", // 90-99
        "114131", "311141", "411131", "211412", "211214", "211232", "2331112" // 100-105, 106 stop special
    };

    // Note: widths_str[106] is for stop: "2331112"

    // Assume text is printable ASCII, use Code Set B
    QByteArray data = text.toLatin1();
    int num_data = data.length();
    if (num_data == 0) {
        QImage empty(width_px, height_px, QImage::Format_ARGB32);
        empty.fill(Qt::white);
        return empty;
    }

    // Check if all characters are printable ASCII
    for (char c : data) {
        if (c < 32 || c > 127) {
            // Unsupported character, return white image
            QImage error(width_px, height_px, QImage::Format_ARGB32);
            error.fill(Qt::white);
            return error;
        }
    }

    // Start B
    int start_value = 104;
    std::vector<int> codes;
    codes.push_back(start_value);

    // Data values
    for (char c : data) {
        int val = static_cast<unsigned char>(c) - 32;
        codes.push_back(val);
    }

    // Checksum
    int weighted_sum = start_value;
    for (size_t pos = 1; pos < codes.size(); ++pos) {
        weighted_sum += static_cast<int>(pos) * codes[pos];
    }
    int check_value = weighted_sum % 103;
    codes.push_back(check_value);

    // Calculate total modules
    int num_symbols = codes.size(); // start + data + check
    int data_modules = 11 * num_symbols;
    int stop_modules = 13;
    int quiet_modules = 10 * 2;
    int total_modules = quiet_modules + data_modules + stop_modules;

    if (total_modules == 0 || width_px <= 0 || height_px <= 0) {
        QImage empty(width_px, height_px, QImage::Format_ARGB32);
        empty.fill(Qt::white);
        return empty;
    }

    double module_px = static_cast<double>(width_px) / total_modules;

    QImage image(width_px, height_px, QImage::Format_ARGB32);
    image.fill(Qt::white);

    QPainter painter(&image);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);

    double x = 0.0;

    // Left quiet zone
    x += 10 * module_px;

    // Symbols: start, data, check
    for (int code : codes) {
        QString w = widths_str[code];
        for (int j = 0; j < 6; ++j) {
            int ww = w[j].digitValue();
            if (j % 2 == 0) { // bar
                painter.drawRect(QRectF(x, 0, ww * module_px, height_px));
            }
            x += ww * module_px;
        }
    }

    // Stop
    QString stop_w = widths_str[106];
    for (int j = 0; j < 7; ++j) {
        int ww = stop_w[j].digitValue();
        if (j % 2 == 0) { // bar
            painter.drawRect(QRectF(x, 0, ww * module_px, height_px));
        }
        x += ww * module_px;
    }

    // Right quiet zone is already white

    return image;
}