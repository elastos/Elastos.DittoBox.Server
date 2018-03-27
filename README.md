# Owncloud Service over Elastos Carrier

## Summary

Owncloud service can be accessed at anytime and from anywhere over Elastos Carrier.

## Build form source

### 1. Build Carrier NDK

You need to download Carrier NDK source from following github address:

```
https://github.com/elastos/Elastos.NET.Carrier.Native.SDK
```

Then, follow the steps addressed README.md of that repository to build carrier ndk distribution.

### 2. Import Carrier NDK

After buiding native ndk distribution, you should export path of distribution to environment variable "$CARRIER\_DIST\_PATH".

```shell
$ export CARRIER_DIST_PATH=YOUR-CARRIER-DIST-PATH
```

This environment value will be used for building owncloud agent.

### 3. Build Owncloud Agent

Run the following commands to build agent on MacOS:

```
$ cd build
$ ./darwin_build.sh 

```

or Run the commands on Linux to build debian package:

```
$ cd build
$ ./linux_build.sh dist
```

or commands on Raspberry Pi device:

```
$ cd build
$ ./raspbian_build.sh dist
```

You also can run the command with 'help' option to get more information:

```
$ ./darwin_build.sh help
```

The debian package of owncloud agent would be generated through build command with 'dist' option and only be allowed on Linux (Raspberry if runs Linux).

## Deploy & Run 

Recommended target platform: Ubuntu server 16.04 LTS / x86_64

Copy generated Debian package (.deb file) to target machine. Then run following command to install Owncloud service agent daemon:

```shell
$ sudo dpkg -i /path/to/elaoc-agentd.deb
```

After install complete, the agent deamon will start automatically. You can run:

```shell
$ sudo systemctl status elaoc-agentd
```

## Thanks

Sinserely thanks to all teams and projects that we relying on directly or indirectly.

## Contributing

We welcome contributions to the Owncloud agent Project in many forms.

## License

This source code files are made available under the MIT License, located in the LICENSE file.
