#ifndef PasswordVerificationH
#define PasswordVerificationH

#include "resetpassword.h"
#include "ui_resetpassword.h"
#include "helper.h"
#include <QToolTip>
#include <QPushButton>

class PasswordVerification : public ResetPassword
{
    Q_OBJECT
    public:
        PasswordVerification(QWidget* parent=nullptr) : max_retry(3), retry(0), ResetPassword(parent)
        {
            ui->label_2->setText("Password");
            setWindowTitle("Verifikasi Password");
            auto simpan = ui->buttonBox->button(QDialogButtonBox::Save);
            simpan->setText("Lanjutkan");
            adjustSize();
        }
        ~PasswordVerification() {}
        inline void setMaxRetries(int mx) { max_retry = mx; }
        
    public:
        void accept() override;
    
    private:
        int max_retry;
        int retry;
};

#endif // PasswordVerificationH