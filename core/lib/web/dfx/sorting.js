if (!window.dfx) {
	window.dfx = function() {};
	window.dfxjQuery = $.noConflict(true);
}

/**
 * Table row reordering with jquery ui.
 *
 * The function can be used to do sorting in row sorting within table
 *
 * @param {string}  tableId         Id of the table should pass with '#''
                                    eg. '#test'
 * @param {String}  tableClass      Class name of the table should pass with '.''
 *                                  eg. '.test'
 * @param {class}   dragableColumn  Classname of the td which is used to drag. Pass 'all' if all column can be dragged.
 *
 * @type void
 */
dfx.tableSort = function(tableId, tableClass, dragableColumn)
{
	if(!tableId || !tableClass) return;
	var $optionsTable = dfxjQuery(tableId+tableClass).find('tbody');
	$optionsTable.sortable({
		scroll: false,
		start: function(e, ui){
			ui.placeholder.height(ui.item.height());
		},
		helper: function(e, ui) {
			ui.addClass('sq-sortable-dragging');
			// Handle the collapsing of the width of the row's columns when dragging
			ui.children().each(function() {
				var $column = dfxjQuery(this);
				$column.width($column.width());
			});
			return ui;
		},//end helper()
		axis: 'y',
		placeholder: "sq-sortable-placeholder"
	});
	// enable dragging from selected column
	if(dragableColumn !== 'all') {
		dfxjQuery(tableId+tableClass+' td').not(dragableColumn).mousedown(function(event){
			event.stopImmediatePropagation();
		});
	};

	$optionsTable.find('tr').attr('title', 'Drag to Reorder');
	$optionsTable.find('input').attr('title', '');
};

