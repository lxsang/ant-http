var methodsb_config = {
	name: 'method_sidebar',
	nodes: [ 
	],
	onClick:function(event)
	{
		$.post( "/ffvm/source", { method: event.target, class:w2ui.cls_sidebar.selected})
  			.done(function( data ) 
  			{
  				editor.setValue(data);
  				$('#src_hd').html(
  					w2ui.cls_sidebar.selected+
  						">>"+
  						event.target
  				);
  				//console.log(w2ui.right_layout.get('main'));
  				/*$("#editor_header").html(
  						"<p>"+
  						w2ui.cls_sidebar.selected+
  						">>"+
  						event.target+
  						"</p>"
  				);*/
  			});
	},
	reload:function()
	{
		if(w2ui.cls_sidebar.selected)
		{
			$.post( "/ffvm/methods_of", { class:w2ui.cls_sidebar.selected})
  			.done(function( data ) 
  			{
  				remove_all_nodes_of(w2ui.method_sidebar);
    			$.each(data, function(idx,val)
    			{
    				w2ui.method_sidebar.add({id:val, text:val,icon: 'fa-cog'});
    			});
  			});
		
		}
	},
	save_method:function()
	{
		if(!w2ui.cls_sidebar.selected) return;
		//regular expression to detect method
		var reg = /^[\s\t\n\r]*((((\~\=)|(\/\/)|([\,\=\>\<\+\-\*\/])|(\>\=)|(\<\=)|(\=\=))[\s\t\n\r]*[a-zA-Z][a-zA-Z0-9]*)|([a-zA-Z][a-zA-Z0-9]*(\:[\s\t\n\r]*[a-zA-Z][a-zA-Z0-9]*[\s\t\n\r]+([\s\t\n]*[a-zA-Z][a-zA-Z0-9]*\:[\s\t\n\r]*[a-zA-Z][a-zA-Z0-9]*[\s\t\n\r]+)*)?))[\s\t\n\r]*/;//has some bugs
		code = editor.getValue();
		result = code.match(reg);
		var t,m,p;
		if(!result || result.length == 0 || !result[0])
			w2alert('Invalid function definition');
		else
		{
			t = result[0].trim();
			if(/:/.test(t))
			{
				m = "";
				while((p = t.indexOf(":")) != -1)
				{
					m += t.substring(0,p).trim() + ":";
					t = t.substring(p+1,t.length).trim();
					if((p = t.indexOf(" ")) != -1)
						t = t.substring(p+1,t.length).trim();
					//console.log('"'+t+'"');
				} 
				//console.log(m);
			}
			else
			{
				result = t.match(/((\~\=)|(\/\/)|([\,\=\>\<\+\-\*\/])|(\>\=)|(\<\=)|(\=\=))/);
				if(result && result.length > 0 && result[0])
					m = result[0];
				else
					m = t;
			}
			console.log(m);
		}
		//code = code.replace(/\'/g,"''");
		if(w2ui.method_sidebar.find({id:m}).length == 0)
		{
			$.post( "/ffvm/new_method", { 
				class:w2ui.cls_sidebar.selected,
				code:code})
  			.done(function( data ) 
  			{
  				if(data.result)
  				{
  					methodsb_config.reload();
  					w2ui.method_sidebar.select(m);
  				}
  				else
  					w2alert("Error: cannot save the method");
  			});
		}
		else
		{
			$.post( "/ffvm/update_method", { 
				class:w2ui.cls_sidebar.selected,
				code:code,
				method:m})
  			.done(function( data ) 
  			{
  				if(data.result)
  				{
  					methodsb_config.reload();
  					w2ui.method_sidebar.select(m);
  				}
  				else
  					w2alert("Error: cannot save the method");
  			});
		}
	}
}
$().w2sidebar(methodsb_config);

