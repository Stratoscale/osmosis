import subprocess


class RunError(Exception):
    def __init__(self, calledProcessError):
        self._calledProcessError = calledProcessError

    def __str__(self):
        return str(self._calledProcessError) + ". Output was:\n%s" % (self._calledProcessError.output, )


def run(command):
    try:
        return subprocess.check_output(command, stderr=subprocess.STDOUT, close_fds=True)
    except subprocess.CalledProcessError as e:
        raise RunError(e)
