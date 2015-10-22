var editor_html = 
		'<div id="editor" style="position: absolute;top:0;right: 0;bottom: 0;left: 0;"> \n'
    	+'</div> \n'
    	+'<script>\n'
    	+ 'ace.require("ace/ext/language_tools");'
    	+'var editor = ace.edit("editor");\n'
		+ 'editor.setOptions({'
    	+ 'enableBasicAutocompletion: true,'
    	//+ 'enableSnippets: true,\n'
    	+ 'enableLiveAutocompletion: true \n'
		+ '});'
    	+'editor.setTheme("ace/theme/monokai");\n'
   	 	+'editor.getSession().setMode("ace/mode/text");\n'
   	 	+'editor.completers.push(editor_syntax_ac);\n'
		+'</script>';
var top_html = 
		"<div id = 'custom_top_header'>"
		+	"<div id='ct_left'><span stype=''></span></div>"
		+	"<div id='ct_right'>"
		+	"	<ul>"
		+	"		<li id='bt_download_image'/>"
		+	"		<li id='bt_save_image' />"
		+	"		<li id='bt_workspace'/>"
		+	"	</ul>"
		+	"</div>"
		+"</div>";
var editor_syntax_ac = {
    getCompletions: function(editor, session, pos, prefix, callback) {
    	$.getJSON("ffvm/editor_ac",
    		function(list) {
      		callback(null,list)});
    }
};
// system config
var config = {
	layout:{
		name: 'layout',
        panels: [
            { 	
            	type: 'top', 
            	size: 45, 
            	style: 'padding: 0px;', 
            	content: top_html
            	//title: 'FireFly Smalltalk'
            },
            { 
            	type: 'left', 
            	size: 200, 
            	style: 'padding: 0px;', 
            	resizable: true,
            	content: 'left',
            	toolbar: {
                    items: [
                    	{ type: 'html',  id: 'classbr_header',
                			html: '<div style="padding: 3px 10px; font-weight:bold;">'+
                      		'Classes browser'+'</div>' 
            				},
                        	{ type: 'spacer' },
                        { type: 'menu',   id: 'cls_ed', caption: '', icon: ' fa-edit', items: [
                            { id:'new_cls', text: 'New class', icon: 'fa-plus' }, 
                            { text: 'Edit class', icon: 'fa-pencil' }, 
                            { text: 'Delete class', icon: 'fa-trash' }
                        ]},
                        { type: 'button',  id: 'clsbr_ref',  caption: '', icon: 'fa-refresh'}
                    ],
                    onClick: function (event) {
                       // this.owner.content('main', event);
                       switch(event.target)
                       {
                       		case 'cls_ed:new_cls':
                       		sclbrr_config.openClassEditor();
                       		break;
                       		case 'clsbr_ref':
                       		sclbrr_config.reload();
                       		break;
                       		default:
                       			console.log(event);
                       }
                       
                    }
                } 
            },
            { 
            	type: 'main', 
            	style: 'padding: 0px;', 
            	content: 'main'
            }
        ]
	},
	right_layout:{
		name: 'right_layout',
        panels: [
            { 
            	type: 'left', 
            	size: 200, 
            	//style: 'padding: 0px;', 
            	resizable: true,
            	content: 'left',
            	toolbar: {
                    items: [ 
                        	{ type: 'html',  id: 'methodsbr_header',
                			html: '<div style="padding: 3px 10px; font-weight:bold;">'+
                      		'Methods browser'+'</div>' 
            				},
                        	{ type: 'spacer' },
                        	{ type: 'button',  id: 'methbr_del',  caption: '', icon: 'fa-trash'},
                        	{ type: 'button',  id: 'methbr_ref',  caption: '', icon: 'fa-refresh'}
                        ],
                    onClick: function (event) {
                       // this.owner.content('main', event);
                       if(event.target == 'methbr_ref')
                       		methodsb_config.reload();
                    }

                } 
            },
            { 
            	type: 'main', 
            	//style: 'padding: 0px;', 
            	content: editor_html,
            	toolbar: {
                    items: [
                        { type: 'html',  id: 'source_header',
                			html: '<div id="src_hd" style="padding: 3px 10px;font-weight:bold;">'+
                      		' Source code'+'</div>' 
            			}			,
                        { type: 'spacer' },
                         { type: 'button',  id: 'ed_ac_reload',  caption: '', icon: 'fa-repeat', hint: 'Reload AC keywords' },
                        { type: 'button',  id: 'src_save',  caption: 'Save', icon: 'fa-save', hint: 'Save method' }
                    ],
                    onClick: function (event) {
                       // this.owner.content('main', event);
                       switch(event.target)
                       {
                       		case 'src_save':
                       			methodsb_config.save_method();
                       			break;
                       		case 'ed_ac_reload':
                       			$.post( "/ffvm/run_on_ws", {code:"imgMeta imageKeywords"});
                       			break;
                       		default:

                       }
                    }
                }

            }
        ]
	},
	class_content_layout:
	{
		name: 'class_content_layout',
        panels: [
        	{ 	
            	type: 'main', 
            	style: 'padding: 0px;', 
            	content: ''
            },
            { 	
            	type: 'bottom', 
            	style: 'padding: 0px;', 
            	content: '',
            	size:200,
            	resizable: true,
            	toolbar: {
                    items: [
                    	{ type: 'html',  id: 'ivbr_header',
                			html: '<div style="padding: 3px 10px; font-weight:bold;">'+
                      		'Instance variables'+'</div>' 
            				},
                        	{ type: 'spacer' },
                        { type: 'menu',   id: 'iv_ed', caption: '', icon: ' fa-edit', items: [
                            { text: 'New instance variable', icon: 'fa-plus' }, 
                            { text: 'Edit variable', icon: 'fa-pencil' }, 
                            { text: 'Delete variable', icon: 'fa-trash' }
                        ]},
                        { type: 'button',  id: 'vrsbr_ref',  caption: '', icon: 'fa-refresh'}
                    ],
                    onClick: function (event) {
                       // this.owner.content('main', event);
                       if(event.target == 'vrsbr_ref')
                       		ivb_config.reload();
                    }
                } 
            }
        ]
	},
	save_image()
	{
		w2confirm('Do you want to save the current image', function (btn) {
			if(btn == "Yes")
			{
				$.post( "/ffvm/save_image", {name:'backup'})
  					.done(function( data ) 
  					{
  						w2alert("The image is saved");
  					});
				}
		});
	}
}

// init the main layout
$(function () {
	$().w2layout(playground_config.layout);
	$().w2layout(config.class_content_layout);
	$().w2layout(config.right_layout);
    $('#layout').w2layout(config.layout);
    w2ui.class_content_layout.content('main',w2ui.method_sidebar);
    w2ui.class_content_layout.content('bottom',w2ui.iv_sidebar);
    w2ui.right_layout.content('left',w2ui.class_content_layout);
    w2ui.layout.content('left', w2ui.cls_sidebar);
    w2ui.layout.content('main', w2ui.right_layout);
    $('#bt_workspace').click(function()
    {
    	playground_config.open_pl_editor();
    	//console.log('open WS');
    });
    $('#bt_save_image').click(function()
    {
    	config.save_image();
    	//console.log('open WS');
    });
});

