#!/bin/bash

astyle --options=dot-astylerc $( ack --type=cc -f )

# vim:ts=2:sw=2:sts=2:et:ft=sh

