![Logo](https://github.com/lxsang/ant-http/raw/master/ant-logo.png)
# ant-http
[![Build Status](https://ci.iohub.dev/api/badges/lxsang/ant-http/status.svg)](https://ci.iohub.dev/lxsang/ant-http)
[![Build Status](https://travis-ci.org/lxsang/ant-http.svg?branch=master)](https://travis-ci.org/lxsang/ant-http)

A lightweight HTTP/HTTPs (1.1) web server written in  C:
- Nonblocking event driven base server with configurable number of thread pool workers.
- Initial goal is for embedded Linux, but can be used as general purpose web server. 
- Support:
  - SSL via open SSL,
  - basic API database via Sqlite 3,
  - web socket,
  - reverse proxy.
- It is also extensible via its extension mechanism that allows to extends the server capability.
- Page compression with gzip, deflate, cache control

## Plugins:
* CGI interface for external scripting language (e.g. PHP): [https://github.com/lxsang/antd-cgi-plugin](https://github.com/lxsang/antd-cgi-plugin)
* Lua extension [https://github.com/lxsang/antd-lua-plugin](https://github.com/lxsang/antd-lua-plugin): using Lua as serverside script
* Web terminal [https://github.com/lxsang/antd-wterm-plugin](https://github.com/lxsang/antd-wterm-plugin): plugin for using Unix terminal from the web via websocket
* Web VNC [https://github.com/lxsang/antd-wvnc-plugin](https://github.com/lxsang/antd-wvnc-plugin): Remote computer access using VNC protocol on the web (via websocket)

## Build from source
### build dep
* git
* make
* build-essential

### server dependencies
* libssl-dev (expecting openssl v1.1.1d, only support TLSv1.2 and TLSv1.3)
* libsqlite3-dev
* zlib-dev

### build
When all dependencies are installed, the build can be done with a few single command lines:

```bash
mkdir antd
cd antd
# Replace x.x.x by the version number
# build without plugin
wget -O- https://get.bitdojo.dev/antd | bash -s "x.x.x"

# or from the distribution tarball
tar xvzf antd-x.x.x.tar.gz
cd antd-x.x.x
./configure --prefix=/usr --enable-debug=yes
make
sudo make install
```
### Generate distribution
```sh
libtoolize
aclocal
autoconf
automake --add-missing
make distcheck
``` 
