#~/bin/bash

cd $WORKSPACE_TOP/osmosis

modified_hash=`git diff | md5sum`
gitrev=`git rev-parse HEAD`
gitrev_hash=`echo $gitrev | md5sum`

hash=`echo $modified_hash $gitrev_hash | md5sum | awk {'print $1'} | cut -c 1-12`
echo -n $hash
