import os
import time
from osmosis import sh


class ObjectStore:
    def __init__(self, root):
        self._root = root

    def root(self):
        return self._root

    def accessAgeByLabel(self):
        return self._ageByLabel(statAttribute="st_atime")

    def creationAgeByLabel(self):
        return self._ageByLabel(statAttribute="st_ctime")

    def labelsOlder(self, ageHours):
        return {k: v for k, v in self.ageHoursByLabel().iteritems() if v >= ageHours}

    def labelPath(self, label):
        return os.path.join(self._root, "labels", label)

    def eraseLabel(self, label):
        os.unlink(self.labelPath(label))

    def purge(self):
        sh.run(["osmosis", "purge", "--objectStores", self._root])

    def labels(self):
        return os.listdir(os.path.join(self._root, "labels"))

    def _ageByLabel(self, statAttribute="st_atime"):
        result = dict()
        now = time.time()
        for label in self.labels():
            atime = getattr(os.stat(self.labelPath(label)), statAttribute)
            age = now - atime
            if age < 0:
                age = 0
            result[label] = int(age)
        return result
