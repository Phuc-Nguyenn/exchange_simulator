


#include "gtest/gtest.h"
#include "OrderBook.h"
#include "Order.h"
#include "Utils.h"

using namespace Exchange;

TEST(OrderBookTest, PlaceOrderTest)
{

    auto instrumentId = Utils::GenerateId();
    OrderBook orderbook(instrumentId, "AAPL", 0.01);
    auto trader1 = Utils::GenerateId();
    auto trader2 = Utils::GenerateId();
    auto trader3 = Utils::GenerateId();
    auto trader4 = Utils::GenerateId();

    // construct the order book
    /**
     *   6        5       3               2       7      5       8
     * ______________________________________________________________
     * 100.0    100.1   100.2   100.3   100.4   100.5   100.6   100.7   
     */
    orderbook.PlaceOrder(Order(Utils::GenerateId(), trader1, instrumentId, OrderType::LIMIT_BUY, 100.0, 6, std::chrono::system_clock::now()));
    orderbook.PlaceOrder(Order(Utils::GenerateId(), trader2, instrumentId, OrderType::LIMIT_BUY, 100.1, 5, std::chrono::system_clock::now()));
    orderbook.PlaceOrder(Order(Utils::GenerateId(), trader3, instrumentId, OrderType::LIMIT_BUY, 100.2, 3, std::chrono::system_clock::now()));
    orderbook.PlaceOrder(Order(Utils::GenerateId(), trader1, instrumentId, OrderType::LIMIT_SELL, 100.7, 8, std::chrono::system_clock::now()));
    orderbook.PlaceOrder(Order(Utils::GenerateId(), trader2, instrumentId, OrderType::LIMIT_SELL, 100.6, 5, std::chrono::system_clock::now()));
    orderbook.PlaceOrder(Order(Utils::GenerateId(), trader3, instrumentId, OrderType::LIMIT_SELL, 100.5, 7, std::chrono::system_clock::now()));
    orderbook.PlaceOrder(Order(Utils::GenerateId(), trader3, instrumentId, OrderType::LIMIT_SELL, 100.4, 2, std::chrono::system_clock::now()));

    EXPECT_NEAR(100.28, orderbook.GetEffectiveFairPrice(), 1e-3);

    ID buyOrderId = Utils::GenerateId();
    orderbook.PlaceOrder(Order(buyOrderId, 1, instrumentId, OrderType::MARKET_BUY, -1, 10, std::chrono::system_clock::now()));

    const Order& marketBuyOrder = orderbook.RetrieveOrder(buyOrderId);

    EXPECT_EQ(marketBuyOrder.getExecuted(), true);
    EXPECT_EQ(marketBuyOrder.getExecutedQuantity(), 10);
    EXPECT_EQ(marketBuyOrder.getExecutedDollarAmount(), 100.4 * 2 + 100.5 * 7 + 100.6 * 1);


}