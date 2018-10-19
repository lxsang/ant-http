[SERVER]             
; server port
; use 443 if one want to use
; SSL
port=8080
; plugin directory
plugins=/opt/www/plugins/
; plugin extension
plugins_ext=.dylib
; SQLITE database path
database=/opt/www/database/
; Website store here
htdocs=/opt/www/htdocs
; tmp dir
tmpdir=/opt/www/tmp/
; server backlocg
backlog=5000
; eable or disalbe SSL
ssl.enable=0
; if SSL is enable, one should specify
; the SSL cert and key files
; ssl.cert=fullchain.pem
; ssl.key=privkey.pem

; This enable some plugins to be initialised at server startup
[AUTOSTART]
; to start a plugin at server statup use:
;plugin = plugin_name_1
;plugin = plugin_name_2, etc

; sever rules
[RULES]
; For example the following rule will
; convert a request of type:
;   name.example.com?rq=1  
;TO:
;    example.com/name/?rq=1
; this is helpful to redirect many sub domains
; to a sub folder of the same server
; ^([a-zA-Z][a-zA-Z0-9]*)\.[a-zA-Z0-9]+\..*$ = /<1><url>?<query>
; Sytax: [regular expression on the original request]=[new request rule]

[FILEHANDLER]
; specify a plugin for handling
; a file type
; lua page script
ls = lua-api
; pure lua script
lua = lua-api
; php and o ther scripting languages can be
; handled by the cgi plugin
; php = cgi