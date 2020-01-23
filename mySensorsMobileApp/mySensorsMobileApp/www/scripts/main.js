var ledState = "0";
var ipAdd = "192.168.4.1";
var flightSocket;

var g = new JustGage({
	id: "gauge",
	value: 0,
	min: -20,
	max: 20,
	gaugeColor: "#52181d",
	levelColors: ["#a80817"],
	gaugeWidthScale: 0.5,
	valueFontColor: "#fff",
	labelFontColor: "#fff",
	label: "°C",
	labelMinFontSize: 30
});

var h = new JustGage({
	id: "gaugeHumi",
	value: 50,
	min: 0,
	max: 100,
	gaugeColor: "#10253f",
	levelColors: ["#5f92d2"],
	gaugeWidthScale: 0.5,
	valueFontColor: "#fff",
	labelFontColor: "#fff",
	label: "%",
	labelMinFontSize: 20
});


$(".gage2").css("transform", "rotate(180deg)");
$(".gage2 text").eq(1).css("transform", "");

$(".startBtn").click(function () {
	flightSocket = new WebSocket("ws://" + ipAdd, "arduino");

	flightSocket.onerror = function (event) {
		flightSocket.close();
		flightSocket = new WebSocket("ws://" + ipAdd, "arduino");
	}

	flightSocket.onmessage = function (event) {
		var sensData = event.data;
		if (sensData.indexOf('!')!=-1) {
			var s = sensData.split('!');

			g.refresh(parseInt(s[0]),40);
			h.refresh(parseInt(s[1]),100);
		}
	}

	$(".greeting").slideUp(100);
	$(".content").css("height","90vh");
})


$(".lBtn").click(function () {
	if ($(this).attr("data-light") == 1) {
		$(this).attr("data-light","0")
	} else{
		$(this).attr("data-light","1")
	}
	ledState = $(this).attr("data-light");
	flightSocket.send(ledState);
})