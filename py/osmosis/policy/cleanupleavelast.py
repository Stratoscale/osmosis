import logging


class ObjectStoreEmptyException(Exception):
    pass


class CleanupLeaveLast:
    def __init__(self, objectStore, last=5):
        self._objectStore = objectStore
        self._last = last

    def go(self):
        labels = self._objectStore.creationAgeByLabel()
        withoutHash = {}
        for label, age in labels.iteritems():
            if not label.startswith("solvent__"):
                continue
            without = label.split("__")
            without[3:4] = []
            without = "__".join(without)
            withoutHash.setdefault(without, []).append((age, label))
            withoutHash[without].sort()
        removed = False
        for without, labels in withoutHash.iteritems():
            if len(labels) <= self._last:
                continue
            logging.info("'%(without)s' has %(labelsCount)d entries, removing %(removeCount)d", dict(
                without=without, labelsCount=len(labels), removeCount=len(labels) - self._last))
            for age, label in labels[self._last:]:
                objectStore.eraseLabel(label)
                removed = True
        if removed:
            objectStore.purge()

if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG)
    from osmosis import objectstore
    objectStore = objectstore.ObjectStore("/var/lib/osmosis/objectstore")
    CleanupLeaveLast(objectStore).go()
