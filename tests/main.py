import unittest
import osmosiswrapper
import os
import stat
import fakeservers
import shutil
import time


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
        if os.getuid() != 0:
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

    def test_SparseFile(self):
        with open(self.client.abspath("sparseFile"), "wb") as f:
            f.write("something")
            f.seek(128*1024)
            f.write("something else")
        self.client.testHash("sparseFile")
        self.assertLessEqual(os.stat(self.client.abspath("sparseFile")).st_blocks * 512, 16 * 1024)
        self.client.checkin("pash")
        os.unlink(self.client.abspath("sparseFile"))
        self.client.checkout("pash")
        self.assertLessEqual(os.stat(self.client.abspath("sparseFile")).st_blocks * 512, 16 * 1024)
        self.assertEquals(
            self.client.readFile("sparseFile"),
            "something" + ("\0" * (128 * 1024 - len("something"))) + "something else")

    def test_mtime(self):
        self.client.writeFile("emptyFile", "")
        originalMtime = int(os.stat(self.client.abspath("emptyFile")).st_mtime)
        self.assertEquals(self.client.fileCount(), 1)
        self.client.checkin("yuvu")
        time.sleep(1)
        self.client.writeFile("emptyFile", "")
        self.assertNotEquals(os.stat(self.client.abspath("emptyFile")).st_mtime, originalMtime)
        self.client.checkout("yuvu")
        self.assertEquals(self.client.fileCount(), 1)
        self.assertEquals(self.client.readFile("emptyFile"), "")
        self.assertEquals(os.stat(self.client.abspath("emptyFile")).st_mtime, originalMtime)

    def test_chwon(self):
        if os.getuid() != 0:
            print "SKIPPING test that requires root permissions"
            return
        self.client.writeFile("emptyFile", "")
        os.chown(self.client.abspath("emptyFile"), 10, 11)
        self.client.checkin("yuvu")
        os.unlink(self.client.abspath("emptyFile"))
        self.client.checkout("yuvu")
        self.assertEquals(self.client.readFile("emptyFile"), "")
        self.assertEquals(os.stat(self.client.abspath("emptyFile")).st_uid, 10)
        self.assertEquals(os.stat(self.client.abspath("emptyFile")).st_gid, 11)

    def test_chwon_useMyUIDGID(self):
        if os.getuid() != 0:
            print "SKIPPING test that requires root permissions"
            return
        self.client.writeFile("emptyFile", "")
        os.chown(self.client.abspath("emptyFile"), 10, 11)
        self.client.checkin("yuvu")
        os.unlink(self.client.abspath("emptyFile"))
        self.client.checkout("yuvu", myUIDandGIDcheckout=True)
        self.assertEquals(self.client.readFile("emptyFile"), "")
        self.assertEquals(os.stat(self.client.abspath("emptyFile")).st_uid, 0)
        self.assertEquals(os.stat(self.client.abspath("emptyFile")).st_gid, 0)

    def test_bugfix_CheckInMultipleCopies(self):
        self.client.writeFile("empty1", "")
        self.client.writeFile("empty2", "")
        self.client.writeFile("contents1", "the contents")
        self.client.writeFile("contents2", "the contents")
        self.client.checkin("yuvu")
        for filename in ['empty1', 'empty2', 'contents1', 'contents2']:
            os.unlink(self.client.abspath(filename))

        self.client.checkout("yuvu")
        self.assertEquals(self.client.fileCount(), 4)
        self.assertEquals(self.client.readFile("empty1"), "")
        self.assertEquals(self.client.readFile("empty2"), "")
        self.assertEquals(self.client.readFile("contents1"), "the contents")
        self.assertEquals(self.client.readFile("contents2"), "the contents")

    def test_bugfix_BigDirList(self):
        for i in xrange(1000):
            self.client.writeFile("empty%d" % i, "")
        self.client.checkin("yuvu")
        self.client.checkout("yuvu")
        self.assertEquals(self.client.fileCount(), 1000)
        for i in xrange(1000):
            self.assertEquals(self.client.readFile("empty%d" % i), "")

    def test_DirectoryChangedPermissions(self):
        os.makedirs(self.client.abspath("directory"))
        self.client.writeFile("directory/theFile", "theContents")
        self.client.checkin("yuvu")
        before = os.stat(self.client.abspath("directory")).st_mode
        self.assertNotEquals(before & 0777, 0700)
        os.chmod(self.client.abspath("directory"), 0700)
        self.client.checkout("yuvu")
        self.assertEquals(self.client.fileCount(), 2)
        self.assertEquals(self.client.readFile("directory/theFile"), "theContents")
        self.assertEquals(os.stat(self.client.abspath("directory")).st_mode, before)

    def test_ListLabels(self):
        self.assertEquals(self.client.listLabels(), [])
        self.assertEquals(self.client.listLabels("yu.*"), [])
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")
        self.assertEquals(self.client.listLabels(), ["yuvu"])
        self.assertEquals(self.client.listLabels("yu.*"), ["yuvu"])
        self.assertEquals(self.client.listLabels(".*nothing.*"), [])

    def test_DeleteALabel(self):
        self.assertEquals(self.client.listLabels(), [])
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")
        self.assertEquals(self.client.listLabels(), ["yuvu"])
        self.client.writeFile("anotherFile", "778899")
        self.client.checkin("pash")
        self.assertEquals(set(self.client.listLabels()), set(["yuvu", "pash"]))
        before = self.server.fileCount()
        self.client.eraseLabel("pash")
        self.assertEquals(self.client.listLabels(), ["yuvu"])
        self.client.checkout("yuvu", removeUnknownFiles=True)
        self.assertEquals(self.client.fileCount(), 1)
        self.assertEquals(self.client.readFile("aFile"), "123456")
        after = self.server.fileCount()
        self.assertEquals(after, before - 3)

    def test_CheckoutUsingDelayedLabel(self):
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")
        os.unlink(self.client.abspath("aFile"))
        self.client.checkoutUsingDelayedLabel("yuvu")
        self.assertEquals(self.client.fileCount(), 1)
        self.assertEquals(self.client.readFile("aFile"), "123456")


if __name__ == '__main__':
    unittest.main()
