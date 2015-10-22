var sidebar_config = {
	name: 'sidebar',
	nodes: [ 
	    { 
	    	id: 'level-sys', 
	    	text: 'System', 
	    	img: 'icon-folder', 
	    	expanded: true, 
	    	group: true,
	      	nodes: 
	      	[ 
	      		{ 
	      			id: 'level-plug', 
	      			text: 'Plugins', 
	      			icon: 'fa-cogs' 
	      		},
	            { id: 'level-fman', text: 'File manager', icon: 'fa-folder-open' }
	        ]
	    },
	    { 
	    	id: 'level-api', 
	    	text: 'RPI. Car', 
	    	img: 'icon-folder', 
	    	group: true,
	    	expanded:true,
	      	nodes: 
	      	[ 
	            { id: 'level-picam', text: 'Camera viewer', icon: 'fa-camera' },
	           // { id: 'level-2-3', text: 'Level 2.3', icon: 'fa-star-empty' }
	        ]
	    }
	],
	onClick:function(event)
	{
		switch(event.target)
		{
			case 'level-plug':
				w2ui.layout.content('main', w2ui.pluginsgrid);
				break;
			case 'level-fman':
				w2ui.layout.content('main',w2ui.fileman_layout);
				//w2ui.filemangrid.url = '/fileman';
				//w2ui.filemangrid.reload();
				//w2ui.filemangrid.route_path();
				//console.log(w2ui.fi);
				break;
			case  'level-picam':
				w2ui.layout.content('main', w2ui.picam_layout);
				break;
			default: //do nothing
				w2ui.layout.content('main',"Empty page");
				console.log('Event: ' + event.type + ' Target: ' + event.target);
				console.log(event);
		}
	}
}
$().w2sidebar(sidebar_config);
