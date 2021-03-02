#!/bin/bash

seq -f "1 n%02g" 1 10 | xargs -n 1 -P 2 -I {} bash -c './HELLO $@' _ {}

echo "Joined"