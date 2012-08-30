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
* $Id: page_hit_count_listing_edit_fns.inc,v 1.3 2012/08/30 00:58:16 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/listing_engine/listing_engine_edit_fns.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Page_Asset_Listing_Edit_Fns
*
* Purpose
*
*
* @author  Basil Shkara <bshkara@squiz.net>
* @author  Christophe Olivar <colivar@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Hit_Count_Listing_Edit_Fns extends Listing_Engine_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Paints the root node selection box
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintRootNode(&$asset, &$o, $prefix)
	{
		$o->openField('Root Nodes');

		parent::paintRootNode($asset, $o, $prefix);

		$o->note('Leave blank to list the sub assets/children of this asset listing page');
		$o->closeField();

		return TRUE;

	}//end paintRootNode()


	/**
	* Get the possible 'sort by' options
	*
	* @param object	&$asset	asset being painted
	*
	* @return array
	* @access protected
	*/
	function _getStandardSortByOptions(&$asset)
	{
		$standard_option = parent::_getStandardSortByOptions($asset);
		$standard_option += Array('hitcount' => 'Hit count of the asset');
		return $standard_option;

	}//end _getStandardSortByOptions()


}//end class

?>
