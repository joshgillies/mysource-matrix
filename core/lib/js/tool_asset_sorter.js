/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: tool_asset_sorter.js,v 1.1 2008/01/31 05:56:06 mbrydon Exp $
*
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix
*/


// Control hiding / display of the "Sort Field" and "Sort Attribute" interfaces
function setFieldType(combo) {
	var sort_field = document.getElementById('sort_field');
	var sort_asset = document.getElementById('sort_attr_asset');
	var sort_attr = document.getElementById('sort_attr_attr');

	if (combo.value == 'sort_by_field') {
		sort_asset.style.display = 'none';
		sort_attr.style.display = 'none';
		sort_field.style.display = '';
	} else {
		sort_field.style.display = 'none';
		sort_asset.style.display = '';
		sort_attr.style.display = '';
	}
}
