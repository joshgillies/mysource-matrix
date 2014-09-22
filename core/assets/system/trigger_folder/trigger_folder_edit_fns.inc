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
* $Id: trigger_folder_edit_fns.inc,v 1.3 2012/08/30 01:09:12 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_edit_fns.inc';


/**
* Trigger_Folder_Edit_Fns
*
* @author Edison Wang <ewang@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Folder_Edit_Fns extends Folder_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Trigger_Folder_Edit_Fns()
	{
		$this->Folder_Edit_Fns();

		$this->static_screens = Array();
		// enable just the details
		// disable lock section
		$this->static_screens['details']['name'] = translate('Details');

		$this->static_screens['details']['force_unlock'] = FALSE;
		$this->static_screens['details']['lock_type'] = 'none';

	}//end constructor


/**
	* Paint the interface for editing trigger folder details
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDetails(&$asset, &$o, $prefix)
	{

		if(!isset($asset->name) || empty($asset->name)) {
			return FALSE;
		}

		// display category delimited with '/'
		$category = explode(':',$asset->name);
		array_shift ($category);
		$category_path = implode('/',$category);

		$o->openField(translate('Category'));

		echo ($category_path);
		$o->closeField();
		// no committing allowed
		return FALSE;

	}//end paintDetails()


	/**
	* Processes the interface for editing trigger folder details
	*
	* Editing the details of an LDAP group is not yet implemented
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processDetails(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processDetails()

}//end class

?>
