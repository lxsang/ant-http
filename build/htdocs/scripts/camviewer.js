var picam_config = {
	name: 'picam_layout',
	toolbar:{
		name: 'picam_toolbar',
    	items: [
        	{ type: 'check',  id: 'picam_capture', caption: 'Start capture', icon: 'fa-camera-retro', checked: false },
        	{ type: 'break',  id: 'break0' },
        	{ type: 'menu',   id: 'picam_res', caption: 'Resolution', icon: 'fa-picture', 
        	items: [
            	{ text: '360x240', icon: 'fa-th'},  
            	{ text: '640x480', icon: 'fa-th'}
       		]},
        	{ type: 'break', id: 'break1' },
        	{ type: 'radio',  id: 'item3',  group: '1', caption: 'Radio 1', icon: 'fa-star', checked: true },
        	{ type: 'radio',  id: 'item4',  group: '1', caption: 'Radio 2', icon: 'fa-heart' },
        	{ type: 'break', id: 'break2' },
        	{ type: 'drop',  id: 'item5', caption: 'Drop Down', icon: 'fa-plus', html: '<div style="padding: 10px">Drop down</div>' },
        	{ type: 'break', id: 'break3' },
        	{ type: 'spacer' },
        	{ type: 'button',  id: 'item7',  caption: 'Item 5', icon: 'fa-flag' }
        ],
        onClick: function (event) {
            
            switch(event.target)
			{
    			case 'picam_capture':
    				//console.log('checked is'+event.item.checked);
    				 if(event.object.checked == false)
    				{
    					console.log("Starting capture");
    					$("#pi_img").load(function() {
                            if (this.complete && typeof this.naturalWidth != "undefined" || this.naturalWidth != 0) {
                            	$("#pi_img").attr("src", "/camviewer?fetch="+new Date().getTime());
                            }
                        });
                        $("#pi_img").attr("src", "/camviewer?fetch="+new Date().getTime());
    				}
    				else
    				{
    					$("#pi_img").unbind('load');
    				}
    				break;
    			default: //do nothing
    				console.log('Event: ' + event.type + ' Target: ' + event.target);
    				console.log(event);
			}
        }
	},
    panels: [
        { 	
        	type: 'top', 
        	size: 50, 
        	style: 'padding: 5px;', 
        	content: 'top'
        },
        { 
        	type: 'main', 
        	style: 'padding: 0px;', 
        	content: '<div style="padding: 10px"><img id="pi_img" src="/camviewer"/></div>' 
        }
    ]
}
$().w2toolbar(picam_config.toolbar)
$().w2layout(picam_config);
w2ui.picam_layout.content('top',w2ui.picam_toolbar);
