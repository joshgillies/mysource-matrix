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
* $Id: tool_edit_fns.inc,v 1.2 2006/12/06 00:13:44 bcaldwell Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Tools_Edit_Fns
*
* @author Nathan de Vries <ndvries@squiz.net>
* @author David Heppell <dheppell@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Tool_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Tool_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end constructor


}//end class
