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
* $Id: web_folder_edit_fns.inc,v 1.3 2006/12/05 23:43:20 bcaldwell Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/site/site_edit_fns.inc';

/**
* Web_Folder_Edit_Fns
*
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Web_Folder_Edit_Fns extends Site_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Web_Folder_Edit_Fns()
	{
		$this->Site_Edit_Fns();
		unset($this->static_screens['preview']);

	}//end constructor


}//end class

?>