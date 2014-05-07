import unittest
import osmosiswrapper
import os
import fakeservers
import shutil


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

    def test_CheckInOneFile_MD5(self):
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu", md5=True)
        self.client.writeFile("aFile", "1234567")
        self.client.checkout("yuvu")
        self.assertEquals(self.client.fileCount(), 1)
        self.assertEquals(self.client.readFile("aFile"), "123456")

    def test_RestoreFileInASubDirectory(self):
        os.makedirs(self.client.abspath("first/second"))
        self.client.writeFile("first/second/theFile", "theContents")
        self.client.checkin("yuvu")
        self.assertEquals(self.client.fileCount(), 3)
        shutil.rmtree(self.client.abspath("first"))
        self.assertEquals(self.client.fileCount(), 0)
        self.client.checkout("yuvu")
        self.assertEquals(self.client.fileCount(), 3)
        self.assertEquals(self.client.readFile("first/second/theFile"), "theContents")

    def test_RestoresPermission_ContentDidNotChange(self):
        self.client.writeFile("theFile", "theContents")
        os.chmod(self.client.abspath("theFile"), 0741)
        self.client.checkin("yuvu")
        os.chmod(self.client.abspath("theFile"), 0777)
        self.client.checkout("yuvu")
        self.assertEquals(os.stat(self.client.abspath("theFile")).st_mode & 0777, 0741)

# restore elsewhere
# restore permissions content did change
# remove existing
# not remove existing
# test checkout non existing does not work
# test checkin double does not work
# test emptyfile

if __name__ == '__main__':
    unittest.main()
