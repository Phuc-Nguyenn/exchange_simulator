#include "OrderBook.h"
#include <cmath>
#include <iostream>

namespace Exchange
{

    /**
     * @brief Place a market buy order
     *
     * return false if nothing is executed
     * returns true if something is executed and the order is changed
     */
    bool OrderBook::PlaceMarketBuy(Order &buyOrder, double limitPrice)
    {
        if (buyOrder.getOrderType() != MARKET_BUY)
        {
#if DEBUG
            std::cout << "OrderBook::PlaceMarketBuy: Order with id: " << buyOrder.getOrderId() << " is not a market buy order, on execution of order: " << buyOrder.getOrderInfoAsStr() << std::endl;
#endif
            buyOrder.setExecuted(false);
            return false;
        }

        if (asks.empty())
        {
#if DEBUG
            std::cout << "OrderBook::PlaceMarketBuy: No asks in the order book, on execution of order: " << buyOrder.getOrderInfoAsStr() << std::endl;
#endif
            buyOrder.setExecuted(false);
            return false;
        }

        std::uint64_t initialQuantity = buyOrder.getQuantity();

        // go down the order book and get the best quantities until the asks list
        // is empty or the order is filled
        while (buyOrder.getQuantity() >= 0 && !asks.empty() && asks.top()->price <= limitPrice)
        {
            while (!asks.top()->orderIds.empty())
            {
                ID askId = asks.top()->orderIds.front();
                Order &askOrder = orders.at(askId);
                if (askOrder.getQuantity() >= buyOrder.getQuantity())
                {
                    asks.top()->quantity -= buyOrder.getQuantity();
                    if(asks.top()->quantity == 0)
                        asks.pop();
                    askOrder.setQuantity(askOrder.getQuantity() - buyOrder.getQuantity());
                    buyOrder.setExecutedQuantity(buyOrder.getExecutedQuantity() + buyOrder.getQuantity());
                    buyOrder.setExecutedDollarAmount(buyOrder.getExecutedDollarAmount() + buyOrder.getQuantity() * askOrder.getPrice());
                    buyOrder.setQuantity(0);
                    buyOrder.setExecuted(initialQuantity > buyOrder.getQuantity());
#if DEBUG
                    std::cout << "OrderBook::PlaceMarketBuy: Ask with id " << askOrder.getOrderInfoAsStr() << " filled buy order with id: " << buyOrder.getOrderInfoAsStr() << std::endl;
#endif
                    return buyOrder.getExecuted();
                }
                else // askOrder.quantity < buyOrder.quantity
                {
                    buyOrder.setExecutedQuantity(buyOrder.getExecutedQuantity() + askOrder.getQuantity());
                    buyOrder.setQuantity(buyOrder.getQuantity() - askOrder.getQuantity());
                    buyOrder.setExecutedDollarAmount(buyOrder.getExecutedDollarAmount() + askOrder.getQuantity() * askOrder.getPrice());
#if DEBUG
                    std::cout << "OrderBook::PlaceMarketBuy: Buy order: " << buyOrder.getOrderInfoAsStr() << " filled ask: " << askOrder.getOrderInfoAsStr() << std::endl;
#endif
                    asks.top()->quantity -= askOrder.getQuantity();
                    asks.top()->orderIds.pop_front();
                }
            }
            asks.pop();
        }
        buyOrder.setExecuted(initialQuantity > buyOrder.getQuantity());
#if DEBUG
        std::cout << "OrderBook::PlaceMarketBuy: Should not reach this code, on execution of order: " << buyOrder.getOrderInfoAsStr() << std::endl;
#endif
        return buyOrder.getExecuted();
    }

