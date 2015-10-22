var playground_config = {
	layout:{
		name: 'pg_layout',
        panels: [
            { 
            	type: 'main', 
            	style: 'padding: 0px;', 
            	content: 'main'
            },
            { 	
            	type: 'right', 
            	size: 300, 
            	resizable:true,
            	style: 'padding: 0px;', 
            	content: '<textarea id = "pg_log" readonly></textarea>',
            	toolbar: {
                    items: [
                    	{ type: 'button',  id: 'pl_run',  caption: 'Run', icon: 'fa-caret-right'},
                        { type: 'spacer' },
                        { type: 'button',  id: 'pl_clear_log',  caption: '', icon: 'fa-trash'}
             			//{ type: 'html',  id: 'pl_header',
                		//	html: '<b>Console</b>' 
            			//}
                        
                    ],
                    onClick: function (event) {
                       	if(event.target == 'pl_run')
                       	{
                       		playground_config.run_code();
                       	} 
                       	else if(event.target=='pl_clear_log')
                       	{
                       		$("#pg_log").val("");
                       	}
                    }
                } 
            	
            }
        ]
	},
	open_pl_editor()
	{
		// create popup with editor
		var html = gen_editor_for('pl_editor',
			'Object respondsTo keys do:[:e| e print]');
		//console.log(html);
		w2popup.open({
        	title: 'Play Ground',
        	body:'<div id="pl_editor_main" style="position: absolute; left: 0px; top: 0px; right: 0px; bottom: 0px;"></div>',
        	width:700,
        	height:500,
        	buttons: '',
        	modal:true,
        	opacity: 0,
        	showMax: false,
        	onOpen:function (event) {
        		event.onComplete = function () {
               		$('#w2ui-popup #pl_editor_main').w2render('pg_layout');
                	w2ui.pg_layout.html('main', html);
            	}
        	},
        	onToggle: function (event) { 
            	event.onComplete = function () {
                w2ui.pg_layout.resize();
            }
        }      
    	}).unlockScreen();
	},
	run_code:function()
	{
		if(pl_editor)
		{
			var code = pl_editor.getValue().trim();
			if(/[a-zA-Z0-9]+/.test(code))
			{
				//code = code.replace(/\'/g,"''");
				if(code.charAt(code.length-1) == '.')
					code = code.substring(0,code.length-1);
				$.post( "/ffvm/run_on_ws", {code:code})
  				.done(function( data ) 
  				{
  					if(data.result)
  					{
  						playground_config.load_log();
  					}
  					else
  						w2alert("Error: cannot execute code");
  				});
			}
			else
				w2alert("Please enter code to execute");
		}
	},
	load_log:function()
	{
		$.get( "/ffvm/ws_log", {})
  			.done(function( data ) 
  			{
  				console.log(data);
  				$("#pg_log").val(data);
  			});
	}
}
