#!/usr/bin/env bash

if [[ ! -d ${ELA_OC_AGENT_DIR} ]]
then
  echo "Make Elastos oc-agent directory..."
  mkdir -p ${ELA_OC_AGENT_DIR}
fi

if [[ ! -f ${ELA_OC_AGENT_CONFIG_FILE} ]]
then
  echo "Copying Elastos oc-agent config file..."
  cp /etc/elaoc/elaoc-agent.conf.org ${ELA_OC_AGENT_CONFIG_FILE}
  DIR=${ELA_OC_AGENT_DIR//\//\\/}
  sed -i "s/datadir = data/datadir=${DIR}/g" ${ELA_OC_AGENT_CONFIG_FILE}

  SHASUM=`echo -n ${ELA_OC_AGENT_SECRET} | shasum -a 256`
  SECRET=${SHASUM::-3}
  sed -i "s/secret_hello=5efa94085cb2977f90ddd1a671c70aa651cd8a2b62183c9cc647628795b12092/secret_hello=${SECRET}/g" ${ELA_OC_AGENT_CONFIG_FILE}
fi

if [[ ! -f ${ELA_OC_AGENT_DIR}/address.png ]]
then
    cp /var/www/owncloud/core/img/address-inv.png ${ELA_OC_AGENT_DIR}/address.png
fi

echo "Prepare Elastos oc-agent config file..."
rm -f /etc/elaoc/elaoc-agent.conf
ln -s ${ELA_OC_AGENT_CONFIG_FILE} /etc/elaoc/elaoc-agent.conf
ln -s ${ELA_OC_AGENT_DIR}/address.png /var/www/owncloud/core/img/address.png

true
