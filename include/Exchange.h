#pragma once

#include <unordered_map>
#include "Utils.h"
#include "OrderBook.h"
#include "../3rdParty/cpp-httplib/httplib.h"

namespace Exchange
{

    class Exchange
    {
        using ID = Utils::ID;

    public:
        Exchange();
        ~Exchange() = default;

        Exchange(const Exchange &other) = delete;

        void StartServer(std::string&& host, std::uint16_t port);

        ID addInstrument(std::string instrumentName, double tick = 0.01);
        void SetHeaders(httplib::Response &res, std::vector<std::pair<std::string, std::string>>&& headers);
        std::unordered_map<std::string, std::string> parseJsonStrToMap(const std::string &str);

    private:
        httplib::Server server;
        std::unordered_map<ID, std::unique_ptr<OrderBook>> orderBooks;
        std::unordered_map<std::string, ID> instrumentNameToId;

        ID resolveInstrument(const httplib::Request &req, httplib::Response &res);
        OrderType interpretOrderType(const std::string &orderType);
        void setRequestOptions();
        std::unordered_map<ID, double> traderBalance;

    }; // class Exchange
}; // namespace Exchange