#pragma once

#include <chrono>
#include <string>
#include <sstream>

#include "./Utils.h"


enum OrderType
{
    MARKET_BUY = 0,
    MARKET_SELL,
    LIMIT_BUY,
    LIMIT_SELL,
    STOP_BUY,
    STOP_SELL,
    GOOD_TILL_CANCELLED_BUY,
    GOOD_TILL_CANCELLED_SELL,
    IMMEDIATE_OR_CANCEL_BUY,
    IMMEDIATE_OR_CANCEL_SELL,
    CANCEL
};

class Order
{
    using ID = Utils::ID;

public:
    Order(ID orderId, std::uint64_t traderId, ID instrumentId, OrderType orderType, double price, std::uint64_t quantity, std::chrono::time_point<std::chrono::system_clock> expiryTime) 
    : orderId(orderId)
    , traderId(traderId)
    , instrumentId(instrumentId)
    , orderType(orderType)
    , price(price)
    , quantity(quantity)
    , expiryTime(expiryTime)
    , executed(false)
    , executedQuantity(0)
    , executedDollarAmount(0)
    {};

    // Getters
    std::uint64_t getTraderId() const { return traderId; }
    ID getInstrumentId() const { return instrumentId; }
    OrderType getOrderType() const { return orderType; }
    double getPrice() const { return price; }
    std::uint64_t getQuantity() const { return quantity; }
    ID getOrderId() const { return orderId; }
    std::chrono::time_point<std::chrono::system_clock> getExpiryTime() const { return expiryTime; }
    bool getExecuted() const { return executed; }
    std::uint64_t getExecutedQuantity() const { return executedQuantity; }
    double getExecutedDollarAmount() const { return executedDollarAmount; }

    std::string getOrderInfoAsStr() const {
        std::ostringstream oss;
        oss << "{ \"orderId\": \"" << getOrderId() << "\", "
            << "\"traderId\": \"" << getTraderId() << "\", "
            << "\"instrumentId\": \"" << getInstrumentId() << "\", "
            << "\"orderType\": \"" << getOrderType() << "\", "
            << "\"price\": " << getPrice() << ", "
            << "\"quantity\": " << getQuantity() << ", "
            << "\"expiryTime\": " << Utils::TimePointAsString(getExpiryTime()) << "} ";
        return oss.str();
    }

    // Setters
    void setTraderId(std::uint64_t traderId) { this->traderId = traderId; }
    void setInstrumentId(ID instrumentId) { this->instrumentId = instrumentId; }
    void setOrderType(OrderType orderType) { this->orderType = orderType; }
    void setPrice(double price) { this->price = price; }
    void setQuantity(std::uint64_t quantity) { this->quantity = quantity; }
    void setExpiryTime(std::chrono::time_point<std::chrono::system_clock> expiryTime) { this->expiryTime = expiryTime; }
    void setExecuted(bool executed) { this->executed = executed; }
    void setExecutedQuantity(std::uint64_t executedQuantity) { this->executedQuantity = executedQuantity; }
    void setExecutedDollarAmount(double executedTotalPrice) { this->executedDollarAmount = executedTotalPrice; }

private:
    ID orderId;
    ID traderId;
    ID instrumentId;
    OrderType orderType;
    double price;
    std::uint64_t quantity;
    std::chrono::time_point<std::chrono::system_clock> expiryTime;
    bool executed;
    std::uint64_t executedQuantity;
    double executedDollarAmount;
};