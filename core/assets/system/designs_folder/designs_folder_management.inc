<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: designs_folder_management.inc,v 1.4 2003/10/15 11:05:15 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Designs_Folder_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
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
		$this->vars =	Array(
							'name' => Array(
								'default' => 'System Designs',
							),
						);

	}//end constructor

}//end class
?>
