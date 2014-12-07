import re
import logging
from osmosis import sh


def dfPercent(location):
    output = sh.run(["df", location])
    try:
        line = " ".join(output.split("\n")[1:])
        return int(re.split(r"\s+", line)[4].strip("%"))
    except:
        logging.exception("Unable to parse DF output:\n%(output)s", dict(output=output))
        raise
