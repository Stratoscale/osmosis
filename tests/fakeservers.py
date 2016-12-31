import os
import io
import ctypes
import struct
import socket
import logging
import binascii
import threading


DOES_EXIST_RESPONSE_YES = 1
DOES_EXIST_RESPONSE_NO = 2
OPCODE_GET = 1
OPCODE_DOES_EXIST = 3
OPCODE_GET_LABEL = 12
OPCODE_LIST_LABELS = 13
OPCODE_ACK = 0xAC
HASH_ALGORITHM_SHA1 = 2


class FakeServer(threading.Thread):
    def __init__(self):
        self._conn = None
        self._sock = socket.socket()
        self._sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._sock.bind(('localhost', 0))
        self._sock.listen(10)
        threading.Thread.__init__(self)
        self.daemon = True
        threading.Thread.start(self)

    def port(self):
        return self._sock.getsockname()[1]

    def hostname(self):
        return "localhost"

    def run(self):
        try:
            while True:
                self._conn, peer = self._sock.accept()
                self._serve()
        except:
            logging.exception("Fake Server")

    def readLog(self):
        return ""

    def _handshake(self):
        self._conn.recv(4096)
        self._conn.send(chr(OPCODE_ACK))


class FakeServerWithNegotiationLogic(FakeServer):
    def __init__(self, clientOfServerToImitate):
        super(FakeServerWithNegotiationLogic, self).__init__()
        self._client = clientOfServerToImitate
        self._handlerMethods = {OPCODE_LIST_LABELS: self._listLabels,
                                OPCODE_GET_LABEL: self._getLabel,
                                OPCODE_DOES_EXIST: self._doesExist}

    def _serve(self):
        self._handshake()
        try:
            while True:
                message = self._conn.recv(1)
                if not message:
                    break
                opcode = ord(message[0])
                if opcode not in self._handlerMethods:
                    break
                handler = self._handlerMethods[opcode]
                try:
                    handler()
                except StopIteration:
                    break
        finally:
            self._conn.close()

    def _sendStruct(self, _struct):
        buf = io.BytesIO()
        buf.write(_struct)
        self._conn.send(buf)

    def _receiveStruct(self, _struct):
        raw = self._conn.recv(1024)
        buf = io.BytesIO(raw)
        buf.readinto(_struct)
        residue = buf.read()
        return residue

    def _sendChunk(self, offset, payload):
        chunk = ChunkHeader(offset, len(payload))
        self._conn.send(chunk)
        if payload:
            self._conn.send(payload)

    def _sendEOF(self):
        self._sendChunk(0, "")

    def _receiveLabel(self):
        labelHeader = LabelHeader()
        label = self._receiveStruct(labelHeader)
        return label

    def _receiveHash(self):
        _hashStruct = Hash()
        self._receiveStruct(_hashStruct)
        _rawHash = "".join([chr(byte) for byte in _hashStruct.hash])
        _hexHash = binascii.hexlify(_rawHash)
        return _hexHash

    def _listLabels(self):
        regex = self._receiveLabel()
        for label in self._client.listLabels(regex=regex):
            self._sendChunk(offset=0, payload=label)
        self._sendEOF()

    def _getLabel(self):
        label = self._receiveLabel()
        labelFilePath = os.path.join(self._client._server.path, "labels", label)
        with open(labelFilePath) as labelFile:
            content = labelFile.read()
        _hash = binascii.unhexlify(content)
        hashStruct = Hash()
        io.BytesIO(chr(HASH_ALGORITHM_SHA1) + _hash).readinto(hashStruct)
        self._conn.send(hashStruct)

    def _doesExist(self):
        _hexHash = self._receiveHash()
        hashFilePath = os.path.join(self._client._server.path,
                                    _hexHash[:2],
                                    _hexHash[2:4],
                                    _hexHash[4:])
        doesHashExists = os.path.exists(hashFilePath)
        assert doesHashExists
        self._conn.send(chr(DOES_EXIST_RESPONSE_YES))

    def _doesExist(self):
        _hash = self._receiveHash()
        self._readFileByHash(_hash)
        self._conn.send(chr(DOES_EXIST_RESPONSE_YES))

    def _readFileByHash(self, _hash):
        hashFilePath = os.path.join(self._client._server.path, _hash[:2], _hash[2:4], _hash[4:])
        with open(hashFilePath) as hashFile:
            content = hashFile.read()
        return content


class FakeServerHangsUp(FakeServer):
    def __init__(self):
        super(FakeServerHangsUp, self).__init__()

    def _serve(self):
        self._conn.recv(4096)
        self._conn.close()


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

    def _serve(self):
        self._handshake()
        message = self._conn.recv(4096)
        opcode = ord(message[0])
        self._blockForever()

    def _blockForever(self):
        raw_input()


class FakeServerCloseAfterListLabelsOp(FakeServerWithNegotiationLogic):
    def __init__(self, clientOfServerToImitate):
        super(FakeServerCloseAfterListLabelsOp, self).__init__(clientOfServerToImitate)

    def _listLabels(self):
        raise StopIteration


class FakeServerCloseAfterGetOp(FakeServerWithNegotiationLogic):
    def __init__(self, clientOfServerToImitate):
        super(FakeServerCloseAfterGetOp, self).__init__(clientOfServerToImitate)
        self._handlerMethods[OPCODE_GET] = self._getFile
        self._getFileCounter = 0

    def _getFile(self):
        self._getFileCounter += 1
        _hash = self._receiveHash()
        content = self._readFileByHash(_hash)
        if self._getFileCounter == 1:
            self._sendChunk(offset=0, payload=content)
            self._sendEOF()
        else:
            halfContent = content[:len(content) / 2]
            assert len(halfContent) < len(content)
            assert len(halfContent) > 0
            self._sendChunk(offset=0, payload=halfContent)
            raise StopIteration


class FakeServerCloseAfterExistsOp(FakeServerWithNegotiationLogic):
    def __init__(self, clientOfServerToImitate):
        super(FakeServerCloseAfterExistsOp, self).__init__(clientOfServerToImitate)

    def _doesExist(self):
        self._conn.send("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00")
        self._conn.send("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00")
        raise StopIteration


class ChunkHeader(ctypes.Structure):
    _pack_ = 1
    _fields_ = [('offset', ctypes.c_size_t),
                ('bytes', ctypes.c_ushort)]


class LabelHeader(ctypes.Structure):
    _pack_ = 1
    _fields_ = [('length', ctypes.c_ushort)]


class Hash(ctypes.Structure):
    _pack_ = 1
    _fields_ = [('hashAlgorithm', ctypes.c_ubyte),
                ('hash', ctypes.c_ubyte * 20)]
