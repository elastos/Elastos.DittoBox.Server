DittoBox Server
===============

## Summary

DittoBox server integrates ownCloud Server and Elastos Carrier. You can access your files at anytime and from anywhere over Elastos Carrier network even server is deployed behind router.

## Build form source

### 1. Build Carrier NDK

Download Carrier NDK source from following Github address:

```
https://github.com/elastos/Elastos.NET.Carrier.Native.SDK
```

Then, follow the steps addressed **README.md** of the repository to build carrier NDK distribution.

### 2. Build ownCloud Agent

With Carrier NDK built,  download **Dittobox** server from the Github address:

```
https://github.com/elastos/Elastos.DittoBox.Server
```

once you have source tree, to make compilation for the target to run on host Linux, need to execute following commands under directory of `${YOUR-SOURCE-ROOT}/build`:

```shell
$ cd YOUR-SOURCE-ROOT/build
$ mkdir linux
$ cd linux
$ cmake -DCARRIER_SDK_PATH=YOUR-PREBUILT-SDK-INSTALL-PATH ../..
$ make
```
Also to build distribution with specific build type **Debug/Release**, as well as with customised install location of distributions, run the following commands:

```shell
$ cmake -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=YOUR-INSTALL-PATH \
        -DCARRIER_SDK_PATH=YOUR-PREBUILT-SDK-INSTALL-PATH ../..
$ make
$ make install
```
Eventually,  run the following command to release distribution package for your pre-release tests or even to customers.

```bash
$ make dist
```

## Deploy & Run

Recommended target platform: Ubuntu server 16.04 LTS / x86_64

Copy generated debian package (.deb file) to target machine. Then run following command to install ownCloud service agent daemon:

```shell
$ sudo dpkg -i /path/to/elaoc-agentd.deb
```

After install complete, the agent deamon will start automatically. You can run:

```shell
$ sudo systemctl status elaoc-agentd
```

to check status of service **elaoc-agentd**.

## Thanks

Sincerely thanks to all teams and projects that we relies on directly or indirectly.

## Contributing

We welcome contributions to the ownCloud agent Project in many forms.

## License

MIT
