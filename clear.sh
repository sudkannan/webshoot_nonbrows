#!/bin/sh

rm -f tmp*
rm -f one*
rm -f two*
rm -f three
rm -f ./-o
rm -f /tmp/*
rm -f dbacl
rm -f *.tmp.*
rm -f *_online
rm -f *_out
#rm -f *.txt

 g++ -DHAVE_CONFIG_H -g -O2 -MT dbacl.o -MD -MP -c -o dbacl.o dbacl.cc && g++ -DHAVE_CONFIG_H  -g -O2 -MT fram.o -MD -MP  -c -o fram.o fram.cc && g++ -DHAVE_CONFIG_H      -g -O2 -MT catfun.o -MD -MP   -c -o catfun.o catfun.cc && g++ -DHAVE_CONFIG_H      -g -O2 -MT fh.o -MD -MP  -c -o fh.o fh.cc && g++ -DHAVE_CONFIG_H      -g -O2 -MT util.o -MD -MP -c -o util.o util.cc && g++ -DHAVE_CONFIG_H      -g -O2 -MT probs.o -MD -MP  -c -o probs.o probs.cc && g++ -DHAVE_CONFIG_H      -g -O2 -MT jenkins.o -MD -MP  -c -o jenkins.o jenkins.cc && g++ -DHAVE_CONFIG_H      -g -O2 -MT jenkins2.o -MD -MP  -c -o jenkins2.o jenkins2.cc && g++ -DHAVE_CONFIG_H      -g -O2 -MT mtherr.o -MD -MP  -c -o mtherr.o mtherr.cc && g++ -DHAVE_CONFIG_H      -g -O2 -MT igam.o -MD -MP  -c -o igam.o igam.cc && g++ -DHAVE_CONFIG_H      -g -O2 -MT gamma.o -MD -MP  -c -o gamma.o gamma.cc && g++ -DHAVE_CONFIG_H  -g -O2 -MT const.o -MD -MP  -c -o const.o const.cc && g++ -DHAVE_CONFIG_H      -g -O2 -MT polevl.o -MD -MP -c -o polevl.o polevl.cc && g++ -DHAVE_CONFIG_H      -g -O2 -MT isnan.o -MD -MP  -c -o isnan.o isnan.cc && g++ -DHAVE_CONFIG_H      -g -O2 -MT ndtr.o -MD -MP -c -o ndtr.o ndtr.cc && g++ -DHAVE_CONFIG_H      -g -O2 -DMBW_MB -c ./mbw.cc -o mb.o && g++ -DHAVE_CONFIG_H      -g -O2 -DMBW_WIDE -c ./mbw.cc -o wc.o && g++ -DHAVE_CONFIG_H -g -O2 -DMBW_WIDE -c ./hello_world.cc -o hello_world.o && g++  -g -O2 -o dbacl dbacl.o fram.o catfun.o fh.o util.o probs.o jenkins.o jenkins2.o mtherr.o igam.o gamma.o const.o polevl.o isnan.o ndtr.o mb.o wc.o hello_world.o -lm
