var vueInst = new Vue({
    el: "#google-order-book",
    data: {
        orderbook: []
    },
    mounted: function () {
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
    methods: {
    }
});