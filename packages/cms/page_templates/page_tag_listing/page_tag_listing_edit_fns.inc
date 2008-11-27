<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: page_tag_listing_edit_fns.inc,v 1.1 2008/11/27 14:35:04 gnoel Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_listing/page_asset_listing_edit_fns.inc';

/**
* Page_Tag_Listing_Edit_Fns
*
* Purpose
*
*
* @author  Geoffroy Noel <gnoel@squiz.co.uk>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Tag_Listing_Edit_Fns extends Page_Asset_Listing_Edit_Fns
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
	* Paint the backend editing interface for this condition
	*
	* @return boolean
	* @access public
	*/
	function paintThesaurusLinks($asset, $o, $prefix)
	{
		return $this->paintNoticeLinkedAsset($asset, 'thesaurus', Array('thesaurus' => 'D'), 'thesaurus');

	}//end processVerdictStyle()


	/**
	* Process the backend editing interface for this condition
	*
	* @return boolean
	* @access public
	*/
	function processThesaurusLinks($asset, $o, $prefix)
	{
		return $this->processNoticeLinkedAsset($asset, 'thesaurus', 'thesaurus');

	}//end processVerdictStyle()


	/**
	* Paint the interface for choosing which asset types to list
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAssetTypes(&$asset, &$o, $prefix)
	{
		$types = $asset->attr('types');
		if ($asset->writeAccess('content')) {
			// print the form fields
			$display_values = Array('type_code' => Array(), 'inherit' => Array());
			foreach ($types as $type => $inherit) {
				$display_values['type_code'][] = $type;
				$display_values['inherit'][] = $inherit;
			}
			asset_type_chooser($prefix.'_types', TRUE, $display_values, NULL, TRUE, TRUE, Array('thesaurus_term'));
			return TRUE;
		} else {
			return parent::paintAssetTypes($asset, $o, $prefix);
		}//end else - if write access to content

	}//end paintAssetTypes()


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
		return $this->paintNoticeLinkedAsset($asset, 'root');

	}//end paintRootNode()


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
	function processRootNode(&$asset, &$o, $prefix)
	{
		return $this->processNoticeLinkedAsset($asset, 'root');

	}//end paintRootNode()


}//end class
?>