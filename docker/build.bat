@echo off

set VERSION=10.0.7
set ELA_OC_AGENT_DEB=https://github.com/elastos/Elastos.DittoBox.Server/releases/download/v1.2.1/elaoc-agentd.deb
set IMAGE_NAME=elastos/dittobox:%VERSION%

call ./hooks/build.bat

