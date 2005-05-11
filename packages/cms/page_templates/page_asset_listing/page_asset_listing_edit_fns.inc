<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: page_asset_listing_edit_fns.inc,v 1.24 2005/05/11 06:07:32 lwright Exp $
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
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.24 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Asset_Listing_Edit_Fns extends Listing_Engine_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Page_Asset_Listing_Edit_Fns()
	{
		$this->Listing_Engine_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = false;
		$this->static_screens['details']['lock_type'] = 'content';

	}//end constructor


	/**
	* Paints the root node selection box
	*
	* @param object	Asset_Listing		&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintRootNode(&$asset, &$o, $prefix)
	{
		// get root links
		$root_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', false, 'major', 'root');
		$root_asset_ids = Array();
		foreach ($root_links as $root_link) {
			$root_asset_ids[$root_link['minorid']] = $root_link['minor_type_code'];
		}

		if ($asset->writeAccess('links')) {
			multiple_asset_finder($prefix.'_root_nodes', array_keys($root_asset_ids));
		} else {
			$infos = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($root_asset_ids));
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header"><?php echo translate('id') ?></td>
					<td class="sq-backend-table-header"><?php echo translate('name') ?></td>
					<td class="sq-backend-table-header"><?php echo translate('type') ?></td>
				</tr>
			<?php
			foreach ($infos as $assetid => $info) {
			?>

			<tr>
				<td class="sq-backend-table-cell"><?php echo $assetid ?></td>
				<td class="sq-backend-table-cell"><?php echo $info['name'] ?></td>
				<td class="sq-backend-table-cell"><?php echo ucwords(str_replace('_',' ', $info['type_code'])) ?></td>
			</tr>
			<?php
			}
			?>
			</table>
			<?php
		}
		return true;

	}//end paintRootNode()


	/**
	* Processes the value input from root node selection box
	*
	* @param object	Asset_Listing		&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function processRootNode(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_root_nodes'])) return false;
		$new_root_nodes = $_POST[$prefix.'_root_nodes'];
		foreach ($new_root_nodes as $new_root_node) {
			$new_root_node_ids[] = $new_root_node['assetid'];
		}

		// get existing root links
		$root_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', false, 'major', 'root');

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$ok = true;

		// lets delete the root nodes that have been removed
		foreach ($root_links as $root_link) {
			if (!in_array($root_link['minorid'], $new_root_node_ids)) {
				if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($root_link['linkid'])) {
					$ok = false;
				}
			}
		}

		// now, create the new node links
		foreach ($new_root_nodes as $index => $node) {
			// first test to see if the node already exists -- if so, ignore it
			foreach ($root_links as $root_link) {
				if ($root_link['minorid'] == $node['assetid']) continue 2;
			}
			// otherwise, create the appropriate notice link
			if ($node['assetid'] != 0) {
				$new_root = &$GLOBALS['SQ_SYSTEM']->am->getAsset($node['assetid']);
				if (!$asset->createLink($new_root, SQ_LINK_NOTICE, 'root')) {
					$ok = false;
					trigger_localised_error('CMS0016', E_USER_WARNING, $new_root->name, $new_root->id);
					break;
				}
			}
		}

		if ($ok) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

	}//end processRootNode()


}//end class

?>
