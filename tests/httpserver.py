import subprocess
import socket
import time


class HttpServer:
    def __init__(self, path):
        self._port = self._findPort()
        self._popen = subprocess.Popen(["python", "-m", "SimpleHTTPServer", str(self._port)], cwd=path)
        self._waitForTCPServer()

    def port(self):
        return self._port

    def url(self):
        return 'http://localhost:%d/' % self.port()

    def stop(self):
        self._popen.terminate()
        self._popen.wait()

    def _findPort(self):
        sock = socket.socket()
        try:
            sock.bind(("localhost", 0))
            return sock.getsockname()[1]
        finally:
            sock.close()

    def _waitForTCPServer(self):
        for i in xrange(10):
            time.sleep(0.05)
            sock = socket.socket()
            try:
                sock.connect(("localhost", self._port))
                return
            except:
                pass
            finally:
                sock.close()
        raise Exception("TCP server did not answer within timeout")
