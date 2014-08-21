<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: menu_get_assets.inc,v 1.19 2012/08/30 01:09:06 ewang Exp $
*
*/


/**
* Returns an array of assetids corresponding to valid menu entries
*
* @param string	$id_name	the name of the menu design area
* @param string	$majorids	an array of major ids for each level of the menu
*
* @return array
* @access public
*/
function menu_get_assets($id_name, $majorids, $ignore_permissions = FALSE)
{

	$db = MatrixDAL::getDb();

	if (!isset($GLOBALS['MENU_DATA'][$id_name])) {
		$GLOBALS['MENU_DATA'][$id_name] = Array();
	}
	$tree_data =& $GLOBALS['MENU_DATA'][$id_name]; // need ref

	$assetids = Array();
	$GLOBALS['SQ_SYSTEM']->am->getAssetTree($majorids, 1, Array(), SQ_SC_LINK_FRONTEND_NAV, Array(), TRUE, $tree_data, $ignore_permissions);
	if (!empty($tree_data)) {
		foreach($tree_data as $row) {
			if (!empty($row) && is_array($row)) {
				$assetids = array_merge($assetids, array_keys($row));
			}//end if
		}//end foreach
	}//end if

	return $assetids;

}//end menu_get_assets()


?>
