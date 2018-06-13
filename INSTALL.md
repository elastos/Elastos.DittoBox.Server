Installation Guide of DittoBox Server
=====================================

DittoBox is a demo application that intergate ownCloud over elastos carrier network, thereby through which people can access personal files to ownCloud drive even deployed behind router.

Currently, we only recommend to conduct installation of DittoBox server on Linux system. The next wizard provides a complete walk-through for installation on Ubuntu 16.04 LTS/x86_64.

## 1. Web Server 

As to web server, use following command to install Apache Http Server and its depedency packages:

```shell
$ sudo apt-get install -y apache2 libapache2-mod-php7.0 \
    openssl php-imagick php7.0-common php7.0-curl php7.0-gd \
    php7.0-imap php7.0-intl php7.0-json php7.0-ldap php7.0-mbstring \
    php7.0-mcrypt php7.0-mysql php7.0-pgsql php-smbclient php-ssh2 \
    php7.0-sqlite3 php7.0-xml php7.0-zip
```

You will need to make some configurations to Http Server after ownCloud installation.

## 2. Database

### 1). Installation

The following types of database products are supported for ownCloud server:

    - SQLite
    - MySql/MariaDB
    - PostgreSQL

According to ownCloud suggest, **MySql/MariaDB** is the recommended database to be used for ownCloud server.

Run the following command to install **MySql/MariaDB**:

```shell
$ sudo apt-get install -y mariadb-server
```

### 2). Configuration

After installation, run the following steps to create a database adminstrator account:

```shell
$ sudo mysql --user=root mysql
> CREATE USER 'dbadmin'@'localhost' IDENTIFIED BY 'Apassword';
> GRANT ALL PRIVILEGES ON *.* TO 'dbadmin'@'localhost' WITH GRANT OPTION;
> FLUSH PRIVILEGES;
> exit
```

## 3. Owncloud Server

### 1). Download

Use following command to download ownCloud community server archive with stable version 10.0.7 and it's sha256 checksum file:

```shell
$ curl -O https://download.owncloud.org/community/owncloud-10.0.7.tar.bz2
$ curl -O https://download.owncloud.org/community/owncloud-10.0.7.tar.bz2.sha256
```

After downloading, then verify integrity of archive.

```shell
$ sha256sum -c owncloud-10.0.7.tar.bz2.sha256 < owncloud-10.0.7.tar.bz2
```

### 2). Installation

Now you can extract the achive contents. Run this unpacking command for **bz2** archive:

```shell
$ tar -xjf owncloud-10.0.7.tar.bz2
```

After extraction, you will have **owncloud** single directory, then copy this directory to **/var/www**, where directory **/var/www** is the document root of Web Server.

```shell
$ sudo cp -r owncloud /var/www
```

Then, reconfigure the ownership of your ownCloud directory to Http user:

```shell
$ chown -R www-data:www-data /var/www/owncloud/
```

### 3). Configuration

As to Ubuntu (or Debian), Apache installs with a useful configuration, so for now all you have to do is create a config file **owncloud.conf** under directory **/etc/apache2/sites-available** with specific lines in it:

```
Alias /owncloud "/var/www/owncloud/"

<Directory /var/www/owncloud/>
  Options +FollowSymlinks
  AllowOverride All

 <IfModule mod_dav.c>
  Dav off
 </IfModule>

 SetEnv HOME /var/www/owncloud
 SetEnv HTTP_HOME /var/www/owncloud

</Directory>
```

Then create a symlink to **/etc/apache2/sites-enabled** with command:

```shell
$sudo ln -s /etc/apache2/sites-available/owncloud.conf /etc/apache2/sites-enabled/owncloud.conf
```

For ownCloud to work correctly, you need the module **mod_rewrite**. Enable it by running:

```shell
$ a2enmod rewrite
```

Additional recommended modules are listed below:

```shell
$ a2enmod headers
$ a2enmod env
$ a2enmod dir
$ a2enmod mime
```

you need to restart **apache2** to reload these configration with command:

```shell
$ sudo systemctl restart apache2
```

You also can use following command to check if **apache2** service is working correctly:

```shell
$ sudo systemctl status apache2
```

## 4. Agent Service

### 1). Build Carrier NDK

You need to download Carrier NDK source from following github address:

```
https://github.com/elastos/Elastos.NET.Carrier.Native.SDK
```
And follow steps of README.md of the repository to build carrier ndk distribution.

### 2). Import Carrier NDK 

After buiding native ndk distribution, you should export path of distribution to environment variable **"\$CARRIER_DIST_PATH"**:

```shell
$ export CARRIER_DIST_PATH=YOUR-CARRIER-DIST-PATH
```

This environment value will be used for building ownCloud agent.

### 3). Build ownCloud Agent

Run the following commands to build agent deb package on Ubuntu Linux:

```shell
$ cd build
$ ./linux_build.sh dist
```

### 3). Deployment

Copy generated debian package (.deb file) to target machine. Then run following command to install ownCloud service agent daemon:

```shell
$ sudo dpkg -i /path/to/elaoc-agentd.deb
```

After install complete, the agent deamon will start automatically. You can run commands to check it's status:

```
$ sudo systemctl status elaoc-agentd
```

### 4). Others

You can reconfigure the settings about ownCloud agent, and it's config file to ownCloud agent is located under directory **/etc/elaoc/elaoc-agent.conf **.

After reconfiguration, you need to restart **elaoc-agentd** service with command:

```shell
$ sudo systemctl restart elaoc-agentd
```

You also can use command to monitor the running log of ownCloud agent:

```
$ tail -f /var/lib/elaoc-agentd/elaoc-agentd.log
```

and through which you can acquire Carrier Node ID and address of ownCloud agent.

If you want reset ownCloud agent, you need to remove private data under directory **/var/lib/elaoc-agentd/data**, and restart **elaoc-agentd** service.


## 5. References

The more details about **How to install ownCloud Server**, please refer to [OwnCloud Server Installation Manual][1]

[1]: https://doc.owncloud.org/server/latest/admin_manual
