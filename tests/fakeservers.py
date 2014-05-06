import threading
import socket
import logging


class FakeServerHangsUp(threading.Thread):
    def __init__(self):
        self._sock = socket.socket()
        self._sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._sock.bind(('localhost', 0))
        self._sock.listen(10)
        threading.Thread.__init__(self)
        self.daemon = True
        threading.Thread.start(self)

    def port(self):
        return self._sock.getsockname()[1]

    def run(self):
        try:
            while True:
                conn, peer = self._sock.accept()
                conn.recv(4096)
                conn.close()
        except:
            logging.exception("Fake Server")
