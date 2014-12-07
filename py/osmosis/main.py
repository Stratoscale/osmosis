import argparse
import logging
import re
from osmosis import objectstore

logging.basicConfig(level=logging.INFO)

parser = argparse.ArgumentParser()
subparsers = parser.add_subparsers(dest="cmd")
labelsOlder = subparsers.add_parser("labelsOlder")
labelsOlder.add_argument("--hours", type=int, required=True)
labelsOlder.add_argument("--regex", required=True)
eraseLabelsOlder = subparsers.add_parser("eraseLabelsOlder")
eraseLabelsOlder.add_argument("--hours", type=int, required=True)
eraseLabelsOlder.add_argument("--regex", required=True)
eraseLabelsOlder.add_argument("--iamsure", action="store_true")
parser.add_argument("--objectStore", default="/var/lib/osmosis/objectstore")
args = parser.parse_args()

if args.cmd == "labelsOlder":
    objectStore = objectstore.ObjectStore(args.objectStore)
    regex = re.compile(args.regex)
    for label, ageHours in objectStore.labelsOlder(args.hours).iteritems():
        if regex.search(label) is None:
            continue
        print "%s: %d hours old" % (label, ageHours)
elif args.cmd == "eraseLabelsOlder":
    objectStore = objectstore.ObjectStore(args.objectStore)
    regex = re.compile(args.regex)
    for label, ageHours in objectStore.labelsOlder(args.hours).iteritems():
        if regex.search(label) is None:
            continue
        if args.iamsure:
            objectStore.eraseLabel(label)
        else:
            print "would have erased %s, which is %d hours old, but you must specify --iamsure" % (
                label, ageHours)

else:
    assert False, "command mismatch"
