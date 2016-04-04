#!/bin/bash

gnuplot animate.plt
convert $(for a in frame*; do printf -- "-delay 10 %s " $a; done; ) animate.gif
