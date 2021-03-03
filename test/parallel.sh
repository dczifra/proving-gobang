#!/bin/bash

cat $1 | xargs -n 1 -P 2 -I {} bash -c './AMOBA --log --PNS2 --parallel <<<"$@"' _ {}
#cat $1 | xargs -n 1 -P 2 -I {} bash -c 'echo $@' _ {}
#seq -f "1 n%02g" 1 10 | xargs -n 1 -P 2 -I {} bash -c './HELLO $@' _ {}

wait
echo "Joined"