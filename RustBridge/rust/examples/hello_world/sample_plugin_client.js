const WebSocket = require('ws');
const client = new WebSocket("ws://127.0.0.1:55555/Service/hello_world", ["jsonrpc"]);

next_id = 10

function send_message() {
  let req = {};
  req.jsonrpc = "2.0"
  req.id = next_id++

  if (next_id % 2 == 0) {
    req.method = "settings.onRequestSettings",
    req.params = [1, 2, 3, 4, 5];
  }
  else {
    req.method = "keyboard.onKeyPress",
    req.params = { 
      a: "a",
      b: "b"
    }
  }

  const s = JSON.stringify(req);
  console.log("send:" + s)
  client.send(s)
}

client.onopen = function(e) {
  send_message()
}

client.onmessage = function(msg) {
  console.log("recv:" + msg.data)
  const req = JSON.parse(msg.data)
  // console.log("recv:" + JSON.stringify(req))
  setTimeout(send_message, 1000)
}
