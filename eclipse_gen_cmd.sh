#!/usr/bin/env tcsh

setenv SRC_ROOT "/tools/projects/erichang/oa_dev"
soource ${SRC_ROOT}/.cshrc

cmake -G"Eclipse CDT4 - Unix Makefiles" ${SRC_ROOT}
