const WebSocket = require("ws")

const request_state = {
  PENDING: 'pending'
};

class JsonRpcClient
{
  constructor(uri, protocols) {
    this._uri = uri;
    this._websocket = null;
    this._next_request_id = 1;
    this._outstanding_requests = {};
    this._json_rpc_version = "2.0";
    this._sub_protocols = protocols;
  }

  open() {
    let promise = new Promise((resolve, reject) => {
      const self = this;
      self._websocket = new WebSocket(self._uri, self._sub_protocols);
      self._websocket.onopen = function(e) {
        resolve(e);
      };
      self._websocket.onerror = function(e) {
        reject(e);
      };
      self._websocket.onmessage = function(e) {
        self._onIncomingMessage(e);
      };
    });
    return promise;
  }

  sendRequest(method_name, method_params, user_data) {
    const self = this;
    const request_id = self._next_request_id++;
    const request_message = {
      jsonrpc: self._json_rpc_version,
      id: request_id,
      method: method_name,
      params: method_params
    };

    let async_ctx = {};
    async_ctx.state = request_state.PENDING;
    async_ctx.user_data = user_data;
    async_ctx.resolve = null;
    async_ctx.reject = null;

    let promise = new Promise((resolve, reject) => {
      async_ctx.resolve = resolve;
      async_ctx.reject = reject;
    });

    self._outstanding_requests[request_id] = async_ctx;
    self.send(request_message);

    return promise;
  }

  notify(event_data) {
    send(event_data);
  }

  send(message) {
    const self = this;
    const json_text = JSON.stringify(message);
    console.log(">>> " + json_text);
    self._websocket.send(json_text);
  }

  _onIncomingMessage(e) {
    const self = this;
    console.log("<<< " + e.data);
    const json = JSON.parse(e.data);
    if (json.id in self._outstanding_requests) {
      const async_ctx = self._outstanding_requests[json.id];
      if (async_ctx.state == request_state.PENDING) {
        const e_res = {};
        e_res.response = json;
        e_res.user_data = async_ctx.user_data;
        async_ctx.resolve(e_res);
      }
      delete self._outstanding_requests[json.id];
    }
    else {
      // TODO: we just got a message with an id that isn't in the 
      // outstanding requests map
    }
  }
}

class Service {
  constructor(name) {
    this.service_name = name;
    this.methods = {};
    this._send_event_function = function() {};
  }

  registerMethod(method_name, method_version, method) {
    const self = this;
    const fq_method_name = method_version + "." + method_name;
    self.methods[fq_method_name] = method;
  }

  notify(event_data) {
  }

  _callMethodByName(method_name, json_rpc_params) {
    const self = this;
    if (self.methods[method_name]) {
      return self.methods[method_name](json_rpc_params);
    }
    else {
      return Promise.reject("method " + method_name + " not found");
    }
  }
}

class ServiceManager {
  constructor(conf) {
    this._json_rpc_client = null;
    this._services = {};
    this._conf = null;
  }

  open(conf) {
    const self = this;
    self._conf = conf;

    const uri = "ws:" + self._conf.host + ":" + self._conf.port + "/jsonrpc";
    self._json_rpc_client = new JsonRpcClient(uri, ["json"]);
    return self._json_rpc_client.open();
  }

  registerService(service) {
    const self = this;
    self._services[service.service_name] = {
      service: service,
      websocket: null,
      send: null
    };
    self._controllerCloneService(service.service_name).then((e) => {
      if (e.response.result) {
        self._controllerActivateService(e.user_data).then((e) => {
          self._connectService(e.user_data);
        });
      }
      else {
        // TODO:
      }
    });
  }

  _connectService(service_name) {
    const self = this;
    const service = self._services[service_name];
    const service_endpoint = "ws://" + this._conf.host + ":" + this._conf.port + "/Service/" + service_name;
    service.websocket = new WebSocket(service_endpoint, ["json"]);
    service.send = function(obj) {
      const json_text = JSON.stringify(obj);
      console.log(">>> " + json_text);
      service.websocket.send(json_text);
    }
    service.websocket.onopen = function(e) {
      const method_list = Object.keys(service.service.methods);
      const params = {
        jsonrpc: "2.0",
        method: "1.registerjsonrpcmethods",
        params: method_list
      };
      service.send(params);
    }
    service.websocket.onmessage = function(e) {
      let res = {};
      res.jsonrpc = "2.0";

      let req = null;
      try {
        req = JSON.parse(e.data);
        res.id = req.id;
        service.service._callMethodByName(req.method, req.params).then(value => {
          res.result = value;
          self._sendResponse(service.websocket, res);
        }).catch(ex => {
          res.error = ex;
          try {
            self._sendResponse(service.websocket, res);
          }
          catch (send_ex) {
            console.log(send_ex);
          }
        });
      }
      catch (dispatch_ex) {
        console.log(send_ex.stack);
      }
    };
  }

  _controllerActivateService(service_name) {
    const self = this;
    const params = {
      callsign: service_name
    };
    return self._json_rpc_client.sendRequest("Controller.1.activate", params, service_name);
  }

  _sendResponse(soc, res) {
    const json_text = JSON.stringify(res);
    console.log("<<<" + json_text);
    soc.send(json_text);
  }

  _controllerCloneService(service_name) {
    const self = this;
    const params = {
      callsign: "WebBridge",
      newcallsign: service_name
    };
    return self._json_rpc_client.sendRequest("Controller.1.clone", params, service_name);
  }
}

exports.Service = Service
exports.ServiceManager = ServiceManager
