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
* $Id: page_asset_listing_edit_fns.inc,v 1.1 2004/01/20 03:41:45 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../page/page_edit_fns.inc';

/**
* Page_Asset_Listing_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Asset_Listing_Edit_Fns extends Page_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Page_Asset_Listing_Edit_Fns()
	{
		$this->Page_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = false;
		$this->static_screens['details']['lock_type'] = 'content';

	}//end constructor


	/**
	* Paint some information about replacing keywords
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintBodycopyInformation(&$asset, &$o, $prefix)
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'page_contents');
		if (empty($link)) return false;
		$info = Array($link['minorid'] => 'contents');
		$href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($info);

		echo 'The contents of the asset listing format can be '.
			'replaced into the <a href="'.$href[$link['minorid']].'">Page Contents Bodycopy</a> by using the %asset_listing% keyword';

	}//end paintBodycopyInformation()


	/**
	* Does nothing
	*
	* @return boolean
	* @access public
	*/
	function processBodycopyInformation()
	{
		return false;

	}//end processBodycopyInformation()


	/**
	* Paint a link to the default format
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDefaultFormatLink(&$asset, &$o, $prefix)
	{
		$folder = &$asset->getFolder('type_formats');
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'default_format');
		if (empty($link)) return false;

		$info = Array($link['minorid'] => 'contents');
		$href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($info);
		echo '<a href="'.$href[$link['minorid']].'">Edit the Default Format</a>';

		return true;

	}//end paintDefaultFormatLink()


	/**
	* Does nothing
	*
	* @return boolean
	* @access public
	*/
	function processDefaultFormatLink()
	{
		return false;

	}//end processDefaultFormatLink()


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

		if ($ok) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

	}//end processRootNode()


	/**
	* Paint the current asset types that have been added
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCurrentAssetTypes(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('content');
		$folder = &$asset->getFolder('type_formats');

		// do a check to see if any of the bodycopies have been removed manually
		$customised = $asset->attr('customised');
		if (!empty($customised)) {
			$deleted = false;
			foreach ($customised as $type) {
				$link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', true, $type);
				if (empty($link)) {
					$deleted = true;
					$key = array_search($type, $customised, true);
					unset($customised[$key]);
				}
			}
			if ($deleted) {
				$asset->setAttrValue('customised', $customised);
				$asset->saveAttributes();
			}
		}

		$types = $asset->attr('types');
		if (empty($types)) {
			echo 'There are no Types Defined';
			return false;
		}

		?>
		<table class="sq-backend-table">
		<tr>
			<td class="sq-backend-table-header">Asset Type</td>
			<td class="sq-backend-table-header">Edit Format</td>
			<?php
			if ($write_access) {
				?>
				<td class="sq-backend-table-header">Customise</td>
				<td class="sq-backend-table-header">Remove?</td>
				<?php
			}
			?>
		</tr>
			<?php
			foreach ($types as $type) {
				?>
				<tr>
					<td class="sq-backend-table-cell">
						<?php
						echo ucwords(str_replace('_',' ', $type));
						?>
					</td>
					<td class="sq-backend-table-cell">
						<?php
							$link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', true, $type);
							$is_customised = false;
							if (!empty($link)) {
								$is_customised = true;
								$info = Array($link['minorid'] => 'contents');
								$href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($info);

								$content = '<a href="'.$href[$link['minorid']].'">'.ucwords(str_replace('_',' ', $type)).' Format</a>';
							} else {
								$content = 'Default Format';
							}
							echo $content;
						?>
					</td>
					<?php
					if ($write_access) {
						?>
						<td class="sq-backend-table-cell">
							<?php
							check_box($prefix.'_customise[]', $type, $is_customised);
							?>
						</td>
						<td class="sq-backend-table-cell">
							<?php
							check_box($prefix.'_remove[]', $type, false);
							?>
						</td>
						<?php
					}
					?>
				</tr>
				<?php
			}
		?>
		</table>
		<?php

	}//end paintCurrentAssetTypes()


	/**
	* Process the current asset types that have been added
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processCurrentAssetTypes(&$asset, &$o, $prefix)
	{
		$customised = $asset->attr('customised');
		$types = $asset->attr('types');
		$folder = &$asset->getFolder('type_formats');

		// we are looking to see if any types have been removed
		if (isset($_POST[$prefix.'_remove'])) {
			foreach ($_POST[$prefix.'_remove'] as $type) {
				if (in_array($type, $types)) {
					$key = array_search($type, $types, true);
					unset($types[$key]);

					// remove from the customise list if its in here
					if (in_array($type, $customised)) {
						$key = array_search($type, $customised, true);
						unset($customised[$key]);

						// we also want to send the bodycopy to the trash
						$link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', true, $type);
						if (empty($link)) {
							trigger_error('Format for type '.$type.' not found', E_USER_WARNING);
						}
						// back up in your trash with the resurection
						$folder->deleteLink($link['linkid']);
					}
				}
			}// end foreach
			$asset->setAttrValue('types', $types);
		}

		if (isset($_POST[$prefix.'_customise'])) {

			if (empty($_POST[$prefix.'_customise'])) return false;

			foreach ($_POST[$prefix.'_customise'] as $type) {
				if (in_array($type, $customised)) continue;

				if (isset($_POST[$prefix.'_remove']) && in_array($type, $_POST[$prefix.'_remove'])) continue;

				$customised[] = $type;

				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
				$bodycopy = new Bodycopy();
				$copy_link = Array('asset' => &$folder, 'value' => $type, 'link_type' => SQ_LINK_TYPE_2, 'dependant' => 1, 'exclusive' => 1);

				$bodycopy->setAttrValue('name', ucwords(str_replace('_',' ', $type)).' Format');

				if (!$bodycopy->create($copy_link)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			}
		}

		if (!empty($customised)) {
			foreach ($customised as $type) {
				if (!isset($_POST[$prefix.'_customise']) || !in_array($type, $_POST[$prefix.'_customise'])) {

					if (isset($_POST[$prefix.'_remove']) && in_array($type, $_POST[$prefix.'_remove'])) continue;

					$link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', false, $type);
					$asset->deleteLink($link['linkid']);
					$key = array_search($type, $customised, true);
					unset($customised[$key]);
				}
			}
		}

		$asset->setAttrValue('customised', $customised);
		return true;

	}//end processCurrentAssetTypes()


	/**
	* Paint section to add an asset type
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddAssetType(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('content')) return false;

		$types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();
		$types = array_diff($types, $asset->attr('types'));
		asort($types);

		$uc_types = Array('' => '');
		foreach ($types as $type) $uc_types[$type] = ucwords(str_replace('_',' ', $type));
		combo_box($prefix.'_new_type', $uc_types, false, '');

	}//end paintAddAssetType()


	/**
	* Process the section to add a new format for an asset
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddAssetType(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_new_type'])) {
			$type = trim($_POST[$prefix.'_new_type']);
			if (empty($type)) return false;
			$types = $asset->attr('types');
			if (!in_array($type, $types)) {
				$types[] = $type;
				$asset->setAttrValue('types', $types);
				return true;
			}
		}
		return false;

	}//end paintAddFormat()


}//end class

?>
