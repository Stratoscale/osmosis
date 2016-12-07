import threading
import socket
import logging


class FakeServer(threading.Thread):
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
                self._serve(conn, peer)
        except:
            logging.exception("Fake Server")

    def readLog(self):
        return ""

    def _handshake(self, conn):
        conn.recv(4096)
        conn.send(chr(self.OPCODE_ACK))


class FakeServerHangsUp(FakeServer):
    def __init__(self):
        super(FakeServerHangsUp, self).__init__()

    def _serve(self, conn, peer):
        conn.recv(4096)
        conn.close()


class FakeServerConnectTimeout(FakeServer):
    def __init__(self):
        super(FakeServerConnectTimeout, self).__init__()

    def port(self):
        return 15652

    def hostname(self):
        return "35.124.99.234"


class FakeServerNotSending(FakeServer):
    def __init__(self):
        super(FakeServerNotSending, self).__init__()

    def _serve(self, conn, peer):
        self._handshake(conn)
        message = conn.recv(4096)
        opcode = ord(message[0])
        print("Rreceived opcode: %d" % (opcode,))
        self._blockForever()


class FakeServerNotReceiving(FakeServer):
    def __init__(self):
        super(FakeServerNotReceiving, self).__init__()

    def _serve(self, conn, peer):
        self._handshake(conn)
        self._blockForever()


class FakeServerCloseAfterListLabelsOp(FakeServer):
    def __init__(self):
        super(FakeServerCloseAfterListLabelsOp, self).__init__()

    def _serve(self, conn, peer):
        self._handshake(conn)
        message = conn.recv(4096)
        opcode = ord(message[0])
        print("Rreceived opcode: %d" % (opcode,))
        conn.close()


class FakeServerCloseAfterExistsOp(FakeServer):
    def __init__(self):
        super(FakeServerCloseAfterExistsOp, self).__init__()

    def _serve(self, conn, peer):
        self._handshake(conn)
        message = conn.recv(4096)
        opcode = ord(message[0])
        print("Rreceived opcode: %d" % (opcode,))
        conn.send("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00")
        conn.send("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00")
        conn.close()
