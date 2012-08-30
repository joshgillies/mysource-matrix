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
* $Id: root_folder_management.inc,v 1.16 2012/08/30 01:09:11 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Root_Folder_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.16 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Root_Folder_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Root_Folder_Management(&$pm)
	{
		$this->Asset_Management($pm);

		// only need to have the default here because the other information will be
		// gleaned from the 'folder' type
		$this->vars = Array(
						'name'	=> Array(
									'default'	=> 'Root Folder',
								   ),
					  );


	}//end constructor


}//end class
?>
