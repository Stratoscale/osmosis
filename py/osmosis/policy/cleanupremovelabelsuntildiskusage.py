import logging
from osmosis.policy import disk


class ObjectStoreEmptyException(Exception):
    pass


class CleanupRemoveLabelsUntilDiskUsage:
    def __init__(self, objectStore, allowedDiskUsagePercent=50):
        self._objectStore = objectStore
        self._allowedDiskUsagePercent = allowedDiskUsagePercent

    def go(self):
        accessAgeByLabel = None
        while self._diskUsage() > self._allowedDiskUsagePercent:
            if len(self._objectStore.labels()) == 0:
                raise ObjectStoreEmptyException("Can not apply policy, object store already empty")
            if accessAgeByLabel is None:
                accessAgeByLabel = self._objectStore.accessAgeByLabel()
                # purge accesses all labels, don't read access times after calling it
            labels = [(accessAgeByLabel[label], label) for label in self._objectStore.labels()]
            labels.sort()
            eraseCount = max(len(labels) / 2, 1)
            logging.info(
                "Disk usage %(diskUsage)s%% > %(allowedDiskUsage)s%%, erasing %(count)s labels",
                dict(
                    diskUsage=self._diskUsage(), allowedDiskUsage=self._allowedDiskUsagePercent,
                    count=eraseCount))
            for age, label in labels[-eraseCount:]:
                self._objectStore.eraseLabel(label)
                logging.info("Erased: %(label)s", dict(label=label))
            logging.info("Purging object store")
            self._objectStore.purge()
            logging.info("Done Purging object store")
        logging.info("Disk usage %(diskUsage)s%%", dict(diskUsage=self._diskUsage()))

    def _diskUsage(self):
        return disk.dfPercent(self._objectStore.root())
