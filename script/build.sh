#!/usr/bin/zsh
cd getline && make -f ${ENVROOT}/lib/plumber/servlet.mk
cd ..
cd say && make -f ${ENVROOT}/lib/plumber/servlet.mk
cd ..
