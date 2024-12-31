#pragma once

#include <queue>
#include <unordered_map>
#include <map>
#include <memory>
#include <list>
#include <limits.h>

#include "Order.h"
#include "Utils.h"

#define DEBUG 1

namespace Exchange
{
using ID = Utils::ID;

enum Action {
    BID,
    ASK
};

struct Level
{
    Level(double price, std::uint64_t quantity, Action action, ID orderId) 
        : price(price)
        , quantity(quantity)
        , action(action)
    {
        orderIds.push_back(orderId);
    };

    const double price;
    mutable std::uint64_t quantity;
    const Action action;
    mutable std::list<ID> orderIds;
};

struct BidComp
{
    bool operator()(const std::shared_ptr<Level>& lhs, const std::shared_ptr<Level>& rhs) const
    {
        return lhs->price < rhs->price;
    }
};

struct AskComp
{
    bool operator()(const std::shared_ptr<Level>& lhs, const std::shared_ptr<Level>& rhs) const
    {
        return lhs->price > rhs->price;
    }
};

class OrderBook
{
public:
    OrderBook(ID instrumentId,  std::string instrumentName, double tick = 0.01) 
        : instrumentId(instrumentId)
        , instrumentName(instrumentName)
        , tick(tick)
        {};
    ~OrderBook() {};
    OrderBook(const OrderBook& other) = delete;

    ID getInstrumentId() const { return instrumentId; }
    void SetInstrumentId(ID id) { instrumentId = id; }
    std::string GetInstrumentName() const { return instrumentName; }
    void SetInstrumentName(const std::string& name) { instrumentName = name; }
    
    bool PlaceOrder(const Order&& order);
    double GetEffectiveFairPrice();
    std::string GetOrderBook() const;
    const Order& RetrieveOrder(ID orderId) const;

private:
    ID instrumentId; 
    std::string instrumentName;

    std::unordered_map<ID, Order> orders;
    std::priority_queue<std::shared_ptr<Level>, std::vector<std::shared_ptr<Level>>, BidComp> bids;
    std::priority_queue<std::shared_ptr<Level>, std::vector<std::shared_ptr<Level>>, AskComp> asks;

    std::unordered_map<double, std::shared_ptr<Level>> bidLevels;
    std::unordered_map<double, std::shared_ptr<Level>> askLevels;


    double tick;

    bool PlaceMarketBuy(Order& order, double limitPrice = __DBL_MAX__);
    bool PlaceMarketSell(Order& order, double limitPrice = 0);
    bool PlaceLimitBuy(Order& );
    bool PlaceLimitSell(Order& );
    bool PlaceStopBuy(Order& );
    bool PlaceStopSell(Order& );
    bool PlaceGoodTillCancelledBuy(Order& );
    bool PlaceGoodTillCancelledSell(Order& );
    bool PlaceImmediateOrCancelBuy(Order& );
    bool PlaceImmediateOrCancelSell(Order& );
    bool CancelOrder(ID );

    template<typename pqType>
    inline std::vector<std::shared_ptr<Level>> PqToVector(pqType pq) const;
};


template<typename pqType>
inline std::vector<std::shared_ptr<Level>> OrderBook::PqToVector(pqType pq) const
{
    std::vector<std::shared_ptr<Level>> vec;
    while (!pq.empty())
    {
        vec.push_back(pq.top());
        pq.pop();
    }
    return vec;
}

}