#!/usr/bin/env bash

apt update
apt install -y gcc g++ make git
apt install -y libphp-embed php-dev php-phpdbg procps
apt install -y python3 python3-dev python3-pip

echo "done ..."