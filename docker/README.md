# DittoBox Server Docker image

This is the official DittoBox Server image from the ownCloud community edition, it is built from ownCloud [server container](https://registry.hub.docker.com/u/owncloud/server/). This image is designed to work with a data volume in the host filesystem.


## Volumes

* /mnt/data


## Ports

* 80
* 443

## Available environment variables

```
ELA_OC_AGENT_SECRET
```

## Inherited environment variables

* [owncloud/server](https://github.com/owncloud-docker/server#available-environment-variables)
* [owncloud/base](https://github.com/owncloud-docker/base#available-environment-variables)
* [owncloud/ubuntu](https://github.com/owncloud-docker/ubuntu#available-environment-variables)


## Build locally

The available versions should be already pushed to the Docker Hub, but in case you want to try a change locally you can always execute the following command (run from a cloned GitHub repository) to get an image built locally on **Unix**-like operating system:

```
./build.sh
```

On Windows, choose to use the following command:

```
build.bat
```

### Launch with plain `docker`

The installation of `docker` is not covered by this instructions, please follow the [official installation instructions](https://docs.docker.com/engine/installation/). After the installation of docker then you can start the Elastos Personal Cloud Driver web server, you can customise the used environment variables as needed on **Unix**-like operating system:

```bash
docker run -d \
  -p 8080:80 \
  -p 8443:443 \
  --volume path_to_owncloud_files:/mnt/data \
  elastos/dittobox:10.0.7
```

On windows, use the following command as below:

```bash
docker run -d ^
  -p 8080:80 ^
  -p 8443:443 ^
  --volume path_to_owncloud_files:/mnt/data ^
  elastos/dittobox:10.0.7
```

### Accessing the ownCloud

By default you can access the ownCloud instance at [https://localhost/](https://localhost/) as long as you have not changed the port binding. The initial user gets set by the environment variables `ADMIN_USERNAME` and `ADMIN_PASSWORD`, per default it's set to `admin` for username and password.


## Contributing

Fork -> Patch -> Push -> Pull Request

## License

MIT


## Copyright

```
Copyright (c) 2018 elastos.org
```
