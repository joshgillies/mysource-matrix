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
* $Id: page_hcl_edit_fns.inc,v 1.4.2.2 2004/03/02 18:46:07 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once dirname(__FILE__).'/../../page/page_edit_fns.inc';


/**
* Page_HCL_Edit_Fns
*
* Purpose
*
* @author  Dmitri Iarandine <diarandine@squiz.net>
* @version $Version$ - 1.0
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
			$root_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($root_link['minorid']);
			if (!is_null($root_asset)) {
				echo $root_asset->name . ' (Id: #' . $root_asset->id . ')';
			} else {
				echo 'No root node has been selected';
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
					trigger_error('Failed adding new root node "'.$root->name.'" [#'.$root->id.']', E_USER_WARNING);
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
					<td class="sq-backend-table-header">Schema Name</td>
					<?php
						if ($write_access) {
							?><td class="sq-backend-table-header">Delete ?</td><?php
						}
					?>
				</tr>
			<?php

			foreach ($schemas as $schema_link) {

				$schema_asset = &$am->getAsset($schema_link['minorid']);
				if (is_null($schema_asset)) continue;

				?>
				<tr>
					<td class="sq-backend-table-cell"><?php echo $schema_asset->name; ?></td>
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
				if (!$linkid) continue;
				if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($linkid)) {
					$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);
					$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
					trigger_error('Failed removing metadata schema "'.$schema->name.'" [#'.$schema->id.']', E_USER_WARNING);
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
					trigger_error('Failed adding metadata schema "'.$schema->name.'" [#'.$schema->id.']', E_USER_WARNING);
				}
			}
		}

	}//end processAddSchema()


}//end class

?>