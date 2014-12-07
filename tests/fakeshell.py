import atexit
import tempfile
import shutil
import os


_dir = tempfile.mkdtemp()
atexit.register(lambda: shutil.rmtree(_dir))
os.environ['PATH'] = _dir + ":" + os.environ['PATH']


def makeDfSensitiveToLabels(diskUsagePerLabel, location, labelsLocation):
    TEMPLATE = (
        "import sys\n"
        "import os\n"
        "\n"
        "if sys.argv[1] != 'LOCATION':\n"
        " raise Exception('Expected different location')\n"
        "diskUsage = DISK_USAGE_PER_LABEL * len(os.listdir('LABELS_LOCATION'))\n"
        "print 'Filesystem     1K-blocks   Used Available Use% Mounted on'\n"
        "print 'unknown            XXX      XX    XXXX   %s%% LOCATION' % diskUsage\n")
    script = TEMPLATE.replace('DISK_USAGE_PER_LABEL', str(diskUsagePerLabel)).replace(
        'LABELS_LOCATION', labelsLocation).replace('LOCATION', location)
    _putScript("df", script)


def _putScript(name, content):
    with open(os.path.join(_dir, name), "w") as f:
        f.write("#!/usr/bin/python\n")
        f.write(content)
    os.chmod(os.path.join(_dir, name), 0755)
