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
* $Id: tag_manager_edit_fns.inc,v 1.3 2012/08/30 01:09:12 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';


/**
* Tag_Manager_Edit_Fns
*
* Purpose
*   Tag Manager's Edit Functions
*
* @author  Elden McDonald  <emcdonald@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Tag_Manager_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Tag_Manager_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

		// disable all static screens
		$this->static_screens = Array();

		// enable just the details
		$this->static_screens['details']['name']         =translate('details');
		$this->static_screens['details']['force_unlock'] = FALSE;
		$this->static_screens['details']['lock_type']    = 'attributes';

	}//end constructor


}//end class

?>
