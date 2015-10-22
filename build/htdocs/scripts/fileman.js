var fileman_gird = {
	name: 'filemangrid',
	header:'Files', 
    url: '/fileman',
    method: 'GET', 
    show: {
        header  : false,
        toolbar : true,
        footer  : true,
        lineNumbers : true,
        toolbarDelete: true
    },        
    columns: [
    	{ field: 'type', caption: 'Type', size: '0%' ,hidden:true},                
        { field: 'name', caption: 'Name', size: '30%' },
        { field: 'size', caption: 'Size (bytes)', size: '15%' },
        { field: 'mime', caption: 'Type', size: '25%' },
        { field: 'changed', caption: 'Last changed', size: '30%' }
    ],
    searches: [
        { type: 'text', field: 'name', caption: 'Plugin name' },
        { type: 'date', field: 'changed', caption: 'Last changed' }
    ],
    toolbar: {
    	items: [
        	{ type: 'button',  id: 'bt_add_file',  caption: 'Add file', icon: 'w2ui-icon-plus' },
        	{
        		type: 'html', 
        		id: 'file_select', 
        		html: '<form id="file_add_form" action="javascript:;" style="display:none;">'
        			+'<input id="file_add_file" type="file" name="pfile" />'
        			+'<input id="file_add_path" type="hidden" name="path" />'
        			+'</form>'
        	},
        	 { type: 'button',  id: 'bt_create_folder',  caption: 'Create folder', img: 'icon-folder' }
    	],
    	onClick: function (target, data) {
       		switch(target)
       		{
       			case 'bt_add_file':
       				w2ui.filemangrid.add_file();
       				break;
       			case 'bt_create_folder':
       				w2popup.open({
			        	title     : 'New folder',
			        	body      : '<div class="w2ui-centered">Folder name: <br/><input type="text" id="mkdir_dname" size="50" /></div>',
			        	buttons   : '<button class="btn" onclick="w2ui.filemangrid.mkdir();w2popup.close();">Create</button>',
				        width     : 400,
				        height    : 200,
				        overflow  : 'hidden',
				        color     : '#333',
				        speed     : '0.3',
				        opacity   : '0.8',
				        modal     : true,
				        showClose : true
			    	});
       				break;
       			default:
       				console.log(data);
       		}
    	}, 
	},
	onDelete: function (event) {
		event.preventDefault();
    	w2confirm('Are you sure you want to delete selected file/record ?', function (btn) {
    		if(btn == "Yes")	
    		{
    			var sel = w2ui.filemangrid.getSelection();
    			var row = w2ui.filemangrid.get(sel);
    			var path = w2ui.filemangrid.folder;
    			if(row)
    			{
    				$.post( "/fileman/rmfolder", { name: row.name, path: path})
	  				.done(function( data ) 
	  				{
	    				if(data.result == 1)
					 		w2ui.filemangrid.reload();
					 	else
					 		w2alert(data.msg);
	  				});
    			}
    		}	 
    	});
    },
	onDblClick: function(event) {
		var el = this.get(event.recid);
		if(el.type == 0)
		{
			this.url = "/fileman?path="+ this.folder+"/"+ el.name;
			this.reload();
		}
		else
        	console.log(el);
    },
	route_path:function(router)
	{
		if(router)
		{
			var toolb = w2ui.fileman_layout.panels[0].toolbar;
			var icon;
			toolb.items = [];
			for(i=0;i< router.length;i++)
			{
				if(i==0)
					icon = 'fa-folder-open';
				else
					icon = 'fa-angle-right';
				toolb.items[i] = { 
					type: 'button',  
					id: 'folder_router_'+ i,  
					caption:router[i].name, 
					icon:icon,
					path:router[i].path};
			}
			
			toolb.render();
			//console.log(this);
		}
	},
	onLoad:function(event)
	{
		var obj;
		if(event.xhr.statusText == "OK")
		{
			obj = JSON.parse(event.xhr.responseText);
			this.route_path(obj.router);
		}
	},
	mkdir:function()
	{
		var dname  = $("#mkdir_dname").val();
		//open pop up
		$.post( "/fileman/mkfolder", { dname: dname, path: this.folder})
  			.done(function( data ) 
  			{
    			if(data.result == 1)
				 		w2ui.filemangrid.reload();
				 	else
				 		w2alert(data.msg);
  			});
	},
    //event
    add_file: function () {
    	$('#file_add_form').unbind('submit');
    	$('#file_add_form').unbind("change");
    	$('#file_add_path').val(this.folder);
    	$('#file_add_form').submit(function(event){
    		event.preventDefault();
			var formData = new FormData($(this)[0]);
			$.ajax({
				url: '/fileman/add',
				type: 'POST',
				data: formData,
				async: false,
				cache: false,
				contentType: false,
				processData: false,
				success: function (returndata) {
				 	if(returndata.result == 1)
				 		w2ui.filemangrid.reload();
				 	else
				 		w2alert(returndata.msg);
				}
			});
			return false;
    	});
    	$('#file_add_file').on("change",function(){
    		//console.log("change in file");
			$('#file_add_form').trigger('submit');
			return false;
    	});
    	$('#file_add_file').trigger('click');
    		
    }
}
var fileman_layout = {
	name:"fileman_layout",
	panels: [
	{
		type:'top',
		size: 29, 
        style: 'padding: 0px;', 
		toolbar:{
			items: [
            ] ,
            onClick: function (target, data) {
            	w2ui.filemangrid.url = "/fileman?path="+data.object.path;
            	w2ui.filemangrid.reload();
       			//console.log(data.object.path);
    		}, 
		}
	},
	{
		type:'main',
		content:''
	}

	]
}
$().w2grid(fileman_gird);
$().w2layout(fileman_layout);
w2ui.fileman_layout.content('main',w2ui.filemangrid);
