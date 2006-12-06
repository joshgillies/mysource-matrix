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
* $Id: designs_folder_management.inc,v 1.9 2006/12/06 04:24:01 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Designs_Folder_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Designs_Folder_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Designs_Folder_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
						'name'	=> Array(
									'default'	=> 'System Designs',
								   ),
					  );

	}//end constructor


}//end class
?>
