import unittest
import osmosiswrapper
import os
import stat
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
        self.client.checkout("yuvu", md5=True)
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

    def test_RestoresPermission_ContentDidChange(self):
        self.client.writeFile("theFile", "theContents")
        os.chmod(self.client.abspath("theFile"), 0741)
        self.client.checkin("yuvu")
        self.client.writeFile("theFile", "the other Contents")
        os.chmod(self.client.abspath("theFile"), 0777)
        self.client.checkout("yuvu")
        self.assertEquals(self.client.readFile("theFile"), "theContents")
        self.assertEquals(os.stat(self.client.abspath("theFile")).st_mode & 0777, 0741)

    def test_RestoreInADifferentDirectory(self):
        self.client.writeFile("theFile", "theContents")
        os.chmod(self.client.abspath("theFile"), 0741)
        self.client.checkin("yuvu")

        client = osmosiswrapper.Client(self.server)
        try:
            client.checkout("yuvu")
            self.assertEquals(client.readFile("theFile"), "theContents")
            self.assertEquals(os.stat(client.abspath("theFile")).st_mode & 0777, 0741)
        finally:
            client.clean()

    def test_DoesNotRemoveOtherExistingFiles(self):
        self.client.writeFile("theFile", "theContents")
        os.chmod(self.client.abspath("theFile"), 0741)
        self.client.checkin("yuvu")

        self.client.writeFile("another", "other contents")
        self.client.checkout("yuvu")
        self.assertEquals(self.client.fileCount(), 2)
        self.assertEquals(self.client.readFile("theFile"), "theContents")
        self.assertEquals(self.client.readFile("another"), "other contents")

    def test_RemoveOtherExistingFiles(self):
        self.client.writeFile("theFile", "theContents")
        os.chmod(self.client.abspath("theFile"), 0741)
        self.client.checkin("yuvu")

        self.client.writeFile("another", "other contents")
        self.client.checkout("yuvu", removeUnknownFiles=True)
        self.assertEquals(self.client.fileCount(), 1)
        self.assertEquals(self.client.readFile("theFile"), "theContents")

    def test_CheckoutNonexistingDoesNotWork(self):
        message = self.client.failedCheckout("yuvu")
        self.assertIn("not exist", message.lower())

    def test_FifoFile(self):
        os.mkfifo(self.client.abspath("aFifo"))
        self.client.checkin("yuvu")
        os.unlink(self.client.abspath("aFifo"))
        self.client.checkout("yuvu")
        self.assertEquals(self.client.fileCount(), 1)
        self.assertTrue(stat.S_ISFIFO(os.stat(self.client.abspath("aFifo")).st_mode))

    def test_SpecialFile(self):
        if (os.getuid() != 0):
            print "SKIPPING test that requires root permissions"
            return
        os.mknod(self.client.abspath("aDevice"), 0600 | stat.S_IFCHR, os.makedev(123, 45))
        self.client.checkin("yuvu")
        os.unlink(self.client.abspath("aDevice"))
        self.client.checkout("yuvu")
        self.assertEquals(self.client.fileCount(), 1)
        self.assertTrue(stat.S_ISCHR(os.stat(self.client.abspath("aDevice")).st_mode))
        self.assertEquals(os.stat(self.client.abspath("aDevice")).st_rdev, os.makedev(123, 45))

    def test_EmptyFile(self):
        self.client.writeFile("emptyFile", "")
        self.assertEquals(self.client.fileCount(), 1)
        self.client.checkin("yuvu")
        os.unlink(self.client.abspath("emptyFile"))
        self.client.checkout("yuvu")
        self.assertEquals(self.client.fileCount(), 1)
        self.assertEquals(self.client.readFile("emptyFile"), "")

if __name__ == '__main__':
    unittest.main()
