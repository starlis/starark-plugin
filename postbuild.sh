#!/bin/bash
echo "Uploading to ark server"
scp cmake-build-release/Release/SAOmega.{dll,pdb} ark1:plugin/
