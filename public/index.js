var orderBook = new Vue({
    el: "#order-book",
    data: {
        orderbook: [],
        price: null
    },
    mounted: function () {
        let self = this;
        this.fetchOrderBook();
        this.fairPrice();
        setInterval(function() {
            self.fetchOrderBook();
            self.fairPrice();
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
        },
        fairPrice: function() {
            let xhttp = new XMLHttpRequest();
            let self = this;
            xhttp.onreadystatechange = function () {
                if (this.readyState === 4 && this.status === 200) {
                    var response = JSON.parse(this.responseText);
                    self.price = response; // Assuming you want to map the response items
                }
            };
            xhttp.open("GET", `http://192.168.1.108:9999/fair-price?instrument-name=GOOGL`);
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
        price: null,
        quantity: null,
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
            let queryParams = `
instrument-name=${encodeURIComponent(self.instrumentName)}&
trader-id=${encodeURIComponent(self.traderId)}&
order-type=${encodeURIComponent(self.orderType)}&
price=${encodeURIComponent(self.price)}&
quantity=${encodeURIComponent(self.quantity)}&
expiry-time=${encodeURIComponent(self.expiryTime)}`;
            xhttp.open("POST", `http://192.168.1.108:9999/order?${queryParams}`);
            xhttp.setRequestHeader("Content-type", "application/json");
            xhttp.send();
        },
    }
});
