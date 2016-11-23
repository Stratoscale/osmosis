import shutil
import tempfile
import subprocess
import socket
import logging
import os
import time
import signal
import socket


class Client:
    def __init__(self, server, server2=None):
        self._server = server
        self._server2 = server2
        self._path = tempfile.mkdtemp()
        self._remoteObjectStore = "localhost:%d" % server.port()
        self.objectStores = None
        self.useRemoteObjectStoreOnly()
        self.objectStores = [self._remoteObjectStore]
        self.additionalObjectStoresForCheckout = []
        if server2 is not None:
            self.additionalObjectStoresForCheckout.append("localhost:%d" % server2.port())
        self.localObjectStorePath = tempfile.mkdtemp()

    def useLocalObjectStoreOnly(self):
        self.objectStores = [self.localObjectStorePath]

    def useRemoteObjectStoreOnly(self):
        self.objectStores = [self._remoteObjectStore]

    def useLocalAndRemoteObjectStores(self):
        self.objectStores = [self.localObjectStorePath, self._remoteObjectStore]

    def clean(self):
        shutil.rmtree(self._path, ignore_errors=True)
        shutil.rmtree(self.localObjectStorePath, ignore_errors=True)

    def path(self):
        return self._path

    def appendToPath(self, append):
        self._path += append

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
            self._printServerOutput()
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

    def transfer(self, label, destServer):
        return self._run("transfer", label, "--transferDestination=localhost:%d" % destServer.port())

    def testHash(self, filename):
        absolute = os.path.join(self._path, filename)
        hash1 = self._run("testhash", absolute).strip()
        hash2 = self._runAny("sha1sum", absolute).strip().split(" ")[0]
        if hash1 != hash2:
            raise Exception("Hashes not equal:\n" + hash1 + "\n" + hash2)
        return hash1

    def whoHasLabel(self, label, timeout=30):
        return self._run("whohaslabel",
                         label,
                         "--broadcastToLocalhost",
                         * self._moreArgs(dict(timeout=timeout,
                                               serverUDPPort=self._broadcastServerPort))).splitlines()

    def createAPathLargerThanPATH_MAX(self, maxAllowedPathSize):
        maxAllowedPathSize += 100
        origPath = os.getcwd()
        filePath = os.path.realpath(self._path)
        try:
            os.chdir(self._path)
            while len(filePath) < maxAllowedPathSize:
                os.mkdir("anotherDir")
                os.chdir("anotherDir")
                filePath = os.path.join(filePath, "anotherDir")
            open("fileWithALongPath.txt", "w").write("contents of a file with a long path")
        finally:
            os.chdir(origPath)
        return filePath

    @staticmethod
    def assertLargePathExists(fullPath):
        pathComponents = fullPath.split(os.path.sep)
        pathComponents = [component for component in pathComponents if component]
        pathSoFar = os.path.sep
        origPath = os.getcwd()
        try:
            os.chdir(pathSoFar)
            for pathComponent in pathComponents:
                os.chdir(pathComponent)
        finally:
            os.chdir(origPath)

    def setBroadcastServerPort(self, port):
        self._broadcastServerPort = port

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
        if kwargs.get('noChainTouch', False):
            moreArgs.append("--noChainTouch")
        if 'ignore' in kwargs:
            moreArgs.append("--ignore=" + kwargs['ignore'])
        if 'reportFile' in kwargs:
            moreArgs.append("--reportFile=" + kwargs['reportFile'])
        if 'timeout' in kwargs:
            moreArgs.append("--timeout=" + str(kwargs['timeout']))
        if 'serverUDPPort' in kwargs:
            moreArgs.append("--serverUDPPort=" + str(kwargs['serverUDPPort']))
        return moreArgs

    def _runAny(self, *args):
        try:
            return subprocess.check_output(
                args, close_fds=True, stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as e:
            logging.exception("\n\n\nOutput:\n" + e.output)
            raise

    def _printServerOutput(self):
        logging.error("\n\n\nServerOutput:\n" + self._server.readLog())
        if self._server2 is not None:
            logging.error("\n\n\nServer2Output:\n" + self._server2.readLog())

    def _run(self, cmd, *args):
        objectStores = list(self.objectStores)
        if cmd == "checkout":
            objectStores += self.additionalObjectStoresForCheckout
        cmd = ["build/cpp/osmosis.bin", "--objectStores=" + "+".join(objectStores), cmd] + list(args)
        try:
            return subprocess.check_output(cmd, close_fds=True, stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as e:
            logging.exception("\n\n\nClientOutput:\n" + e.output)
            self._printServerOutput()
            raise

    def _failedRun(self, cmd, *args):
        objectStores = list(self.objectStores)
        if cmd == "checkout":
            objectStores += self.additionalObjectStoresForCheckout
        try:
            subprocess.check_output(
                ["build/cpp/osmosis.bin", "--objectStores=" + "+".join(objectStores), cmd] + list(args),
                close_fds=True, stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as e:
            return e.output
        except KeyboardInterrupt:
            self._printServerOutput()
        else:
            raise Exception("running with args '%s' should have failed" % (args, ))

    def abspath(self, relpath):
        return os.path.join(self._path, relpath)

    def readFile(self, relpath):
        with open(self.abspath(relpath), "rb") as f:
            return f.read()

    def writeFile(self, relpath, content):
        if not os.path.isdir(os.path.dirname(self.abspath(relpath))):
            os.makedirs(os.path.dirname(self.abspath(relpath)))
        with open(self.abspath(relpath), "wb") as f:
            f.write(content)

    def createSymlink(self, relTargetPath, relLinkPath):
        os.symlink(self.abspath(relTargetPath), self.abspath(relLinkPath))

    def fileCount(self):
        count = 0
        for root, dirs, files in os.walk(self._path):
            count += len(files) + len(dirs)
        return count

    def insertMalformedFileInsteadOfDirInLocalObjectStore(self, dirPath, fileType):
        assert self.localObjectStorePath is not None
        dirPath = os.path.normpath(os.path.join(self.localObjectStorePath, dirPath))
        if os.path.exists(dirPath):
            shutil.rmtree(dirPath)
        else:
            os.makedirs(os.path.dirname(dirPath))
        if fileType == "fifo":
            print dirPath
            os.mkfifo(dirPath)
        elif fileType == "regular file":
            assert False
        elif fileType == "character special file":
            assert False
        elif fileType == "block special file":
            assert False
        elif fileType == "symbolic link":
            assert False
        elif fileType == "socket":
            sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            sock.bind(dirPath)
        else:
            assert False


class Server:
    def __init__(self):
        self._port = self._freePort()
        self.path = tempfile.mkdtemp()
        self._log = tempfile.NamedTemporaryFile()
        self._proc = subprocess.Popen([
            "build/cpp/osmosis.bin", "server", "--objectStoreRootPath=" + self.path,
            "--serverTCPPort=%d" % self._port], close_fds=True, stdout=self._log, stderr=self._log)
        self._waitForTCPServer()

    def exit(self):
        self._proc.terminate()
        self._proc.wait()
        shutil.rmtree(self.path, ignore_errors=True)

    def readLog(self):
        with open(self._log.name, "r") as f:
            return f.read()

    def port(self):
        return self._port

    def fileCount(self, excludeDir=None):
        count = 0
        for root, dirs, files in os.walk(self.path):
            if excludeDir in dirs:
                dirs.remove(excludeDir)
            count += len(files) + len(dirs)
        return count

    def injectMalformedObject(self, previousContent, malformedContent):
        for root, dirs, files in os.walk(self.path):
            for filename in files:
                with open(os.path.join(root, filename), "rb") as f:
                    contents = f.read()
                if contents != previousContent:
                    continue
                with open(os.path.join(root, filename), "wb") as f:
                    f.write(malformedContent)

    def labelLog(self, flush=True):
        if flush:
            os.kill(self._proc.pid, signal.SIGUSR1)
        output = subprocess.check_output([
            "build/cpp/osmosis.bin", "labellog", "--objectStoreRootPath=" + self.path], close_fds=True)
        return output.strip().split("\n")

    def purge(self):
        os.kill(self._proc.pid, signal.SIGUSR1)
        return subprocess.check_output([
            "build/cpp/osmosis.bin", "purge", "--objectStoreRootPath=" + self.path], close_fds=True)

    def leastRecentlyUsed(self, keep, maximumDiskUsage):
        os.kill(self._proc.pid, signal.SIGUSR1)
        return subprocess.check_output([
            "build/cpp/osmosis.bin", "leastrecentlyused",
            "--objectStoreRootPath=" + self.path,
            "--keep=" + keep,
            "--maximumDiskUsage=" + maximumDiskUsage], close_fds=True)

    def _freePort(self):
        sock = socket.socket()
        try:
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            sock.bind(('localhost', 0))
            return sock.getsockname()[1]
        finally:
            sock.close()

    def _waitForTCPServer(self, timeout=30, interval=0.1):
        before = time.time()
        while time.time() - before < timeout:
            if self._rawTCPConnect(("localhost", self._port)):
                return
            time.sleep(interval)
        raise Exception("Osmosis TCP Server 'localhost:%(port)s' did not respond within timeout" % dict(
            port=self._port))

    def _rawTCPConnect(self, tcpEndpoint):
        s = socket.socket()
        try:
            s.connect(tcpEndpoint)
            return True
        except:
            return False
        finally:
            s.close()


class BroadcastServer:
    def __init__(self, rootPath=None, port=None):
        self._port = self._freePort() if port is None else port
        self._rootPath = rootPath
        self._proc = None

    def start(self):
        self.path = tempfile.mkdtemp() if self._rootPath is None else self._rootPath
        self._log = tempfile.NamedTemporaryFile()
        self._proc = subprocess.Popen([
            "build/cpp/osmosis.bin", "broadcastserver", "--objectStoreRootPath=" + self.path,
            "--serverUDPPort=%d" % self._port], close_fds=True, stdout=self._log, stderr=self._log)
        self._waitForUDPServer()

    def exit(self):
        if self._proc is not None:
            self._proc.terminate()
            self._proc.wait()
            shutil.rmtree(self.path, ignore_errors=True)

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

    def _waitForUDPServer(self, timeout=30, interval=0.1):
        before = time.time()
        while time.time() - before < timeout:
            if self._rawUDPSend("localhost", self._port):
                return
            time.sleep(interval)
        raise Exception("Osmosis TCP Server 'localhost:%(port)s' did not respond within timeout" % dict(
            port=self._port))

    def _rawUDPSend(self, address, port):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        try:
            sock.sendto("\xFF\xFF", (address, port))
            return True
        except:
            return False
        finally:
            sock.close()
