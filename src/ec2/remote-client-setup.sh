#!/bin/bash

scp -o StrictHostKeyChecking=no -r client-setup.sh pmc $1:
ssh -o StrictHostKeyChecking=no -n $1 ./client-setup.sh