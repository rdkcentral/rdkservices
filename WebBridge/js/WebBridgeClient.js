var WebSocket = require("ws");

const soc = new WebSocket("ws://localhost:55555/jsonrpc", "json");

var arg_x = 2;
var arg_y = 2;
var seq_no = 1;

function sendTriggerLowBatteryTest() {
  let req = {};
  req.jsonrpc = "2.0";
  req.id = seq_no++;
  req.method = "org.rdk.Calculator.1.triggerLowBatteryTest";

  const s = JSON.stringify(req);
  console.log("send:" + s);
  soc.send(s);
}

function sendAddRequest() {
  let req = {};
  req.jsonrpc = "2.0";
  req.id = seq_no++;
  req.method = "org.rdk.Calculator.1.add";
  req.params = [ arg_x, arg_y ];
  arg_x++;
  arg_y++;

  const s = JSON.stringify(req);
  console.log("send:" + s)
  soc.send(s)
}

function sendRegister() {
  let req = {};
  req.jsonrpc = "2.0";
  req.id = seq_no++;
  req.method = "org.rdk.Calculator.1.register";
  req.params = { "event" : "low_battery", "id" : "charles_root" };

  const s = JSON.stringify(req);
  console.log("send:" + s);
  soc.send(s);
}

soc.onopen = function(e) {
  console.log("soc connected");
  sendRegister();
  sendTriggerLowBatteryTest();
}
soc.onmessage = function(msg) {
  const req = JSON.parse(msg.data)
  console.log("recv:" + JSON.stringify(req))
}
