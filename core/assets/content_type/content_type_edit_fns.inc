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
* $Id: content_type_edit_fns.inc,v 1.15 2007/12/03 23:12:50 lwright Exp $
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
* @version $Revision: 1.15 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Content_Type_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

		unset($this->static_screens['permissions']);
		unset($this->static_screens['workflow']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['metadata']);
		unset($this->static_screens['linking']);
		unset($this->static_screens['settings']);
		unset($this->static_screens['history']);
		unset($this->static_screens['tagging']);

	}//end constructor


	/**
	* Paints raw HTML to be printed on the frontend
	*
	* @param Content_Type	$asset		the content type we are painting
	* @param boolean		$editing	are we printing an editing interface
	* @param boolean		$generating	are we genenerating the cache file
	*
	* @return void
	* @access public
	*/
	function paint(Content_Type $asset, $editing=FALSE, $generating=FALSE)
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
	* @param Content_Type	$asset	the content type object
	* @param string			$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintBackendIcons(Content_Type $asset, $prefix)
	{

	}//end paintBackendIcons()


	/**
	* Prints extra backend icons at the right hand side of the menu
	*
	* @param Content_Type	$asset	the WYSIWYG Content Type object
	* @param string			$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintBackendStatusIcons(Content_Type $asset, $prefix)
	{

	}//end paintBackendStatusIcons()


}//end class

?>
