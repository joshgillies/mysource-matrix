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
* $Id: file_bridge_edit_fns.inc,v 1.3 2012/08/30 00:58:36 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';

/**
* File_Bridge_Edit_Fns
*
* Purpose
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage file
*/
class File_Bridge_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function File_Bridge_Edit_Fns()
	{
		parent::__construct();

	}//end constructor


	/**
	* Prints the interface for editing filename
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintFilename(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			echo $asset->attr('name');
		} else {
			echo $asset->attr('name');
		}
		return $wa;

	}//end paintFilename()


	/**
	* Processes the interface for editing filename
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processFilename(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');

		return $wa;

	}//end processFilename()


        
	 /**
	* Prints the interface for editing filename
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintPath(&$asset, &$o, $prefix)
	{
                                                require_once (SQ_DATA_PATH.'/private/conf/file_bridge.inc');
                                                $o->openField('Path');
                                                echo FILE_BRIDGE_PATH;
                                                $o->note("Configure path in ".SQ_DATA_PATH.'/private/conf/file_bridge.inc. Path is relative to Matrix installation location unless prefixed with \'/\'.');
                                                $o->closeField();

	}//end paintFilename()
        
}//end class

?>
