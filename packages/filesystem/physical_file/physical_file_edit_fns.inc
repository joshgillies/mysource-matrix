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
* $Id: physical_file_edit_fns.inc,v 1.2 2012/08/30 00:58:36 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Physical_File_Edit_Fns
*
* Purpose
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage file
*/
class Physical_File_Edit_Fns extends Asset_Edit_Fns
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


	/**
	* Paint the interface for viewing the file
	*
	* @param object				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintView($asset, Backend_Outputter $o, $prefix)
	{
		$type = $asset->getMimeType();

		if (strpos($type, 'text') === 0) {
			$asset->printBody();
		} else {
			echo '<a href="?a='.$asset->id.'" target="_BLANK">'.$asset->name.'</a>';
		}//end if

		return TRUE;

	}//end paintView()


	/**
	* Processes the interface for viewing the file
	*
	* @param object				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processView($asset, Backend_Outputter $o, $prefix)
	{
		return FALSE;

	}//end processView()

}//end class

?>
