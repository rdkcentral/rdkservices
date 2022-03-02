import socket

HOST = "127.0.0.1"
PORT = 9998

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    conn, addr = s.accept()
    with conn:
        print(f"Connected by {addr}")
        while True:
            data = conn.recv(1024)
            print("Recive request: ", data)
            if not data:
                break

            if data.find(b"GET /jsonrpc/?token=SecurityToken") != -1:
                conn.sendall(b"HTTP/1.1 101 SWITCH_PROTOCOL\
                    \r\nConnection: UPGRADE\
                    \r\nUpgrade: WEBSOCKET\
                    \r\nSec-WebSocket-Accept: UKJ4wE9ClN9OW90eWhC/afpNyvM=\r\nSec-WebSocket-Protocol: JSON\
                    \r\nSec-WebSocket-Version: 13\
                    \r\nContent-Length: 0\r\n\r\n")
            elif data.find(b"status@org.rdk.HdmiInput") != -1:
                conn.sendall(b"\x81~\x00\xd4{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":[{\"callsign\":\"org.rdk.HdmiInput\",\"locator\":\"libWPEFrameworkHdmiInput.so\",\"classname\":\"HdmiInput\",\"autostart\":false,\"precondition\":[\"Platform\"],\"state\":\"activated\",\"observers\":0}]}")
            elif not data: 
                # Close connection when finished
                break
                
