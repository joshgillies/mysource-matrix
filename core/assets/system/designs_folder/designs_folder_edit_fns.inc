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
* $Id: designs_folder_edit_fns.inc,v 1.9 2006/12/06 04:24:01 emcdonald Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_edit_fns.inc';

/**
* Designs_Folder_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Designs_Folder_Edit_Fns extends Folder_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Designs_Folder_Edit_Fns()
	{
		$this->Folder_Edit_Fns();

		unset($this->static_screens['settings']);
		unset($this->static_screens['preview']);
		unset($this->static_screens['lookupValues']);
		unset($this->static_screens['web_paths']);
		unset($this->static_screens['metadata']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['workflow']);
		unset($this->static_screens['tagging']);
		unset($this->static_screens['roles']);
		unset($this->static_screens['permissions']);

	}//end constructor


}//end class
?>
