from http.server import BaseHTTPRequestHandler, HTTPServer


class S(BaseHTTPRequestHandler):
    def _set_headers(self):
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.end_headers()

    def do_POST(self):
        self._set_headers()
        self.wfile.write(b"{\"paramList\":[\"Device.X_CISCO_COM_LED.RedPwm\":123],\"success\":true}")


def run(port=80):
    httpd = HTTPServer(('127.0.0.1', port), S)
    httpd.serve_forever()


if __name__ == "__main__":
    run(10999)
