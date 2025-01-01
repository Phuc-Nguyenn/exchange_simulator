#include "../3rdParty/cpp-httplib/httplib.h"
#include "Exchange.h"
#include "Utils.h"

namespace Exchange
{
using ID = Utils::ID;

Exchange::Exchange() : server(), orderBooks()
{}

void Exchange::StartServer(std::string&& host, std::uint16_t port)
{
    setRequestOptions();

    server.Get("/ping", [&](const httplib::Request &req, httplib::Response &res) {
        res.set_content("Ping received at: " + Utils::TimePointAsString(Utils::TimeNow()), "text/plain");
        auto it = req.headers.find("Host");
        if(it != req.headers.end()) std::cout << "Pinged by: " << it->second << std::endl;
        SetHeaders(res, {{"Status", "200"}});
        return;
    });

    server.Get("/order-book", [&](const httplib::Request &req, httplib::Response &res) {
        ID instrumentId = resolveInstrument(req, res);
        if(instrumentId == 0) return;
        
        res.set_content(orderBooks.at(instrumentId)->GetOrderBook(), "application/json");
        SetHeaders(res, {{"Status", "200"}, {"Access-Control-Allow-Origin", "*"}});
        return;
    });

    server.Get("/instrument-id", [&](const httplib::Request &req, httplib::Response &res) {
        auto it = req.params.find("instrument-name");
        if(it == req.params.end())
        {
            res.set_content("No instrument name provided\n", "text/plain");
            SetHeaders(res, {{"Status", "400"}});
            return;
        });

        server.Options("/order-book", [&](const httplib::Request &req, httplib::Response &res) {
            SetHeaders(res, {
                {"Access-Control-Allow-Origin", "*"}, 
                {"Access-Control-Allow-Headers", "Content-Type, Authorization"}, 
                {"Access-Control-Allow-Methods", "POST, GET, OPTIONS"}
        });
        ID instrumentId = resolveInstrument(req, res);
        if(instrumentId == 0) {
            SetHeaders(res, {{"Status", "404"}});
            return;
        });

        server.Get("/instrument-id", [&](const httplib::Request &req, httplib::Response &res) {
            auto it = req.params.find("instrument-name");
            if(it == req.params.end())
            {
                res.set_content("No instrument name provided\n", "text/plain");
                SetHeaders(res, {{"Status", "400"}});
                return;
            }
            if(instrumentNameToId.find(it->second) == instrumentNameToId.end())
            {
                res.set_content("Instrument name \""+it->second+"\" not found\n", "text/plain");
                SetHeaders(res, {{"Status", "404"}});
                return;
            }
            res.set_content(std::to_string(instrumentNameToId.at(it->second)), "text/plain");
            SetHeaders(res, {{"Status", "200"}});
            return;
        });

        server.Options("/order", [&](const httplib::Request &req, httplib::Response &res) {
            SetHeaders(res, {
                {"Access-Control-Allow-Origin", "*"}, 
                {"Access-Control-Allow-Headers", "Content-Type, Authorization"}, 
                {"Access-Control-Allow-Methods", "POST, GET, OPTIONS"},
                {"Access-Control-Max-Age", "86400"}
            });
            res.set_content("", "text/plain");
            return;
        });

        server.Post("/order", [&](const httplib::Request &req, httplib::Response &res) {
            
            SetHeaders(res, { 
                    {"Access-Control-Allow-Origin", "*"}, 
                    {"Access-Control-Allow-Headers", "Content-Type, Authorization"}, 
                    {"Access-Control-Allow-Methods", "POST, GET, OPTIONS"}
            });
            ID instrumentId = resolveInstrument(req, res);
            if(instrumentId == 0) {
                SetHeaders(res, {{"Status", "404"}});
                return;
            }

            ID orderId = Utils::GenerateId();
            ID traderId = std::stoull(req.get_param_value("trader-id"));
            OrderType orderType = interpretOrderType(req.get_param_value("order-type"));
            double price = std::stod(req.get_param_value("price"));
            std::uint64_t quantity = std::stoull(req.get_param_value("quantity"));
            auto expiryTime = std::chrono::system_clock::from_time_t(std::stoull(req.get_param_value("expiry-time")));
            Order order(orderId, traderId, instrumentId, orderType, price, quantity, expiryTime);
            orderBooks.at(instrumentId)->PlaceOrder(std::move(order));

            res.set_content(orderBooks[instrumentId]->RetrieveOrder(orderId).getOrderInfoAsStr(), "application/json");
            SetHeaders(res, {{"Status", "200"}});
            return;
        });

        server.Get("/", [&](const httplib::Request &req, httplib::Response &res) {
            res.set_file_content("../public/index.html", "text/html");
            SetHeaders(res, {{"Status", "200"}});
            return;
        });

        ID orderId = Utils::GenerateId();
        ID traderId = std::stoull(req.get_param_value("trader-id"));
        OrderType orderType = interpretOrderType(req.get_param_value("order-type"));
        double price = std::stod(req.get_param_value("price"));
        std::uint64_t quantity = std::stoull(req.get_param_value("quantity"));
        auto expiryTime = std::chrono::system_clock::from_time_t(std::stoull(req.get_param_value("expiry-time")));
        Order order(orderId, traderId, instrumentId, orderType, price, quantity, expiryTime);
        orderBooks.at(instrumentId)->PlaceOrder(std::move(order));

        res.set_content(orderBooks[instrumentId]->RetrieveOrder(orderId).getOrderInfoAsStr(), "application/json");
        SetHeaders(res, {{"Status", "200"}});
        return;
    });

    server.Get("/balance", [&](const httplib::Request &req, httplib::Response &res) {
        auto it = req.params.find("trader-id");
        if(it == req.params.end())
        {
            res.set_content("No trader id provided\n", "text/plain");
            SetHeaders(res, {{"Status", "400"}});
            return;
        }
        ID traderId = std::stoull(it->second);
        if(traderBalance.find(traderId) == traderBalance.end())
        {
            res.set_content("Trader id not found\n", "text/plain");
            SetHeaders(res, {{"Status", "404"}});
            return;
        }
        res.set_content(std::to_string(traderBalance.at(traderId)), "text/plain");
        SetHeaders(res, {{"Status", "200"}});
        return;
    });

    server.Get("/fair-price", [&](const httplib::Request &req, httplib::Response &res) {
        ID instrumentId = resolveInstrument(req, res);
        if(instrumentId == 0) return;
        res.set_content(std::to_string(orderBooks.at(instrumentId)->GetEffectiveFairPrice()), "text/plain");
        SetHeaders(res, {{"Status", "200"}, {"Access-Control-Allow-Origin", "*"}});
        return;
    });

    server.Get("/", [&](const httplib::Request &req, httplib::Response &res) {
        res.set_file_content("../public/index.html", "text/html");
        SetHeaders(res, {{"Status", "200"}});
        return;
    });

    server.Get("/index.js", [&](const httplib::Request &req, httplib::Response &res) {
        res.set_file_content("../public/index.js", "application/javascript");
        SetHeaders(res, {{"Status", "200"}});
        return;
    });

    server.Get("/index.css", [&](const httplib::Request &req, httplib::Response &res) {
        res.set_file_content("../public/index.css", "text/css");
        SetHeaders(res, {{"Status", "200"}});
        return;
    });

    server.listen(host.c_str(), port);
}

ID Exchange::addInstrument(std::string instrumentName, double tick)
{
    if(instrumentNameToId.find(instrumentName) != instrumentNameToId.end())
    {
        return instrumentNameToId.at(instrumentName);
    }
    ID instrumentId = Utils::GenerateId();
    orderBooks[instrumentId] = std::make_unique<OrderBook>(instrumentId, instrumentName, tick);
    instrumentNameToId[instrumentName] = instrumentId;
    return instrumentId;
}

ID Exchange::resolveInstrument(const httplib::Request &req, httplib::Response &res)
{
    auto itId = req.params.find("instrument-id");
    auto itName = req.params.find("instrument-name");
    ID instrumentId;
    if(itId == req.params.end() && itName == req.params.end())
    {
        res.set_content("No instrument id or name provided", "text/plain");
        SetHeaders(res, {{"Status", "400"}});
        return 0;
    }
    else if(itId != req.params.end())
    {
        instrumentId = std::stoull(itId->second);
        if(orderBooks.find(instrumentId) == orderBooks.end())
        {
            res.set_content("No instrument id or name provided", "text/plain");
            SetHeaders(res, {{"Status", "400"}});
            return 0;
        }
        else if(itId != req.params.end())
        {
            instrumentId = std::stoull(itId->second);
            if(orderBooks.find(instrumentId) == orderBooks.end())
            {
                res.set_content("Instrument id not found", "text/plain");
                SetHeaders(res, {{"Status", "404"}});
                return 0;
            }
        }
        else if(itName != req.params.end())
        {
            if(instrumentNameToId.find(itName->second) == instrumentNameToId.end())
            {
                res.set_content("Instrument name not found\n", "text/plain");
                SetHeaders(res, {{"Status", "404"}});
                return 0;
            }
            instrumentId = instrumentNameToId.at(itName->second);
        }
        return instrumentId;
    }
    else if(itName != req.params.end())
    {
        if(instrumentNameToId.find(itName->second) == instrumentNameToId.end())
        {
            res.set_content("Instrument name not found\n", "text/plain");
            SetHeaders(res, {{"Status", "404"}});
            return 0;
        }
        instrumentId = instrumentNameToId.at(itName->second);
    }
    return instrumentId;
}


