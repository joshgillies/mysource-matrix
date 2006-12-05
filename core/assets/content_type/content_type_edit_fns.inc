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
* $Id: content_type_edit_fns.inc,v 1.13 2006/12/05 05:48:39 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Content_Type_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Content_Type_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Content_Type_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

		unset($this->static_screens['permissions']);
		unset($this->static_screens['workflow']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['metadata']);
		unset($this->static_screens['linking']);
		unset($this->static_screens['settings']);
		unset($this->static_screens['history']);

	}//end constructor


	/**
	* Paints raw HTML to be printed on the frontend
	*
	* @param object		&$asset		the content type we are painting
	* @param boolean	$editing	are we printing an editing interface
	* @param boolean	$generating	are we genenerating the cache file
	*
	* @return void
	* @access public
	*/
	function paint(&$asset, $editing=FALSE, $generating=FALSE)
	{
		$output =  $asset->attr('html');
		if ($generating) {
			// escape dangerous content
			require_once SQ_FUDGE_PATH.'/general/text.inc';
			escape_php($output);
		}
		echo $output;

	}//end paint()


	/**
	* Prints any icons the content type needs next to the cell properties
	*
	* @param object	&$asset	the content type object
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintBackendIcons(&$asset, $prefix)
	{

	}//end paintBackendIcons()


	/**
	* Prints extra backend icons at the right hand side of the menu
	*
	* @param object	&$asset	the WYSIWYG Content Type object
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintBackendStatusIcons(&$asset, $prefix)
	{

	}//end paintBackendStatusIcons()


}//end class

?>
