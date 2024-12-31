var orderBook = new Vue({
    el: "#order-book",
    data: {
        orderbook: [],
    },
    mounted: function () {
        let self = this;
        this.fetchOrderBook();
        setInterval(function() {
            self.fetchOrderBook();
        }, 1000); // 1000 milliseconds = 1 second
    },
    methods: {
        fetchOrderBook: function() {
            let xhttp = new XMLHttpRequest();
            let self = this;
            xhttp.onreadystatechange = function () {
                if (this.readyState === 4 && this.status === 200) {
                    var response = JSON.parse(this.responseText);
                    self.orderbook = response; // Assuming you want to map the response items
                }
            };
            xhttp.open("GET", `http://192.168.1.108:9999/order-book?instrument-name=GOOGL`); // get trip with id
            xhttp.setRequestHeader("Content-type", "application/json");
            xhttp.send();
        }
    }
});

var orderForm = new Vue({
    el: "#order-form",
    data: {
        instrumentName: "",
        traderId: "",
        orderType: "",
        price: 0,
        quantity: 0,
        expiryTime: "",
    },
    methods: {
        submitOrder: function() {
            let xhttp = new XMLHttpRequest();
            let self = this;
            xhttp.onreadystatechange = function () {
                if (this.readyState === 4 && this.status === 200) {
                    alert("Order submitted successfully!");
                } else if (this.readyState === 4) {
                    alert("Error submitting order.");
                }
            };
            xhttp.open("POST", "http://192.168.1.108:9999/submit-order");
            xhttp.setRequestHeader("Content-type", "application/json");
            xhttp.send(JSON.stringify({
                instrumentName: self.instrumentName,
                traderId: self.traderId,
                orderType: self.orderType,
                price: self.price,
                quantity: self.quantity,
                expiryTime: self.expiryTime
            }));
        }
    }
});
