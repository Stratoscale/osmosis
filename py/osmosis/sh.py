import subprocess


def run(command):
    try:
        return subprocess.check_output(command, stderr=subprocess.STDOUT, close_fds=True)
    except subprocess.CalledProcessError as e:
        raise subprocess.CalledProcessError("Command '%s' failed: %d\n%s" % (
            command, e.returncode, e.output))
