![Logo](https://github.com/lxsang/ant-http/raw/master/ant-logo.png)
# ant-http
[![Build Status](https://travis-ci.org/lxsang/ant-http.svg?branch=master)](https://travis-ci.org/lxsang/ant-http)

A lightweight and portable HTTP/HTTPs web server written in standard C: 
- It can be configurable to work well on embedded Linux for server application. 
- Support SSL via open SSL, database via Sqlite 3, web socket integrated
- It is also extensible via its extensions mechanism that allows to extends the server capability. For example: [this extension](https://github.com/lxsang/ant-plugins) allows the server use embedded Lua as server side script for dynamic web.

# Plugins: 
* Lua extension [https://github.com/lxsang/antd-lua-plugin](https://github.com/lxsang/antd-lua-plugin): using Lua as serverside script
* PHP extension [https://github.com/lxsang/antd-ph7-plugin](https://github.com/lxsang/antd-ph7-plugin): using PHP as serverside script
* Web terminal [https://github.com/lxsang/antd-wterm-plugin](https://github.com/lxsang/antd-wterm-plugin): plugin for using Unix termninal from the web via websocket
* Web VNC [https://github.com/lxsang/antd-wvnc-plugin](https://github.com/lxsang/antd-wvnc-plugin): Remote computer accessing using VNC protocol on the web (via websocket)
