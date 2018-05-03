OwnCloud Service over Elastos Carrier
=====================================

## Summary

OwnCloud service can be accessed at anytime and from anywhere over Elastos Carrier network even the service is deployed behind router.

## Build form source

### 1. Build Carrier NDK

You need to download Carrier NDK source from following github address:

```
https://github.com/elastos/Elastos.NET.Carrier.Native.SDK
```

Then, follow the steps addressed **README.md** of that repository to build carrier ndk distribution.

### 2. Import Carrier NDK

After buiding native ndk distribution, you should export path of distribution to environment variable **"$CARRIER\_DIST\_PATH"**.

```shell
$ export CARRIER_DIST_PATH=YOUR-CARRIER-DIST-PATH
```

This environment value will be used for building ownCloud agent.

### 3. Build ownCloud Agent

Run the following commands to build agent on MacOS:

```shell
$ cd build
$ ./darwin_build.sh 

```

or Run the commands on Linux to build debian package:

```shell
$ cd build
$ ./linux_build.sh dist
```

or commands on Raspberry Pi device:

```shell
$ cd build
$ ./raspbian_build.sh dist
```

You also can run the command with 'help' option to get more information:

```shell
$ ./darwin_build.sh help
```

Beaware, debian package of ownCloud agent would be generated through build command with **"dist"** option and only be allowed on **Linux**-like (Raspberry if runs Linux) system.

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
