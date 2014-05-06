import unittest
import osmosiswrapper
import os
import fakeservers


class Test(unittest.TestCase):
    def setUp(self):
        self.server = osmosiswrapper.Server()
        self.client = osmosiswrapper.Client(self.server)

    def tearDown(self):
        self.client.clean()
        self.server.exit()

    def test_ZeroFiles(self):
        self.client.checkin("yuvu")
        self.client.checkout("yuvu")
        self.assertEquals(self.client.fileCount(), 0)

    def test_CheckInOneFile(self):
        self.client.writeFile("aFile", "123456")
        self.assertEquals(self.client.fileCount(), 1)
        self.client.checkin("yuvu")
        self.client.checkout("yuvu")
        self.assertEquals(self.client.fileCount(), 1)
        self.assertEquals(self.client.readFile("aFile"), "123456")
        os.unlink(self.client.abspath("aFile"))
        self.assertEquals(self.client.fileCount(), 0)
        self.client.checkout("yuvu")
        self.assertEquals(self.client.fileCount(), 1)
        self.assertEquals(self.client.readFile("aFile"), "123456")

    def test_CheckInOneFile_ContentRestored(self):
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")
        self.client.writeFile("aFile", "1234567")
        self.client.checkout("yuvu")
        self.assertEquals(self.client.fileCount(), 1)
        self.assertEquals(self.client.readFile("aFile"), "123456")

    def test_HandshakeFails_NormalErrorMessage(self):
        server = fakeservers.FakeServerHangsUp()
        client = osmosiswrapper.Client(server)
        try:
            message = client.failedCheckin("yuvu")
            self.assertIn("handshake", message.lower())
        finally:
            self.client.clean()

    def test_DoubleCheckinFailed(self):
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")
        self.client.failedCheckin("yuvu")

        os.unlink(self.client.abspath("aFile"))
        self.client.checkout("yuvu")
        self.assertEquals(self.client.fileCount(), 1)
        self.assertEquals(self.client.readFile("aFile"), "123456")

    def test_TwoCheckIns(self):
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")
        self.client.writeFile("aFile", "1234567777")
        self.client.checkin("pash")

        self.client.checkout("yuvu")
        self.assertEquals(self.client.fileCount(), 1)
        self.assertEquals(self.client.readFile("aFile"), "123456")
        self.client.checkout("pash")
        self.assertEquals(self.client.fileCount(), 1)
        self.assertEquals(self.client.readFile("aFile"), "1234567777")

# test checkout non existing does not work
# test checkin double does not work
# test emptyfile

if __name__ == '__main__':
    unittest.main()
