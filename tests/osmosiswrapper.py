import shutil
import tempfile
import subprocess
import socket
import logging
import os
import time


class Client:
    def __init__(self, server, server2=None):
        self._server = server
        self._server2 = server2
        self._path = tempfile.mkdtemp()
        self.objectStores = ["127.0.0.1:%d" % server.port()]
        self.additionalObjectStoresForCheckout = []
        if server2 is not None:
            self.additionalObjectStoresForCheckout.append("127.0.0.1:%d" % server2.port())

    def clean(self):
        shutil.rmtree(self._path, ignore_errors=True)

    def path(self):
        return self._path

    def checkin(self, label, **kwargs):
        return self._run("checkin", self._path, label, * self._moreArgs(kwargs))

    def failedCheckin(self, label, **kwargs):
        return self._failedRun("checkin", self._path, label, * self._moreArgs(kwargs))

    def checkout(self, label, **kwargs):
        return self._run("checkout", self._path, label, * self._moreArgs(kwargs))

    def failedCheckout(self, label, **kwargs):
        return self._failedRun("checkout", self._path, label, * self._moreArgs(kwargs))

    def checkoutUsingDelayedLabel(self, label):
        popen = subprocess.Popen(
            ["build/cpp/osmosis.bin",
                "--objectStores=" + "+".join(self.objectStores + self.additionalObjectStoresForCheckout),
                "checkout",
                self._path,
                "+"],
            close_fds=True,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT)
        time.sleep(0.5)
        popen.stdin.write(label + "\n")
        output = popen.stdout.read()
        result = popen.wait()
        popen.stdin.close()
        popen.stdout.close()
        if result != 0:
            logging.error("\n\n\nClientOutput:\n" + output)
            logging.error("\n\n\nServerOutput:\n" + self._server.readLog())
            if self._server2 is not None:
                logging.error("\n\n\nServer2Output:\n" + self._server2.readLog())
            raise Exception("checkoutUsingDelayedLabel failed")
        return output

    def listLabels(self, regex=None):
        args = []
        if regex is not None:
            args.append(regex)
        result = self._run("listlabels", * args)
        labels = result.strip().split("\n")
        if "" in labels:
            labels.remove("")
        return labels

    def eraseLabel(self, label, **kwargs):
        return self._run("eraselabel", label, * self._moreArgs(kwargs))

    def renameLabel(self, labelBefore, labelAfter):
        return self._run("renamelabel", labelBefore, labelAfter)

    def testHash(self, filename):
        absolute = os.path.join(self._path, filename)
        hash1 = self._run("testhash", absolute).strip()
        hash2 = self._runAny("sha1sum", absolute).strip().split(" ")[0]
        if hash1 != hash2:
            raise Exception("Hashes not equal:\n" + hash1 + "\n" + hash2)

    def _moreArgs(self, kwargs):
        moreArgs = []
        if kwargs.get('removeUnknownFiles', False):
            moreArgs.append("--removeUnknownFiles")
        if kwargs.get('md5', False):
            moreArgs.append("--MD5")
        if kwargs.get('myUIDandGIDcheckout', False):
            moreArgs.append("--myUIDandGIDcheckout")
        if kwargs.get('putIfMissing', False):
            moreArgs.append("--putIfMissing")
        return moreArgs

    def _runAny(self, *args):
        try:
            return subprocess.check_output(
                args, close_fds=True, stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as e:
            logging.exception("\n\n\nOutput:\n" + e.output)
            raise

    def _run(self, cmd, *args):
        objectStores = list(self.objectStores)
        if cmd == "checkout":
            objectStores += self.additionalObjectStoresForCheckout
        try:
            return subprocess.check_output(
                ["build/cpp/osmosis.bin", "--objectStores=" + "+".join(objectStores), cmd] + list(args),
                close_fds=True, stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as e:
            logging.exception("\n\n\nClientOutput:\n" + e.output)
            logging.error("\n\n\nServerOutput:\n" + self._server.readLog())
            if self._server2 is not None:
                logging.error("\n\n\nServer2Output:\n" + self._server2.readLog())
            raise

    def _failedRun(self, cmd, *args):
        objectStores = list(self.objectStores)
        if cmd == "checkout":
            objectStores += self.additionalObjectStoresForCheckout
        try:
            return subprocess.check_output(
                ["build/cpp/osmosis.bin", "--objectStores=" + "+".join(objectStores), cmd] + list(args),
                close_fds=True, stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as e:
            return e.output
        else:
            raise Exception("running with args '%s' should have failed" % (args, ))

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
            count += len(files) + len(dirs)
        return count


class Server:
    def __init__(self):
        self._port = self._freePort()
        self.path = tempfile.mkdtemp()
        self._log = tempfile.NamedTemporaryFile()
        self._proc = subprocess.Popen([
            "build/cpp/osmosis.bin", "server", "--objectStoreRootPath=" + self.path,
            "--serverTCPPort=%d" % self._port], close_fds=True, stdout=self._log, stderr=self._log)

    def exit(self):
        self._proc.terminate()
        self._proc.wait()
        shutil.rmtree(self.path, ignore_errors=True)

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

    def fileCount(self):
        count = 0
        for root, dirs, files in os.walk(self.path):
            count += len(files) + len(dirs)
        return count
