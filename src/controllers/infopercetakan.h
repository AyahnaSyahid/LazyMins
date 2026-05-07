#pragma once
#include <string>

struct InfoPercetakan {
    std::string nama;
    std::string telp;
    std::string email;
    std::string alamat;
};

class InfoPercetakanController{
    public:
        InfoPercetakanController() = default;
        ~InfoPercetakanController() = default;
        InfoPercetakan getInfoPercetakan() const;
        bool saveInfoPercetakan(const InfoPercetakan& info, std::string * =nullptr);
    private:
        InfoPercetakan info;
};
