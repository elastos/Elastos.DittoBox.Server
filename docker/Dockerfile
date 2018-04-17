FROM owncloud/server:latest
MAINTAINER elastos.org <devops@elastos.org>

ARG VERSION
ARG BUILD_DATE
ARG VCS_REF

ARG ELA_OC_AGENT_DEB

RUN curl -sLo ela_oc_agent.deb ${ELA_OC_AGENT_DEB} && \
  dpkg -i ela_oc_agent.deb && \
  rm -f ela_oc_agent.deb

COPY rootfs /

LABEL \
  org.label-schema.version=$VERSION \
  org.label-schema.build-date=$BUILD_DATE \
  org.label-schema.vcs-ref=$VCS_REF \
  org.label-schema.vcs-url="https://github.com/elastos/Elastos.PersonalCloudDrive.Service.git" \
  org.label-schema.name="Elastos Cloud Server" \
  org.label-schema.vendor="elastos.org" \
  org.label-schema.schema-version="1.0"
