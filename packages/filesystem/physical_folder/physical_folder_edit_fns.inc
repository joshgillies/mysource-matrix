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
* $Id: physical_folder_edit_fns.inc,v 1.2 2012/08/30 00:58:36 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Physical_Folder_Edit_Fns
*
* Purpose
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage file
*/
class Physical_Folder_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		// override static screens - only display details because the others don't apply
		$this->static_screens = Array(
									'details'	=> Array(
													'name'			=> translate('Details'),

													'force_unlock'	=> FALSE,
													'lock_type'		=> 'none',
												   ),
								);
		unset($this->static_screens['web_paths']);

	}//end constructor

}//end class

?>
