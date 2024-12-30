#include "Exchange.h"

int main( int argc, char* argv[])
{
    Exchange::Exchange exchange;
    exchange.addInstrument("AAPL", 0.01);
    exchange.addInstrument("GOOGL", 0.01);
    exchange.addInstrument("MSFT", 0.01);
    exchange.addInstrument("AMZN", 0.01);
    exchange.addInstrument("TSLA", 0.01);
    exchange.addInstrument("NDQ", 0.01);

    if(argc == 3)
    {
        exchange.StartServer(argv[1], std::stoi(argv[2]));
    }
    else if(argc == 2)
    {
        exchange.StartServer("localhost", std::stoi(argv[1]));
    }
    else
    {
        std::cout << "Usage: " << "./exchange_server [<hostname>] <port>" << std::endl;
    }
    return 0;
}