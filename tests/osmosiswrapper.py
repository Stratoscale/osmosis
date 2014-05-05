import shutil
import tempfile
import subprocess
import socket
import logging
import os


class Client:
    def __init__(self, server):
        self._server = server
        self._path = tempfile.mkdtemp()

    def clean(self):
        shutil.rmtree(self._path, ignore_errors=True)

    def path(self):
        return self._path

    def checkin(self, label):
        return self._run("checkin", self._path, label)

    def checkout(self, label):
        return self._run("checkout", self._path, label)

    def _run(self, *args):
        try:
            return subprocess.check_output(
                ["build/cpp/osmosis.bin", "--serverTCPPort=%d" % self._server.port()] + list(args),
                close_fds=True, stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as e:
            logging.exception("\n\n\nClientOutput:\n" + e.output)
            logging.error("\n\n\nServerOutput:\n" + self._server.readLog())
            raise

    def abspath(self, relpath):
        return os.path.join(self._path, relpath)

    def readFile(self, relpath):
        with open(self.abspath(relpath), "rb") as f:
            return f.read()

    def writeFile(self, relpath, content):
        with open(self.abspath(relpath), "wb") as f:
            f.write(content)

    def fileCount(self):
        count = 0
        for root, dirs, files in os.walk(self._path):
            count += len(files)
        return count


class Server:
    def __init__(self):
        self._port = self._freePort()
        self._path = tempfile.mkdtemp()
        self._log = tempfile.NamedTemporaryFile()
        self._proc = subprocess.Popen([
            "build/cpp/osmosis.bin", "server", "--objectStoreRootPath=" + self._path,
            "--serverTCPPort=%d" % self._port], close_fds=True, stdout=self._log, stderr=self._log)

    def exit(self):
        self._proc.terminate()
        shutil.rmtree(self._path, ignore_errors=True)

    def readLog(self):
        with open(self._log.name, "r") as f:
            return f.read()

    def port(self):
        return self._port

    def _freePort(self):
        sock = socket.socket()
        try:
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            sock.bind(('localhost', 0))
            return sock.getsockname()[1]
        finally:
            sock.close()
