var dummy_config = {
	name: 'dummygrid', 
    header: 'Dummy plugin',
    url: '/dummy',
    method: 'GET', 
    show: {
        header  : false,
        toolbar : true,
        footer  : true,
        lineNumbers : true,
        toolbarAdd: true
    },        
    columns: [                
        { field: 'id', caption: '#', size: '10%' },
        { field: 'color', caption: 'Text', size: '90%'}
    ],
    searches: [
        { type: 'text', field: 'color', caption: 'Text' }
    ],
    //event
    onAdd: function (evt) {
    }
}
$().w2grid(dummy_config);
