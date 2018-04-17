#!/usr/bin/env bash

declare -x ELA_OC_AGENT_SECRET
[[ -z "${ELA_OC_AGENT_SECRET}" ]] && ELA_OC_AGENT_SECRET="secret"

declare -x ELA_OC_AGENT_DIR
[[ -z "${ELA_OC_AGENT_DIR}" ]] && ELA_OC_AGENT_DIR="${OWNCLOUD_VOLUME_ROOT}/agent"

declare -x ELA_OC_AGENT_CONFIG_FILE
[[ -z "${ELA_OC_AGENT_CONFIG_FILE}" ]] && ELA_OC_AGENT_CONFIG_FILE="${ELA_OC_AGENT_DIR}/elaoc-agent.conf"
