var pluginman_config = {
	name: 'pluginsgrid', 
    header: 'Installed plugins',
    url: '/pluginsman',
    method: 'GET', 
    show: {
        header  : false,
        toolbar : true,
        footer  : true,
        lineNumbers : true,
        toolbarAdd: true
    },        
    columns: [                
        { field: 'name', caption: 'Plugin name', size: '30%' },
        { field: 'size', caption: 'Size (bytes)', size: '30%' },
        { field: 'changed', caption: 'Last changed', size: '40%' }
    ],
    searches: [
        { type: 'text', field: 'name', caption: 'Plugin name' },
        { type: 'date', field: 'changed', caption: 'Last changed' }
    ],
    toolbar: {
    	items: [
        	//{ type: 'break' },
        	{
        		type: 'html', 
        		id: 'file_select', 
        		html: '<form id="pinstaller_form" action="javascript:;" style="display:none;">'
        			+'<input type="hidden" name="test" value="10"/><input id="pinstaller_file" type="file" name="pfile" />'
        			+'</form><span style="display:none;" id="pinstaller_progr">Loading</p>'
        	}
    	],
    	onClick: function (target, data) {
       		console.log(target);
    	}
	},
    //event
    onAdd: function (evt) {
    	$('#pinstaller_form').unbind('submit');
    	$('#pinstaller_file').unbind("change");
    	$('#pinstaller_form').submit(function(event){
    		event.preventDefault();
    		 $('#pinstaller_progr').css("display","inline");
			var formData = new FormData($(this)[0]);
			$.ajax({
				url: '/pluginsman/install',
				type: 'POST',
				data: formData,
				async: false,
				cache: false,
				contentType: false,
				processData: false,
				success: function (returndata) {
					$('#pinstaller_progr').css("display","none");
				 	if(returndata.result == 1)
				 		w2ui.pluginsgrid.reload();
				 	else
				 		w2alert(returndata.msg);
				}
			});
			return false;
    	});
    	$('#pinstaller_file').on("change",function(){
    		//console.log("change in file");
			$('#pinstaller_form').trigger('submit');
			return false;
    	});
    	$('#pinstaller_file').trigger('click');
    		
    }
}
$().w2grid(pluginman_config);
