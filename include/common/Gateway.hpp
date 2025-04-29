#pragma once

#include "Instrument.hpp"
#include "AsyncHttp.hpp"
#include "config.hpp"

#include <iostream>
#include <string>

class Gateway {
    private:
        AsyncHttp http;

    protected:
        AsyncHttp& getHttp() {return http;}


    public:
        std::string url;
        Exchange name;

        virtual BBO getBBO(Token buyToken, Token sellToken) = 0;
        virtual std::string getTicker(Token& base, Token& quote) = 0;

        Gateway() {
            http.init(5);
        }

        void destroy() {
            std::cout << "Destroying " << name << " Gateway\n";
            http.destroy();
        }

        virtual ~Gateway() = default;
};