    bool OrderBook::PlaceMarketSell(Order &sellOrder, double limitPrice)
    {
        if (sellOrder.getOrderType() != MARKET_SELL)
        {
#if DEBUG
            std::cout << "OrderBook::PlaceMarketSell: Order with id: " << sellOrder.getOrderId() << " is not a market sell order, on execution of order: " << sellOrder.getOrderInfoAsStr() << std::endl;
#endif
            sellOrder.setExecuted(false);
            return false;
        }
        if (bids.empty())
        {
#if DEBUG
            std::cout << "OrderBook::PlaceMarketSell: No bids in the order book, on execution of order: " << sellOrder.getOrderInfoAsStr() << std::endl;
#endif
            sellOrder.setExecuted(false);
            return false;
        }

        std::uint64_t initialQuantity = sellOrder.getQuantity();
        // go down the order book and get the best quantities until the bids list
        // is empty or the order is filled
        while (sellOrder.getQuantity() > 0 && !bids.empty() && bids.top()->price >= limitPrice)
        {
            while (!bids.top()->orderIds.empty())
            {
                ID bidId = bids.top()->orderIds.front();
                Order &bidOrder = orders.at(bidId);
                if (bidOrder.getQuantity() >= sellOrder.getQuantity())
                {
                    bids.top()->quantity -= sellOrder.getQuantity();
                    if(bids.top()->quantity == 0)
                        bids.pop();
                    bidOrder.setQuantity(bidOrder.getQuantity() - sellOrder.getQuantity());
                    sellOrder.setExecutedQuantity(sellOrder.getExecutedQuantity() + sellOrder.getQuantity());
                    sellOrder.setExecutedDollarAmount(sellOrder.getExecutedDollarAmount() + sellOrder.getQuantity() * bidOrder.getPrice());
                    sellOrder.setQuantity(0);
                    sellOrder.setExecuted(initialQuantity > sellOrder.getQuantity());
#if DEBUG
                    std::cout << "OrderBook::PlaceMarketSell: Bid with id " << bidOrder.getOrderInfoAsStr() << " filled sell order with id: " << sellOrder.getOrderInfoAsStr() << std::endl;
#endif
                    return sellOrder.getExecuted();
                }
                else // bidOrder.quantity < sellOrder.quantity
                {
                    sellOrder.setQuantity(sellOrder.getQuantity() - bidOrder.getQuantity());
                    sellOrder.setExecutedQuantity(sellOrder.getExecutedQuantity() + bidOrder.getQuantity());
                    sellOrder.setExecutedDollarAmount(sellOrder.getExecutedDollarAmount() + bidOrder.getQuantity() * bidOrder.getPrice());
#if DEBUG
                    std::cout << "OrderBook::PlaceMarketSell: Sell order with id: " << sellOrder.getOrderInfoAsStr() << " filled bid with id " << bidOrder.getOrderInfoAsStr() << std::endl;
#endif
                    bids.top()->quantity -= bidOrder.getQuantity();
                    bids.top()->orderIds.pop_front();
                }
            }
            bids.pop();
        }
        sellOrder.setExecuted(initialQuantity > sellOrder.getQuantity());
#if DEBUG
        std::cout << "OrderBook::PlaceMarketSell: Order not filled, on execution of order: " << sellOrder.getOrderInfoAsStr() << std::endl;
#endif
        return sellOrder.getExecuted();
    }

    bool OrderBook::PlaceLimitBuy(Order &order)
    {
        if (order.getOrderType() != LIMIT_BUY)
        {
#if DEBUG
            std::cout << "OrderBook::PlaceLimitBuy: Order with id: " << order.getOrderId() << " is not a limit buy order" << std::endl;
#endif
            return false;
        }

        order.setPrice(Utils::SnapDownPriceToTick(order.getPrice(), tick));

        // try to execute as much of the order as a market buy at the limit price
        order.setOrderType(MARKET_BUY);
        if (PlaceMarketBuy(order, order.getPrice()) && order.getQuantity() == 0)
        {
#if DEBUG
            std::cout << "OrderBook::PlaceLimitBuy: Order executed as market buy" << std::endl;
#endif
            return true;
        }
        order.setOrderType(LIMIT_BUY);
        // for the remaining quantity, place the order in the order book as a bid
        if (bidLevels.find(order.getPrice()) == bidLevels.end())
        {
            bidLevels[order.getPrice()] = std::make_shared<Level>(order.getPrice(), order.getQuantity(), BID, order.getOrderId());
            bids.push(bidLevels[order.getPrice()]);
#if DEBUG
            std::cout << "OrderBook::PlaceLimitBuy: New ask level created at price " << order.getPrice() << std::endl;
#endif
        }
        else
        {
            bidLevels[order.getPrice()]->quantity += order.getQuantity();
            bidLevels[order.getPrice()]->orderIds.push_back(order.getOrderId());
#if DEBUG
            std::cout << "OrderBook::PlaceLimitBuy: Order: " << order.getOrderId() << " inserted into bid level with price " << order.getPrice() << std::endl;
#endif
        }
        orders.insert({order.getOrderId(), order});
        return true;
    }

