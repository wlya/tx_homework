import socket

sk = socket.socket()
sk.bind(("0.0.0.0",8082))
sk.listen(5)
while True:
    conn,address = sk.accept()
    conn.send(bytes(address[0],encoding="utf-8"))
    conn.close()