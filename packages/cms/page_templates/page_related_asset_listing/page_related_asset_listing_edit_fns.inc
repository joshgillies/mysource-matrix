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
* $Id: page_related_asset_listing_edit_fns.inc,v 1.10 2012/08/30 00:58:09 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_listing/page_asset_listing_edit_fns.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Page_Asset_Listing_Edit_Fns
*
* Purpose
*
*
*
* @author  Elden McDonald <emcdonald@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Related_Asset_Listing_Edit_Fns extends Page_Asset_Listing_Edit_Fns
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
	* Paints the relator selection box
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintRelator(&$asset, &$o, $prefix)
	{
		// get relator links
		// we need them indexed numerically for getAssetInfo.
		$relator_asset_ids = $asset->getStaticRelators();
		sort($relator_asset_ids);

		if ($asset->writeAccess('links')) {
			multiple_asset_finder($prefix.'_relator_nodes', $relator_asset_ids);
		} else {
			$infos = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($relator_asset_ids);
			?><ul style="margin: 1px 15px;"><?php
			foreach ($infos as $assetid => $info) {
				echo '<li>'.get_asset_tag_line($assetid).'</li>';
			}
			?></ul><?php
		}
		return TRUE;

	}//end paintRelator()


	/**
	* Processes the value input from relator node selection box
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function processRelator(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_relator_nodes'])) {
			return FALSE;
		}
		$new_relator_nodes = $_POST[$prefix.'_relator_nodes'];
		foreach ($new_relator_nodes as $new_relator_node) {
			$new_relator_node_ids[] = $new_relator_node['assetid'];
		}

		// get existing relator links
		$relator_asset_ids = $asset->getStaticRelators();

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// lets delete the relator nodes that have been removed
		foreach ($relator_asset_ids as $relator_asset_id) {
			if (!in_array($relator_asset_id, $new_relator_node_ids)) {
				$link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($asset->id, $relator_asset_id, SQ_LINK_NOTICE, 'relator', $side_of_link='major');
				if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($link['linkid'])) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
			}
		}

		// now, create the new node links
		foreach ($new_relator_nodes as $index => $node) {
			// first test to see if the node already exists -- if so, ignore it
			if (isset($relator_asset_ids[$node['assetid']])) {
				continue;
			}
			// otherwise, create the appropriate notice link
			if (!empty($node['assetid'])) {
				$new_relator = $GLOBALS['SQ_SYSTEM']->am->getAsset($node['assetid']);
				if (!$asset->createLink($new_relator, SQ_LINK_NOTICE, 'relator')) {
					trigger_localised_error('CMS0016', sprintf(translate('Failed adding new root node "%1$s" [#%2$s]'), $new_relator->name, $new_relator->id), E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;

				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end processRelator()


	/**
	* Paints the thesaurus filter selection box
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintThesaurusFilters(&$asset, &$o, $prefix)
	{
		return $this->paintNoticeLinkedAsset($asset, 'thesaurus_filter', Array('thesaurus' => 'D'), $prefix.'thesaurus_filters', TRUE);

	}//end paintThesaurusFilter()


	/**
	* Processes the value input from thesaurus filter selection box
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function processThesaurusFilters(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('links')) {
			$this->processNoticeLinkedAsset($asset, 'thesaurus_filter', $prefix.'thesaurus_filters', TRUE);
		}
		return FALSE; // no need to save attrs, it's only links

	}//end processThesaurusFilter()

}//end class

?>