    OrderType Exchange::interpretOrderType(const std::string &orderType)
    {
        res.set_header(header.first.c_str(), header.second.c_str());
    }
}

void Exchange::setRequestOptions()
{
    server.Options("/order-book", [&](const httplib::Request &req, httplib::Response &res) {
        SetHeaders(res, {
            {"Access-Control-Allow-Origin", "*"}, 
            {"Access-Control-Allow-Headers", "Content-Type"}, 
            {"Access-Control-Max-Age", "86400"}
        });
        res.set_content("", "text/plain");
        return;
    });

    server.Options("/order", [&](const httplib::Request &req, httplib::Response &res) {
        SetHeaders(res, {
            {"Access-Control-Allow-Origin", "*"}, 
            {"Access-Control-Allow-Headers", "Content-Type, Authorization"}, 
            {"Access-Control-Allow-Methods", "POST, GET, OPTIONS"},
            {"Access-Control-Max-Age", "86400"}
        });
        res.set_content("", "text/plain");
        return;
    });

    server.Options("/fair-price", [&](const httplib::Request &req, httplib::Response &res) {
        SetHeaders(res, {
            {"Access-Control-Allow-Origin", "*"}, 
            {"Access-Control-Allow-Headers", "Content-Type, Authorization"}, 
            {"Access-Control-Allow-Methods", "POST, GET, OPTIONS"},
            {"Access-Control-Max-Age", "86400"}
        });
        res.set_content("", "text/plain");
        return;
    });
}


OrderType Exchange::interpretOrderType(const std::string &orderType)
{
    if(orderType == "MARKET_BUY") return MARKET_BUY;
    if(orderType == "MARKET_SELL") return MARKET_SELL;
    if(orderType == "LIMIT_BUY") return LIMIT_BUY;
    if(orderType == "LIMIT_SELL") return LIMIT_SELL;
    if(orderType == "STOP_BUY") return STOP_BUY;
    if(orderType == "STOP_SELL") return STOP_SELL;
    if(orderType == "GOOD_TILL_CANCELLED_BUY") return GOOD_TILL_CANCELLED_BUY;
    if(orderType == "GOOD_TILL_CANCELLED_SELL") return GOOD_TILL_CANCELLED_SELL;
    if(orderType == "IMMEDIATE_OR_CANCEL_BUY") return IMMEDIATE_OR_CANCEL_BUY;
    if(orderType == "IMMEDIATE_OR_CANCEL_SELL") return IMMEDIATE_OR_CANCEL_SELL;
    return CANCEL;
}
}
