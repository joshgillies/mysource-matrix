<?php

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_edit_fns.inc';

/**
* Designs_Folder_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Designs_Folder_Edit_Fns extends Folder_Edit_Fns
{

	/**
	* An array of tab code names and their display names
	* @var Array()
	*/
	var $tabs = Array('details' => Array('name' => 'Details',
										  'icon' => 'create'
										  )
					   );


	/**
	* Constructor
	*
	*/
	function Designs_Folder_Edit_Fns() 
	{
		$this->Folder_Edit_Fns();
	}

}//end class
?>
