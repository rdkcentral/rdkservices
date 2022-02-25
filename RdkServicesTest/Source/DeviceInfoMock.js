var mockServerClient = require('mockserver-client').mockServerClient;

mockServerClient("localhost", 10999).mockAnyResponse({
    "httpRequest": {
    	"method" : "POST",
    	"path" : "/",
   	"headers" : {
      	"content-length" : [ "31" ],
      		"Host" : [ "127.0.0.1:10999" ],
      		"Content-Type" : [ "application/x-www-form-urlencoded" ],
      		"Accept" : [ "*/*" ]
    	},
    	"keepAlive" : true,
    	"secure" : false,
    	"body" : "{\"paramList\":[{\"name\":\"test\"}]}"
    },
    "httpResponse": {
        "statusCode": 200,
        "headers": {
            "Location": [
                "https://www.mock-server.com"
            ]
        },
	"body" : "{\"paramList\":[\"Device.X_CISCO_COM_LED.RedPwm\":123]}"
    }
}).then(
    function () {
        console.log("expectation created");
    },
    function (error) {
        console.log(error);
    }
);
