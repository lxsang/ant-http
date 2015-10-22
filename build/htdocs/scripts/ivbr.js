var ivb_config = {
	name: 'iv_sidebar',
	nodes: [ 
	],
	onClick:function(event)
	{
	},
	reload:function()
	{
		if(w2ui.cls_sidebar.selected)
		{
			$.post( "/ffvm/variables_of", { class:w2ui.cls_sidebar.selected})
  			.done(function( data ) 
  			{
  				remove_all_nodes_of(w2ui.iv_sidebar);
    			$.each(data, function(idx,val)
    			{
    				w2ui.iv_sidebar.add({id:val, text:val,icon: 'fa-cog'});
    			});
  			});
		
		}
	}
}
$().w2sidebar(ivb_config);

