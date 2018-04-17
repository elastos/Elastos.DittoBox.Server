#!/usr/bin/env bash

if [[ ! -d ${ELA_OC_AGENT_DIR} ]]
then
  echo "Make Elastos oc-agent directory..."
  mkdir -p ${ELA_OC_AGENT_DIR}
fi

if [[ ! -f ${ELA_OC_AGENT_CONFIG_FILE} ]]
then
  echo "Copying Elastos oc-agent config file..."
  cp /etc/elaoc/elaoc-agent.conf ${ELA_OC_AGENT_CONFIG_FILE}
  DIR=${ELA_OC_AGENT_DIR//\//\\/}
  sed -i "s/datadir = data/datadir=${DIR}/g" ${ELA_OC_AGENT_CONFIG_FILE}

  SHASUM=`echo -n ${ELA_OC_AGENT_SECRET} | shasum -a 256`
  SECRET=${SHASUM::-3}
  sed -i "s/secret_hello=5efa94085cb2977f90ddd1a671c70aa651cd8a2b62183c9cc647628795b12092/secret_hello=${SECRET}/g" ${ELA_OC_AGENT_CONFIG_FILE}

  mv /etc/elaoc/elaoc-agent.conf /etc/elaoc/elaoc-agent.conf.org
  ln -s ${ELA_OC_AGENT_CONFIG_FILE} /etc/elaoc/elaoc-agent.conf
fi

true
