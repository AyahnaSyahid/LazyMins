#include "passwordverification.h"

void PasswordVerification::accept() {
    if(UserItem::passwordMatch(user, ui->lineEdit->text())) {
        QDialog::accept();
    } else {
        QToolTip::showText(mapToGlobal(ui->lineEdit->geometry().topLeft()),
                              "<i style='color:red'>Password Salah!!</i>", nullptr, QRect(), 1000);
        return;
    }
    retry++;
    if(retry >= max_retry) {
        MessageHelper::information(this, "Akses dibatalkan", QString("Pengguna gagal memasukan password setelah %1 kali percobaan").arg(max_retry));
        QDialog::reject();
    }
}
