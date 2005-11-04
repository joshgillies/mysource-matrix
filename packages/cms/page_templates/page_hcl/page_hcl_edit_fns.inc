<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: page_hcl_edit_fns.inc,v 1.12 2005/11/04 00:00:40 lwright Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';


/**
* Page_HCL_Edit_Fns
*
* Purpose
*
* @author  Dmitri Iarandine <diarandine@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_HCL_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*/
	function Page_HCL_Edit_Fns()
	{
		$this->Page_Edit_Fns();
		$this->static_screens['details']['lock_type'] = 'content';

	}//end constructor


	/**
	* Paints the root node selection box
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintRootNode(&$asset, &$o, $prefix)
	{
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', false, 'root');
		$write_access = $asset->writeAccess('links');

		if (!$write_access) {
			$root_asset = null;
			if (!empty($root_link)) {
				echo get_asset_tag_line($root_link['minorid']);
			} else {
				echo translate('cms_no_root_node');
			}
		} else {
			asset_finder($prefix.'_rootid', (!empty($root_link)) ? $root_link['minorid'] : '');
		}

		return $write_access;

	}//end paintRootNode()


	/**
	* Processes the value input from root node selection box
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function processRootNode(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_rootid']['assetid'])) return false;
		$new_root  = $_POST[$prefix.'_rootid']['assetid'];

		// getting existing root link
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', false, 'root');

		// dont process anything if the root node has not changed
		if (!empty($root_link) && $root_link['minorid'] == $new_root) return false;

		$ok = true;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// deleting old root link if any found...
		if (!empty($root_link) && !$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($root_link['linkid'])) {
			$ok = false;
		} else if ($new_root) {
			$root = &$GLOBALS['SQ_SYSTEM']->am->getAsset($new_root);
			if (!is_null($root)) {
				if (!$asset->createLink($root, SQ_LINK_NOTICE, 'root')) {
					$ok = false;
					trigger_localised_error('CMS0016', E_USER_WARNING, $root->name, $root->id);
				}
			}
		}

		// checking our flag and committing or rolling back the whole transaction
		if ($ok) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

	}//end processRootNode()


	/**
	* Paints the list of stored metadata schemas for HCL template
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintHCLSchemas(&$asset, &$o, $prefix)
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$write_access = $asset->writeAccess('links');

		$schemas = Array();
		$schemas = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, 'metadata_schema', false, 'major', 'schema');

		if (!empty($schemas)) {
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header"><?php echo translate('schema_name') ?></td>
					<?php
						if ($write_access) {
							?><td class="sq-backend-table-header"><?php echo translate('delete_question'); ?></td><?php
						}
					?>
				</tr>
			<?php

			foreach ($schemas as $schema_link) {

				$schema_asset = &$am->getAsset($schema_link['minorid']);
				if (is_null($schema_asset)) continue;

				?>
				<tr>
					<td class="sq-backend-table-cell"><?php echo get_asset_tag_line($schema_asset->id); ?></td>
					<?php
					if ($write_access) {
						?><td class="sq-backend-table-cell"><?php check_box($prefix.'_schemas[]', $schema_link['linkid']); ?></td><?php
					}
					?>
				</tr>
				<?php

			} // end foreach schemas

			?></table><?php

		} else {
			echo 'There are currently no Metadata schemas in this list';
		}

		return $write_access;

	}//end paintHCLSchemas()


	/**
	* Process adding of the new schema to list, or schema deletion
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function processHCLSchemas(&$asset, &$o, $prefix)
	{
		// now checking if there were any checkboxes marked->delete those schemas from list
		if (isset($_POST[$prefix.'_schemas'])) {
			foreach ($_POST[$prefix.'_schemas'] as $linkid) {
				$linkid = (int)$linkid;
				if (!$linkid) continue;
				if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($linkid)) {
					$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);
					$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
					trigger_localised_error('CMS0017', E_USER_WARNING, $schema->name, $schema->id);
				}
			}
		}

	}//end processHCLSchemas()


	/**
	* Paint the interface for adding a metadata schema
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return boolean
	* @access private
	*/
	function paintAddSchema(&$asset, &$o, $prefix)
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am; // instantiating Asset Manager
		if (!$asset->writeAccess('links')) return false; // if details page is not locked, do not proceed

		asset_finder($prefix.'_newschema', 0, Array('metadata_schema' => 'I'));

		return true;

	}//end paintAddSchema()


	/**
	* Process the interface for adding a metadata schema
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function processAddSchema(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_newschema']['assetid'])) return false;
		$new_schema  = $_POST[$prefix.'_newschema']['assetid']; // submitted root id

		if ($new_schema) {
			$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($new_schema);
			if (!is_null($schema)) {
				if (!$asset->createLink($schema, SQ_LINK_NOTICE, 'schema')) {
					trigger_localised_error('CMS0018', E_USER_WARNING, $schema->name, $schema->id);
				}
			}
		}

	}//end processAddSchema()


}//end class

?>