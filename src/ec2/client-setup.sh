#!/bin/bash

apt-get update
apt-get -y install libopenmpi-dev openmpi-bin
echo export PATH="/root/pmc:$PATH" >> ~/.bashrc
