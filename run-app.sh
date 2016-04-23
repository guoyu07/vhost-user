#!/bin/bash

UDS="/tmp/user.uds"

[ -e $UDS ] && rm -f $UDS

./app $UDS tap0
