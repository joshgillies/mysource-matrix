<?php
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
* $Id: design_area_menu_recursive_edit_fns.inc,v 1.25 2006/12/05 22:42:14 emcdonald Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_areas/menu/design_area_menu_type/design_area_menu_type_edit_fns.inc';

/**
* Design_Area_Menu_Recursive_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.25 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Menu_Recursive_Edit_Fns extends Design_Area_Menu_Type_Edit_Fns
{

	/**
	* The width of the arrow image
	* @var int
	*/
	var $arrow_width  = 4;

	/**
	* The height of the arrow image
	* @var int
	*/
	var $arrow_height = 6;


	/**
	* Takes the contents for the sub menu element and attempts to process it,
	* is a separate fn to allow menu types that don't want to allow sub menu's to override
	*
	* @param object	&$asset		the asset to which we belong
	* @param array	&$contents	the sub menu's contents array that we generated with _parseString()
	*
	* @return boolean
	* @access private
	* @see Design_Area_Menu_Type_Edit_Fns::_processContents();
	*/
	function _processSubMenu(&$asset, &$contents)
	{
		// we just want to ignore any sub menu's in our asset contents
		trigger_localised_error('CORE0215', E_USER_WARNING);
		return FALSE;

	}//end _processSubMenu()


}//end class
?>