    bool OrderBook::PlaceLimitSell(Order &order)
    {
        if (order.getOrderType() != LIMIT_SELL)
        {
#if DEBUG
            std::cout << "OrderBook::PlaceLimitSell: Order with id: " << order.getOrderId() << " is not a limit sell order" << std::endl;
#endif
            return false;
        }

        order.setPrice(Utils::SnapDownPriceToTick(order.getPrice(), tick));

        // try to execute as much of the order as a market buy at the limit price
        order.setOrderType(MARKET_SELL);
        if (PlaceMarketSell(order, order.getPrice()) && order.getQuantity() == 0)
        {
#if DEBUG
            std::cout << "OrderBook::PlaceLimitSell: Order executed as market sell" << std::endl;
#endif
            return true;
        }
        order.setOrderType(LIMIT_SELL);
        // for the remaining quantity, place the order in the order book as a ask
        if (askLevels.find(order.getPrice()) == askLevels.end())
        {
            askLevels[order.getPrice()] = std::make_shared<Level>(order.getPrice(), order.getQuantity(), ASK, order.getOrderId());
            asks.push(askLevels[order.getPrice()]);
#if DEBUG
            std::cout << "OrderBook::PlaceLimitBuy: New ask level created at price " << order.getPrice() << std::endl;
#endif
        }
        else
        {
            askLevels[order.getPrice()]->quantity += order.getQuantity();
            askLevels[order.getPrice()]->orderIds.push_back(order.getOrderId());
#if DEBUG
            std::cout << "OrderBook::PlaceLimitSell: Order: " << order.getOrderId() << " inserted into ask level with price " << order.getPrice() << std::endl;
#endif
        }
        orders.insert({order.getOrderId(), order});
        return true;
    }

    bool OrderBook::PlaceStopBuy(Order &order)
    {
        return true;
    }

    bool OrderBook::PlaceStopSell(Order &order)
    {
        return true;
    }

    bool OrderBook::PlaceGoodTillCancelledBuy(Order &order)
    {
        return true;
    }

    bool OrderBook::PlaceGoodTillCancelledSell(Order &order)
    {
        return true;
    }

    bool OrderBook::PlaceImmediateOrCancelBuy(Order &order)
    {
        if (order.getOrderId() != IMMEDIATE_OR_CANCEL_BUY)
        {
#if DEBUG
            std::cout << "OrderBook::PlaceImmediateOrCancelBuy: Order with id: " << order.getOrderId() << " is not a immediate or cancel buy order" << std::endl;
#endif
            return false;
        }

        order.setOrderType(MARKET_BUY);
        if (PlaceMarketBuy(order, order.getPrice()) && order.getExecuted())
        {
#if DEBUG
            std::cout << "OrderBook::PlaceImmediateOrCancelBuy: Order with id: " << order.getOrderId() << "  executed as market buy" << std::endl;
#endif
            return true;
        }
        return false;
    }

    bool OrderBook::PlaceImmediateOrCancelSell(Order &order)
    {
        if (order.getOrderType() != IMMEDIATE_OR_CANCEL_SELL)
        {
#if DEBUG
            std::cout << "OrderBook::PlaceImmediateOrCancelSell: Order with id: " << order.getOrderId() << " is not an immediate or cancel sell order" << std::endl;
#endif
            return false;
        }

        order.setOrderType(MARKET_SELL);
        if (PlaceMarketSell(order, order.getPrice()) && order.getExecuted())
        {
#if DEBUG
            std::cout << "OrderBook::PlaceImmediateOrCancelSell: Order with id: " << order.getOrderId() << " executed as market sell" << std::endl;
#endif
            return true;
        }
        return false;
    }

