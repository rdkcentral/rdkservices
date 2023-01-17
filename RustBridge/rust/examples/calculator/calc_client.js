const WebSocket = require('ws');
const client = new WebSocket("ws://127.0.0.1:55555/Service/calculator", ["jsonrpc"]);

function send_message() {
  let req = {};
  req.jsonrpc = "2.0"
  req.id = 5;
  req.method = "calculator.add";
  req.params = [2, 2]

  const s = JSON.stringify(req);
  console.log("send:" + s)
  client.send(s)
}

client.onopen = function(e) {
  send_message()
}

client.onmessage = function(msg) {
  console.log("recv:" + msg.data)
  client.close();
}
