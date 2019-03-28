#/bin/bash

VERSION=10.0.7
ELA_OC_AGENT_DEB=https://github.com/elastos/Elastos.DittoBox.Server/releases/download/v1.2.1/elaoc-agentd.deb
IMAGE_NAME=elastos/dittobox:${VERSION}

docker build \
  --build-arg VERSION=${VERSION} \
  --build-arg ELA_OC_AGENT_DEB=${ELA_OC_AGENT_DEB} \
  --build-arg BUILD_DATE=$(date -u +"%Y-%m-%dT%H:%M:%SZ") \
  --build-arg VCS_REF=$(git rev-parse --short HEAD) \
  -t ${IMAGE_NAME} .