#!/usr/bin/env bash

if [ -z ${OA_LINK_DIR+x} ]
then
    echo "OA_LINK_DIR is unset"
    exit 1
fi

if [ -z ${BAG_PYTHON+x} ]
then
    echo "BAG_PYTHON is unset"
    exit 1
fi

cd python
if [ -d "build" ]
then
  echo "removing build ..."
  rm -rf "build"
fi

${BAG_PYTHON} setup.py build
