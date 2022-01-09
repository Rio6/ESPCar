#!/bin/sh -e
ncat -lu 3232 | ffplay -f mjpeg -i -
