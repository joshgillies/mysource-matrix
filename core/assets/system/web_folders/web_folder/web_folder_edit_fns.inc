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
* $Id: web_folder_edit_fns.inc,v 1.6 2012/08/30 01:09:16 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/site/site_edit_fns.inc';

/**
* Web_Folder_Edit_Fns
*
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Web_Folder_Edit_Fns extends Site_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		unset($this->static_screens['preview']);
		unset($this->static_screens['layouts']);

	}//end constructor


}//end class

?>
