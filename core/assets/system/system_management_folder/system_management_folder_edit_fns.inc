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
* $Id: system_management_folder_edit_fns.inc,v 1.9 2008/05/02 01:57:10 bpearson Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_edit_fns.inc';

/**
* System_Management_Folder_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class System_Management_Folder_Edit_Fns extends Folder_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		unset($this->static_screens['permissions']);
		unset($this->static_screens['workflow']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['metadata']);
		unset($this->static_screens['dependants']);
		unset($this->static_screens['linking']);
		unset($this->static_screens['settings']);
		unset($this->static_screens['history']);
		unset($this->static_screens['preview']);
		unset($this->static_screens['roles']);
		unset($this->static_screens['tagging']);
		unset($this->static_screens['lookupValues']);
		unset($this->static_screens['layouts']);

	}//end constructor


}//end class
?>
