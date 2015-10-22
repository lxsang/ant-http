// system config
var config = {
	layout:{
		name: 'layout',
        panels: [
            { 	
            	type: 'top', 
            	size: 29, 
            	style: 'padding: 0px;', 
            	content: '',
            	title: 'RCAR HTTP Server Manager'
            },
            { 
            	type: 'left', 
            	size: 160, 
            	style: 'padding: 0px;', 
            	content: 'left' 
            },
            { 
            	type: 'main', 
            	style: 'padding: 0px;', 
            	content: 'main' 
            }
        ]
	}
}

// init the main layout
$(function () {
    $('#layout').w2layout(config.layout);
    w2ui.layout.content('left', w2ui.sidebar);
    w2ui.layout.content('main', w2ui.pluginsgrid);
});

