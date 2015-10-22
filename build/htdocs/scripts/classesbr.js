var class_tpl = "Object addSubClass: #Foo instanceVariableNames: 'x y'";

function gen_editor_for(id,default_val)
{
	return '<div id="'+id+'" style="position: absolute;top:0;right: 0;bottom: 0;left: 0;"> \n'
			+ default_val
			+ '</div>'		
	    	+'<script>\n'
	    	+ 'ace.require("ace/ext/language_tools");'
	    	+'var '+id+' = ace.edit("'+id+'");\n'
			+ ''+id+'.setOptions({'
	    	+ 'enableBasicAutocompletion: true,'
	    	//+ 'enableSnippets: true,\n'
	    	+ 'enableLiveAutocompletion: true \n'
			+ '});'
	    	+id + '.setTheme("ace/theme/monokai");\n'
	   	 	+id + '.getSession().setMode("ace/mode/text");\n'
			+'</script>'
}
	
var sclbrr_config = {
	name: 'cls_sidebar',
	nodes: [ ],
	onClick:function(event)
	{
		$.post( "/ffvm/classinfo", { class: event.target})
  			.done(function( data ) 
  			{
  				remove_all_nodes_of(w2ui.method_sidebar);
  				remove_all_nodes_of(w2ui.iv_sidebar);
  				//w2ui.method_sidebar.remove(w2ui.method_sidebar.nodes);
				//console.log(data);
    			$.each(data[0], function(idx,val)
    			{
    				w2ui.method_sidebar.add({id:val, text:val,icon: 'fa-cog'});
    			});
    			$.each(data[1], function(idx,val)
    			{
    				w2ui.iv_sidebar.add({id:val, text:val,icon: 'fa-lock'});
    			});
  			});
  		// now get instance variables
  		
	},
	reload: function()
	{
		
		remove_all_nodes_of(w2ui.cls_sidebar);
		w2ui.cls_sidebar.add({id:'Object', 
				text:'Object',icon: 'fa-sitemap',expanded: true});
		$.getJSON( "/ffvm", function(data) {
  			$.each(data.Object, function (index,value) {
  				insert_item_to(w2ui.cls_sidebar,"Object",value);
  			});
		});
	},
	openClassEditor:function()
	{
		// create popup with editor
		var html = gen_editor_for('cls_editor',class_tpl);
		//console.log(html);
		w2popup.open({
        	title: 'Create new class',
        	body:html ,
        	buttons: '<button class="btn" onclick="sclbrr_config.newClass()">Create</button>',
        	modal:true,
        	opacity: 0,
        	width:500,
        	height:300,
        	showMax: false
    	}).unlockScreen();
	},
	newClass:function() {
		var cls,superClas;
		var code = cls_editor.getValue();
		var tmp_arr = code.split("instanceVariableNames:");
		if(tmp_arr.length != 2)
		{
			w2alert('Wrong code syntax!!!');
			return;
		}
		tmp_arr = tmp_arr[0].split("addSubClass:");
		if(tmp_arr.length != 2)
		{
			w2alert('Wrong code syntax!!!');
			return;
		}
		cls = tmp_arr[1].replace("#","").trim();
		superClas = tmp_arr[0].trim();
		// check if super class exist
		if(w2ui.cls_sidebar.find({id:superClas}).length == 0)
		{
			w2alert('Superclass : ' + superClas + ' not found!!');
			return;
		}
		if(w2ui.cls_sidebar.find({id:cls}).length != 0)
		{
			w2alert('Class : ' + cls + ' is already exist!!');
			return;
		}
		// it's now safe to send the code to server
		$.post( "/ffvm/new_class", { class:cls, code:code})
  			.done(function( data ) 
  			{
  				//console.log(data);
  				if(data.result)
  				{
  					sclbrr_config.reload();
  					w2ui.cls_sidebar.select(cls);
  					w2popup.close();
  				}
  			});
		
	}
}
function remove_all_nodes_of(sidebar)
{
	var nd = []; 
	for (var i in sidebar.nodes) nd.push(sidebar.nodes[i].id);
	sidebar.remove.apply(sidebar, nd);
}
function insert_item_to(el, id, obj)
{
	switch($.type(obj))
	{
		case "object":
			$.each(obj, function(key,val){
				el.insert(id,null,
					[{id:key, text:key,icon: 'fa-sitemap'}]);
				insert_item_to(el,key,val);
			});
			break;
		case "array":
			//console.log("ARRAY");
			$.each(obj, function  (idx,val) {
				insert_item_to(el,id,val);
			})
		break;
		default:
			el.insert(id,null,
					[{id:obj, text:obj,icon: 'fa-th-list'}]);
			break;

	}
}
$().w2sidebar(sclbrr_config);
w2ui.cls_sidebar.on('render', function (event) {
    sclbrr_config.reload();
 });
