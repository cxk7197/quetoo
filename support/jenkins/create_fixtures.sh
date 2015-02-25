#!/bin/bash -e
#
# Create unit test fixtures for continuous integration builds
#

mkdir -p ~/.quetoo/default/maps
touch ~/.quetoo/default/maps/torn.bsp

echo -e '// generated by Quetoo, do not modify\n' > ~/.quetoo/default/quetoo.cfg
