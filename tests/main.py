import unittest
import osmosiswrapper
import os
import stat
import fakeservers
import httpserver
import shutil
import time
import logging
import fakeshell
import tempfile
import json
import subprocess
from osmosis.policy import cleanupremovelabelsuntildiskusage
from osmosis import objectstore


class Test(unittest.TestCase):
    def setUp(self):
        self.server = osmosiswrapper.Server()
        self.client = osmosiswrapper.Client(self.server)
        self.broadcastServer = osmosiswrapper.BroadcastServer(rootPath=self.server.path)
        self.client.setBroadcastServerPort(port=self.broadcastServer.port())

    def tearDown(self):
        self.client.clean()
        self.server.exit()
        if self.broadcastServer is not None:
            self.broadcastServer.exit()

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
        before = self.server.fileCount(excludeDir='labelLog')
        self.client.eraseLabel("pash")
        self.assertEquals(self.client.listLabels(), ["yuvu"])
        self.client.checkout("yuvu", removeUnknownFiles=True)
        self.assertEquals(self.client.fileCount(), 1)
        self.assertEquals(self.client.readFile("aFile"), "123456")
        after = self.server.fileCount(excludeDir='labelLog')
        self.assertEquals(after, before - 1)
        self.server.purge()
        after = self.server.fileCount(excludeDir='labelLog')
        self.assertEquals(after, before - 3)

    def test_CheckoutUsingDelayedLabel(self):
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")
        os.unlink(self.client.abspath("aFile"))
        self.client.checkoutUsingDelayedLabel("yuvu")
        self.assertEquals(self.client.fileCount(), 1)
        self.assertEquals(self.client.readFile("aFile"), "123456")

    def test_RenameLabel(self):
        self.client.writeFile("aFile", "123456")
        self.assertEquals(self.client.fileCount(), 1)
        self.client.checkin("yuvu")

        self.client.renameLabel("yuvu", "pash")
        os.unlink(self.client.abspath("aFile"))
        self.assertEquals(self.client.fileCount(), 0)
        self.client.checkout("pash")
        self.assertEquals(self.client.fileCount(), 1)
        self.assertEquals(self.client.readFile("aFile"), "123456")

        self.assertEquals(self.client.listLabels(), ["pash"])
        message = self.client.failedCheckout("yuvu")
        self.assertIn("not exist", message.lower())
        self.client.eraseLabel("pash")

    def test_joinedCheckout(self):
        self.client.writeFile("first", "1")
        self.client.checkin("yuvu")
        os.unlink(self.client.abspath("first"))

        self.client.writeFile("second", "2")
        self.client.checkin("pash")
        os.unlink(self.client.abspath("second"))

        self.client.writeFile("third", "3")
        self.client.checkin("mushu")
        os.unlink(self.client.abspath("third"))

        self.client.checkout("yuvu+pash+mushu")
        self.assertEquals(self.client.fileCount(), 3)
        self.assertEquals(self.client.readFile("first"), "1")
        self.assertEquals(self.client.readFile("second"), "2")
        self.assertEquals(self.client.readFile("third"), "3")

    def test_joinedCheckout_SomeFilesExistsInBoth(self):
        self.client.writeFile("first", "1")
        self.client.checkin("yuvu")

        self.client.writeFile("second", "2")
        self.client.checkin("pash")
        os.unlink(self.client.abspath("second"))
        os.unlink(self.client.abspath("first"))

        self.client.checkout("yuvu+pash")
        self.assertEquals(self.client.fileCount(), 2)
        self.assertEquals(self.client.readFile("first"), "1")
        self.assertEquals(self.client.readFile("second"), "2")

    def test_joinedCheckout_SomeFilesExistsInBoth_ButWithDifferentContents_CheckoutFails(self):
        self.client.writeFile("first", "1")
        self.client.checkin("yuvu")

        self.client.writeFile("first", "2")
        self.client.writeFile("second", "2")
        self.client.checkin("pash")
        os.unlink(self.client.abspath("second"))
        os.unlink(self.client.abspath("first"))

        message = self.client.failedCheckout("yuvu+pash")
        self.assertIn("join", message.lower())

    def test_checkoutWithBackupServerThatIsNotReallyUsed(self):
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")

        server2 = osmosiswrapper.Server()
        client = osmosiswrapper.Client(self.server, server2)
        try:
            client.checkout("yuvu")
        finally:
            client.clean()
            server2.exit()

    def test_checkoutWithBackupServerThatIsNotReallyUsed_ActuallyBackupIsDead(self):
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")

        server2 = osmosiswrapper.Server()
        client = osmosiswrapper.Client(self.server, server2)
        server2.exit()
        try:
            client.checkout("yuvu")
        finally:
            client.clean()

    def test_checkoutWithBackupServer(self):
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")

        server2 = osmosiswrapper.Server()
        client = osmosiswrapper.Client(server2, self.server)
        try:
            client.checkout("yuvu")
            self.assertEquals(client.listLabels(), [])
        finally:
            client.clean()
            server2.exit()

    def test_checkoutWithBackupServer_PutIfMissing(self):
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")

        server2 = osmosiswrapper.Server()
        client = osmosiswrapper.Client(server2, self.server)
        try:
            client.checkout("yuvu", putIfMissing=True)
            self.assertEquals(client.listLabels(), ["yuvu"])
            client2 = osmosiswrapper.Client(server2)
            try:
                client2.checkout("yuvu")
            finally:
                client2.clean()
        finally:
            client.clean()
            server2.exit()

    def test_bugfix_checkinTwiceWithDifferentLabels_LabelIsPutEvenThoughDirListObjectAlreadyExists(self):
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")
        self.client.checkin("pash")

    def test_checkinAndCheckoutWithANonRemoteObjectStore(self):
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")

        client = osmosiswrapper.Client(self.server)
        try:
            client.objectStores = [self.server.path]
            client.checkout('yuvu')
            self.assertEquals(client.fileCount(), 1)
            self.assertEquals(client.readFile("aFile"), "123456")
            client.writeFile("aFile", "something else")
            client.checkin('pash')
        finally:
            client.clean()

        self.client.checkout('pash')
        self.assertEquals(self.client.fileCount(), 1)
        self.assertEquals(self.client.readFile("aFile"), "something else")

    def test_IgnoreOnCheckOut(self):
        self.client.writeFile("firstFile", "123456")
        self.client.checkin("yuvu")
        os.unlink(self.client.abspath("firstFile"))
        self.client.writeFile("directory/secondFile", "223344")
        self.client.checkout("yuvu", ignore=self.client.abspath("directory"), removeUnknownFiles=True)
        self.assertEquals(self.client.fileCount(), 3)
        self.assertEquals(self.client.readFile("firstFile"), "123456")
        self.assertEquals(self.client.readFile("directory/secondFile"), "223344")

    def test_LocalObjectStoreInsideCheckout_Ignored(self):
        self.client.writeFile("firstFile", "123456")
        self.client.checkin("yuvu")
        os.unlink(self.client.abspath("firstFile"))

        client = osmosiswrapper.Client(self.server)
        try:
            client.objectStores = [client.abspath("var/lib/osmosis/objectstore")] + client.objectStores
            client.writeFile("willberemoved", "garbage")
            client.checkout(
                'yuvu', removeUnknownFiles=True, ignore=client.abspath("var/lib/osmosis"),
                putIfMissing=True)
            self.assertGreater(client.fileCount(), 1)
            self.assertEquals(client.readFile("firstFile"), "123456")
            self.assertFalse(os.path.exists(client.abspath("willberemoved")))

            client2 = osmosiswrapper.Client(self.server)
            try:
                client2.objectStores = [client.objectStores[0]]
                client.checkout("yuvu")
            finally:
                client2.clean()
        finally:
            client.clean()

    def test_TransferBetweenObjectStores(self):
        self.client.writeFile("firstFile", "123456")
        self.client.checkin("yuvu")
        os.unlink(self.client.abspath("firstFile"))

        server = osmosiswrapper.Server()
        try:
            self.client.transfer("yuvu", server)
            client = osmosiswrapper.Client(server)
            try:
                client.checkout("yuvu")
                self.assertEquals(client.fileCount(), 1)
                self.assertEquals(client.readFile("firstFile"), "123456")
            finally:
                client.clean()
        except:
            logging.error("Destination server log:\n%(log)s", dict(log=server.readLog()))
            raise
        finally:
            server.exit()

    def test_CheckInOneFile_SymbolicLinkRestored(self):
        os.symlink("/it", os.path.join(self.client.path(), "aLink"))
        self.client.checkin("yuvu")
        os.unlink(os.path.join(self.client.path(), "aLink"))
        os.symlink("/notit", os.path.join(self.client.path(), "aLink"))
        self.client.checkout("yuvu")
        self.assertEquals(self.client.fileCount(), 1)
        self.assertEquals(os.readlink(os.path.join(self.client.path(), "aLink")), "/it")

    def test_CheckoutTwoRepositories_NearestHasADefectiveCopy(self):
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")
        serverNear = osmosiswrapper.Server()
        client = osmosiswrapper.Client(serverNear, self.server)
        client.checkout("yuvu", putIfMissing=True)
        serverNear.injectMalformedObject("123456", "defective")
        os.unlink(os.path.join(client.path(), "aFile"))
        client.checkout("yuvu", putIfMissing=True)
        self.assertEquals(client.fileCount(), 1)
        self.assertEquals(client.readFile("aFile"), "123456")

    def test_CheckOut_AFileOverANonEmptyDirectory(self):
        self.client.writeFile("aFile", "123")
        self.client.checkin("yuvu")
        os.unlink(os.path.join(self.client.path(), "aFile"))
        os.mkdir(os.path.join(self.client.path(), "aFile"))
        self.client.writeFile("aFile/inSubdirectory", "555")
        self.client.checkout("yuvu")
        self.assertEquals(self.client.fileCount(), 1)
        self.assertEquals(self.client.readFile("aFile"), "123")

    def test_Bugfix_RemoveOtherExistingFiles_DoesNotRemoveSymlinksThatPointToNonExistingFiles(self):
        self.client.writeFile("theFile", "theContents")
        self.client.checkin("yuvu")

        self.client.writeFile("another", "other contents")
        os.symlink(
            os.path.join(self.client.path(), "another"),
            os.path.join(self.client.path(), "symlink"))
        os.unlink(os.path.join(self.client.path(), "another"))
        self.client.checkout("yuvu", removeUnknownFiles=True)
        self.assertEquals(self.client.fileCount(), 1)
        self.assertEquals(self.client.readFile("theFile"), "theContents")

    def test_BugFix_PutIfMissing_WhenDirListObjectAlreadyExist(self):
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")

        server2 = osmosiswrapper.Server()
        client = osmosiswrapper.Client(server2, self.server)
        try:
            client.checkout("yuvu", putIfMissing=True)
            self.assertEquals(client.listLabels(), ["yuvu"])
            client.eraseLabel("yuvu")
            client.checkout("yuvu", putIfMissing=True)
        finally:
            client.clean()
            server2.exit()

    def test_BugFix_TwoServers_LabelWithMoreCharactersExists(self):
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")
        self.client.checkin("yuv")

        server2 = osmosiswrapper.Server()
        client = osmosiswrapper.Client(server2, self.server)
        try:
            client.checkout("yuvu", putIfMissing=True)
            client.checkout("yuv", putIfMissing=True)
        finally:
            client.clean()
            server2.exit()

    def test_BugFix_TransferBetweenObjectStores_ALabelAlreadyExistsWithMoreCharacters(self):
        self.client.writeFile("firstFile", "123456")
        self.client.checkin("yuvu")
        self.client.checkin("yuv")

        server = osmosiswrapper.Server()
        try:
            self.client.transfer("yuvu", server)
            self.client.transfer("yuv", server)
        except:
            logging.error("Destination server log:\n%(log)s", dict(log=server.readLog()))
            raise
        finally:
            server.exit()

    def test_WillNotCheckIn_IfDraftsDirectoryExists(self):
        self.client.writeFile("firstFile", "123456")
        self.client.checkin("yuvu")
        self.client.writeFile("osmosisDrafts", "123456")
        message = self.client.failedCheckin("yuvu2")
        self.assertIn("drafts", message.lower())

    def test_Bugfix_CheckoutUsingDelayedLabel_DraftsDirLeftBehind(self):
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")
        os.unlink(self.client.abspath("aFile"))
        self.client.writeFile("osmosisDrafts", "123456")
        self.client.checkoutUsingDelayedLabel("yuvu")
        self.assertEquals(self.client.fileCount(), 1)
        self.assertEquals(self.client.readFile("aFile"), "123456")

    def test_Bugfix_TrailingSlash(self):
        self.client.appendToPath("/")
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")
        self.client.writeFile("aFile", "1234567")
        self.client.checkout("yuvu")
        self.assertEquals(self.client.fileCount(), 1)
        self.assertEquals(self.client.readFile("aFile"), "123456")

    def test_Policy_CleanupRemoveLabelsUntilDiskUsage(self):
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu1")
        self.client.checkin("yuvu2")
        self.client.checkin("yuvu3")
        self.client.checkin("yuvu4")
        self.client.checkin("yuvu5")
        fakeshell.makeDfSensitiveToLabels(
            100 / 5, self.server.path, os.path.join(self.server.path, 'labels'))
        objectStore = objectstore.ObjectStore(self.server.path)
        self.assertEquals(len(objectStore.labels()), 5)
        tested = cleanupremovelabelsuntildiskusage.CleanupRemoveLabelsUntilDiskUsage(objectStore, 50)
        tested.go()
        self.assertEquals(len(objectStore.labels()), 2)

    def test_ProgressReport_FinalReportIsThatEverythingCompleted(self):
        self.client.writeFile("aFile", "123456")
        reportFile = tempfile.NamedTemporaryFile()
        self.client.checkin("yuvu", reportFile=reportFile.name)
        report = json.loads(reportFile.read())
        self.assertEquals(report[u'state'], u'checkin')
        self.assertEquals(report[u'percent'], 100)
        self.assertEquals(report[u'put'][u'done'], 1)
        os.unlink(self.client.abspath("aFile"))
        self.client.checkout("yuvu", reportFile=reportFile.name)
        self.assertEquals(self.client.fileCount(), 1)
        self.assertEquals(self.client.readFile("aFile"), "123456")
        reportFile.seek(0)
        report = json.loads(reportFile.read())
        self.assertEquals(report[u'state'], u'fetching')
        self.assertEquals(report[u'percent'], 100)
        self.assertEquals(report[u'fetchesRequested'], 1)
        self.assertEquals(report[u'fetchesCompleted'], 1)

    def test_CheckoutOverHttp(self):
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")
        server = httpserver.HttpServer(self.server.path)
        try:
            client = osmosiswrapper.Client(server)
            client.objectStores = [server.url()]
            client.checkout("yuvu")
            self.assertEquals(client.fileCount(), 1)
            self.assertEquals(client.readFile("aFile"), "123456")
        finally:
            server.stop()

    def test_DirlistContainsSameObjectSeveralTimes(self):
        for i in xrange(100):
            self.client.writeFile("aFile%d" % i, "123456")
        self.client.checkin("yuvu")
        self.client.checkin("yuvu2")
        server = osmosiswrapper.Server()
        try:
            self.client.transfer("yuvu", server)
            self.client.transfer("yuvu2", server)
        except:
            logging.error("Destination server log:\n%(log)s", dict(log=server.readLog()))
            raise
        finally:
            server.exit()

    def test_ChainTouchVSNoChainTouch(self):
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")
        eventsBefore = self.server.labelLog()

        server2 = osmosiswrapper.Server()
        client = osmosiswrapper.Client(server2, self.server)
        try:
            client.checkout("yuvu", noChainTouch=True, putIfMissing=True)
            eventsAfter = self.server.labelLog()
            self.assertLess(len(eventsBefore), len(eventsAfter))

            client.checkout("yuvu", noChainTouch=True)
            eventsAfter2 = self.server.labelLog()
            self.assertEquals(eventsAfter2, eventsAfter)

            client.checkout("yuvu")
            eventsAfter3 = self.server.labelLog()
            self.assertLess(len(eventsAfter2), len(eventsAfter3))
        finally:
            client.clean()
            server2.exit()

    def test_LeastRecentlyUsed(self):
        self.client.writeFile("megabyte", "X" * (1024*1024))
        self.client.checkin("keepForever")
        time.sleep(2)
        self.client.writeFile("megabyte", "1" * (1024*1024))
        self.client.checkin("yuvu1")
        time.sleep(2)
        self.client.writeFile("megabyte", "2" * (1024*1024))
        self.client.checkin("yuvu2")
        time.sleep(2)
        self.client.writeFile("megabyte", "3" * (1024*1024))
        self.client.checkin("yuvu3")
        time.sleep(2)
        self.client.writeFile("megabyte", "4" * (1024*1024))
        self.client.checkin("yuvu4")

        self.assertEquals(
            set(self.client.listLabels()), set(["yuvu1", "yuvu2", "yuvu3", "yuvu4", "keepForever"]))
        self.server.leastRecentlyUsed(keep=".*keep.*", maximumDiskUsage="%dK" % int(3.5 * 1024))
        self.assertEquals(
            set(self.client.listLabels()), set(["yuvu3", "yuvu4", "keepForever"]))

    def test_LabelLogFlushesItself(self):
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")
        before = self.server.labelLog(flush=False)
        for i in xrange(100):
            self.client.checkout("yuvu")
        after = self.server.labelLog(flush=False)
        self.assertLess(len(before), len(after))

    def test_Bugfix_purgeCrashesIfLabelFileWasTruncated_HappensOnLocalSSDCaches(self):
        self.assertEquals(self.client.listLabels(), [])
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")
        open(os.path.join(self.server.path, "labels", "yuvu"), "w").close()
        self.server.purge()

    def test_ReadOnlyFileSystem_LabelLogDoesNotExist(self):
        objectStoreDir = tempfile.mkdtemp()
        try:
            self.client.objectStores = [objectStoreDir]
            self.client.writeFile("aFile", "123456")
            self.client.checkin("yuvu")
            self.assertEquals(self.client.listLabels(), ["yuvu"])

            os.unlink(self.client.abspath("aFile"))
            self.client.checkout("yuvu")
            self.assertEquals(self.client.fileCount(), 1)
            self.assertEquals(self.client.readFile("aFile"), "123456")

            os.unlink(self.client.abspath("aFile"))
            shutil.rmtree(os.path.join(objectStoreDir, "labelLog"))
            os.chmod(objectStoreDir, 0555)
            self.client.checkout("yuvu")
            self.assertEquals(self.client.fileCount(), 1)
            self.assertEquals(self.client.readFile("aFile"), "123456")

            os.unlink(self.client.abspath("aFile"))
            os.chmod(objectStoreDir, 0777)
            shutil.rmtree(os.path.join(objectStoreDir, "osmosisDrafts"))
            os.chmod(objectStoreDir, 0555)
            self.client.checkout("yuvu")
            self.assertEquals(self.client.fileCount(), 1)
            self.assertEquals(self.client.readFile("aFile"), "123456")
        finally:
            shutil.rmtree(objectStoreDir, ignore_errors=True)

    def test_Bugfix_PutFileInLocalObjectStoreCrashesIfParentDirIsCorruptedAndTurnedIntoNonDirFile(self):
        self.client.useLocalObjectStoreOnly()
        self.client.writeFile("aFile", "123456")
        fileHash = self.client.testHash("aFile")
        designatedParentDirInCache = os.path.join(fileHash[:2], fileHash[2:4])
        corruptedPath = os.path.join(self.client.localObjectStorePath, designatedParentDirInCache)
        self.client.insertMalformedFileInsteadOfDirInLocalObjectStore(corruptedPath, "fifo")
        self.assertFalse(stat.S_ISDIR(os.stat(corruptedPath).st_mode))
        self.client.checkin("yuvu")
        self.assertTrue(stat.S_ISDIR(os.stat(corruptedPath).st_mode))
        self.client.eraseLabel("yuvu")
        self.client.insertMalformedFileInsteadOfDirInLocalObjectStore(corruptedPath, "socket")
        self.assertFalse(stat.S_ISDIR(os.stat(corruptedPath).st_mode))
        self.client.checkin("yuvu")
        self.assertTrue(stat.S_ISDIR(os.stat(corruptedPath).st_mode))
        self.client.writeFile("aFile", "not 123456")
        self.client.checkout("yuvu")
        self.assertEquals(self.client.fileCount(), 1)
        self.assertEquals(self.client.readFile("aFile"), "123456")

    def test_Bugfix_DontRemoveAnUnknownFileWhichOneOfItsAncestorsIsASymlinkSinceItMightBelongToLabel(self):
        self.client.writeFile("a/b", "123456")
        self.client.createSymlink(relTargetPath="a", relLinkPath="c")
        self.client.checkin("yuvu")
        self.client.clean()
        self.client.writeFile("a/b", "123456")
        self.client.writeFile("c/b", "123456")
        self.assertEquals(self.client.readFile("a/b"), "123456")
        self.client.checkout("yuvu", removeUnknownFiles=True)
        self.assertEquals(self.client.readFile("a/b"), "123456")

    def test_CheckoutRecoversLocalObjectStoreIfLabelFileWasTruncatedByErasingIt_LocalStoreOnly(self):
        self.client.useLocalObjectStoreOnly()
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")
        labelFilePath = os.path.join(self.client.localObjectStorePath, "labels", "yuvu")
        self.assertTrue(os.path.exists(labelFilePath))
        open(labelFilePath, "w").close()
        self.assertRaises(subprocess.CalledProcessError, self.client.checkout, "yuvu")
        self.assertFalse(os.path.exists(labelFilePath))

    def test_CheckoutRecoversLocalObjectStoreIfLabelFileWasTruncatedByErasingIt_LocalAndRemoteStores(self):
        self.client.writeFile("aFile", "123456")
        self.client.useLocalObjectStoreOnly()
        self.client.checkin("yuvu")
        self.client.useRemoteObjectStoreOnly()
        self.client.checkin("yuvu")
        labelFilePath = os.path.join(self.client.localObjectStorePath, "labels", "yuvu")
        self.assertTrue(os.path.exists(labelFilePath))
        open(labelFilePath, "w").close()
        self.client.useLocalAndRemoteObjectStores()
        self.client.checkout("yuvu")
        self.assertFalse(os.path.exists(labelFilePath))
        self.assertEquals(self.client.readFile("aFile"), "123456")

    def test_IgnorePathsLongerThanPATH_MAXWhenRemovingUnknownFiles(self):
        PATH_MAX = int(subprocess.check_output(["getconf", "PATH_MAX", "/"]).strip())
        maxAllowedPathSize = PATH_MAX + 200
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")
        largePath = self.client.createAPathLargerThanPATH_MAX(maxAllowedPathSize)
        self.client.checkout("yuvu", removeUnknownFiles=True)
        self.assertEquals(self.client.readFile("aFile"), "123456")
        self.client.assertLargePathExists(largePath)

    def test_CheckoutRecoversWhenDeleteUnknownFilesInBadFileState(self):
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")
        # Link that points to itself causes file to become in bad state
        # trying to this link info will result in errno ELOOP
        self.client.createSymlink(relTargetPath="a", relLinkPath="a")
        self.client.checkout("yuvu", removeUnknownFiles=True)
        self.assertEquals(self.client.readFile("aFile"), "123456")
        self.assertFalse(os.path.exists('a'))

    def test_WhoHasLabel(self):
        self.broadcastServer.start()
        self.assertEquals(self.client.whoHasLabel("yuvu"), [])
        self.assertEquals(self.client.whoHasLabel("yu"), [])
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")
        objectStore = "127.0.0.1:%(port)s" % dict(port=self.broadcastServer.port())
        self.assertEquals(self.client.whoHasLabel("yuvu"), [objectStore])
        self.assertEquals(self.client.whoHasLabel("yu"), [])

    def test_CheckoutContinuesWhenOneOfTheObjectStoresFailsDuringListLabelsOp(self):
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")
        badServer = fakeservers.FakeServerCloseAfterListLabelsOp(self.client)
        client = osmosiswrapper.Client(badServer, self.server)
        try:
            client.checkout("yuvu")
            self.assertEquals(client.readFile("aFile"), "123456")
        finally:
            client.clean()

    def test_CheckoutContinuesWhenOneOfTheObjectStoresFailsDuringExistsOp(self):
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")
        badServer = fakeservers.FakeServerCloseAfterExistsOp(self.client)
        client = osmosiswrapper.Client(badServer, self.server)
        try:
            client.checkout("yuvu")
            self.assertEquals(client.readFile("aFile"), "123456")
        finally:
            client.clean()

    def test_CheckoutContinuesWhenOneOfTheObjectStoresFailsDuringGetOp(self):
        self.client.writeFile("aFile", "123456")
        self.client.checkin("yuvu")
        badServer = fakeservers.FakeServerCloseAfterGetOp(self.client)
        client = osmosiswrapper.Client(badServer, self.server)
        client.setTCPTimeout(1000)
        try:
            client.checkout("yuvu")
            self.assertEquals(client.readFile("aFile"), "123456")
        finally:
            client.clean()

    def test_ConnectTimeout(self):
        TIMEOUT_SEC = 0.1
        badServer = fakeservers.FakeServerConnectTimeout()
        client = osmosiswrapper.Client(badServer)
        timeoutInMilliseconds = TIMEOUT_SEC * 1000
        client.setTCPTimeout(timeoutInMilliseconds)
        before = time.time()
        try:
            client.listLabels("yuvu")
        except subprocess.CalledProcessError as ex:
            self.assertIn(ex.message, "Could not connect")
            after = time.time()
        else:
            self.assertFalse(True, "Did not timeout when connecting to non-existing objectstore")
        durationInMilliseconds = (after - before) * 1000
        self.assertLess(durationInMilliseconds, timeoutInMilliseconds + 30)

    def test_ReceiveTimeout(self):
        TIMEOUT_SEC = 0.01
        badServer = fakeservers.FakeServerNotSending()
        client = osmosiswrapper.Client(badServer)
        timeoutInMilliseconds = TIMEOUT_SEC * 1000
        client.setTCPTimeout(timeoutInMilliseconds)
        before = time.time()
        try:
            client.listLabels("yuvu")
        except subprocess.CalledProcessError as ex:
            self.assertIn("Timeout while reading", ex.output)
            after = time.time()
        else:
            self.assertFalse(True, "Did not timeout when connecting to non-existing objectstore")
        durationInMilliseconds = (after - before) * 1000
        self.assertLess(durationInMilliseconds, timeoutInMilliseconds + 30)

if __name__ == '__main__':
    unittest.main()
