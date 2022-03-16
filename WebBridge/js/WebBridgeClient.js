const WebSocket = require('ws');
const client = new WebSocket("ws://10.0.0.159:55555/jsonrpc", "json");
client.onopen = function(e) {
  console.log("client connected");

  let req = {};
  req.jsonrpc = "2.0"
  req.id = 10

  req.method = "org.rdk.Calculator.1.add";
  req.params = [1, 2, 3, 4, 5];

  const s = JSON.stringify(req);
  console.log("send:" + s)
  client.send(s)
}
client.onmessage = function(msg) {
  const req = JSON.parse(msg.data)
  console.log("recv:" + JSON.stringify(req))
}
