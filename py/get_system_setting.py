import sys
import errno
import os.path


OS_RELEASE_FILEPATH = os.path.join("/etc", "os-release")


def getOsRelease():
    osInfo = dict()
    try:
        with open(OS_RELEASE_FILEPATH, "r") as f:
            for line in f:
                line = line.strip()
                if "=" not in line:
                    continue
                key, value = line.split("=", 1)
                key = key.strip()
                value = value.strip("\" ")
                osInfo[key] = value
    except IOError as ex:
        if ex.errno == errno.ENOENT:
            print "Cannot determine OS type (the file '%(osReleasePath)s' was not found) in order to " \
                "determine the system manager type (e.g. upstart, systemd)." % \
                dict(osReleasePath=OS_RELEASE_FILEPATH)
            sys.exit(1)
        else:
            raise
    return osInfo


def getUbuntuAttributes(osRelease):
    version = osRelease["VERSION_ID"]
    components = version.split(".")
    major = int(components[0])
    minor = int(components[0])
    if major >= 15 and minor >= 4:
        systemManager = "systemd"
        serviceFilesDirPath = "/lib/systemd/system"
    else:
        systemManager = "upstart"
        serviceFilesDirPath = "/etc/init"
    return dict(systemManager=systemManager, serviceFilesDirPath=serviceFilesDirPath)


def ubuntu(osRelease, settingType):
    settings = getUbuntuAttributes(osRelease)
    return settings[settingType]


def fedora(osRelease, settingType):
    settings = dict(systemManager="systemd", serviceFilesDirPath="/usr/lib/systemd/system")
    return settings[settingType]


def centos(osRelease, settingType):
    settings = dict(systemManager="systemd", serviceFilesDirPath="/usr/lib/systemd/system")
    return settings[settingType]


def getSystemSetting(osRelease, settingType):
    osID = osRelease["ID"]
    osHandlers = dict(ubuntu=ubuntu, fedora=fedora, centos=centos)
    if osID not in osHandlers:
        print "Unsupported operating system type: %(osID)s. Cannot perform installation since the system " \
            "manager (e.g. upstart, systemd) cannot be determined by the OS type. You can tell the " \
            "installation what type of system manager you use by expanding this script." % dict(osID=osID)
        sys.exit(1)
    handler = osHandlers[osID]
    return handler(osRelease, settingType)


def main():
    if len(sys.argv) != 2:
        print "Usage: get_system_setting.py settingType."
        print "Possible setting types: systemManager (e.g. systemd, upstart), serviceFilesDirPath"
        sys.exit(1)
    settingType = sys.argv[1]
    osRelease = getOsRelease()
    print getSystemSetting(osRelease, settingType)


if __name__ == "__main__":
    main()
