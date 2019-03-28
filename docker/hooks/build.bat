@echo off

docker build ^
  --build-arg VERSION=%VERSION% ^
  --build-arg ELA_OC_AGENT_DEB=%ELA_OC_AGENT_DEB% ^
  --build-arg VCS_REF=1.0 ^
  -t %IMAGE_NAME% .