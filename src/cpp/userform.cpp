#include "userform.h"
#include "ui_userform.h"

UserForm::UserForm(QWidget* parent) :
    ui(new Ui::UserForm), QWidget(parent)
{
    ui->setupUi(this);
}

UserForm::~UserForm()
{
    delete ui;
}