    bool OrderBook::CancelOrder(ID orderId)
    {
        auto it = orders.find(orderId);
        if (it == orders.end())
        {
#if DEBUG
            std::cout << "OrderBook::CancelOrder: Order with id: " << orderId << " not found" << std::endl;
#endif
            return false;
        }
        Order& order = it->second;
        if (bidLevels.find(order.getPrice()) != bidLevels.end())
        {
            bidLevels[order.getPrice()]->quantity -= order.getQuantity();
            bidLevels[order.getPrice()]->orderIds.remove(orderId);
        } 
        else
        {
            askLevels[order.getPrice()]->quantity -= order.getQuantity();
            askLevels[order.getPrice()]->orderIds.remove(orderId);
        }
        return true;
    }

    const Order& OrderBook::RetrieveOrder(ID orderId) const
    {
        return orders.at(orderId);
    }

    bool OrderBook::PlaceOrder(const Order &&order)
    {
        orders.insert({order.getOrderId(), std::move(order)});
        switch (order.getOrderType())
        {
        case MARKET_BUY:
            return PlaceMarketBuy(orders.at(order.getOrderId()));
        case MARKET_SELL:
            return PlaceMarketSell(orders.at(order.getOrderId()));
        case LIMIT_BUY:
            return PlaceLimitBuy(orders.at(order.getOrderId()));
        case LIMIT_SELL:
            return PlaceLimitSell(orders.at(order.getOrderId()));
        case STOP_BUY:
            return PlaceStopBuy(orders.at(order.getOrderId()));
        case STOP_SELL:
            return PlaceStopSell(orders.at(order.getOrderId()));
        case GOOD_TILL_CANCELLED_BUY:
            return PlaceGoodTillCancelledBuy(orders.at(order.getOrderId()));
        case GOOD_TILL_CANCELLED_SELL:
            return PlaceGoodTillCancelledSell(orders.at(order.getOrderId()));
        case IMMEDIATE_OR_CANCEL_BUY:
            return PlaceImmediateOrCancelBuy(orders.at(order.getOrderId()));
        case IMMEDIATE_OR_CANCEL_SELL:
            return PlaceImmediateOrCancelSell(orders.at(order.getOrderId()));
        }
        return false;
    }

    double OrderBook::GetEffectiveFairPrice()
    {
        if (bids.empty() || asks.empty())
        {
            return 0.0;
        }
        return Utils::SnapDownPriceToTick((bids.top()->price * bids.top()->quantity + asks.top()->price * asks.top()->quantity) / (bids.top()->quantity + asks.top()->quantity), tick);
    }

    std::string OrderBook::GetOrderBook() const
    {
        std::string orderBookStr = "{\"instrumentName\": \"" + instrumentName + "\", \"bids\": [";
        auto tempBids = bids;
        while (!tempBids.empty())
        {
            orderBookStr += "{ \"price\": " + std::to_string(tempBids.top()->price) + ", \"quantity\": " + std::to_string(tempBids.top()->quantity) + "}";
            tempBids.pop();
            if (!tempBids.empty())
            {
                orderBookStr += ", ";
            }
        }
        orderBookStr += "], \"asks\": [";
        auto tempAsks = PqToVector<decltype(asks)>(asks);
        while (!tempAsks.empty())
        {
            orderBookStr += "{ \"price\": " + std::to_string(tempAsks.back()->price) + ", \"quantity\": " + std::to_string(tempAsks.back()->quantity) + "}";
            tempAsks.pop_back();
            if (!tempAsks.empty())
            {
                orderBookStr += ", ";
            }
        }
        orderBookStr += "]}";
        return orderBookStr;
    }
} // namespace Exchange
