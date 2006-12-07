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
* $Id: content_type_nest_content_edit_fns.inc,v 1.43 2006/12/07 00:04:16 emcdonald Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/content_type/content_type_edit_fns.inc';

/**
* Content_Type_Nest_Content_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.43 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Nest_Content_Edit_Fns extends Content_Type_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Content_Type_Nest_Content_Edit_Fns()
	{
		$this->Content_Type_Edit_Fns();

	}//end constructor


	/**
	* Prints the backend interface for this WYSIWYG editor
	*
	* @param object	&$asset	the nest content content type object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(&$asset, $prefix)
	{
		if ($asset->writeAccess('links') && $asset->writeAccess('attributes')) {
			$nested_asset =& $this->_getNestedAsset($asset);
			$asset_link = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($nested_asset->id);
			if (empty($asset_link)) {
				$asset_link = strip_url(current_url(FALSE, TRUE)).'/?a='.$nested_asset->id;
			}

			if (!is_null($nested_asset)) {
				?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-cell-alt"><b><?php echo translate('cms_ct_nest_content_currently_nesting', $nested_asset->attr('name')) ?></b></td>
					</tr>
					<tr>
						<td class="sq-backend-table-cell"><?php asset_finder($prefix.'_nested_assetid', $nested_asset->id, Array('page' => 'D')); ?></td>
					</tr>
					<tr>
						<td class="sq-backend-table-cell"><b><a href="<?php echo $asset_link; ?>" id="sq_preview_new_window" target="_blank"><?php echo translate('cms_ct_nest_content_click_to_preview') ?></a></b></td>
					</tr>
				</table>
				<?php
			} else {
				?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-cell-alt"><b><?php echo translate('cms_ct_nest_content_no_page_selected'); ?></b></td>
					</tr>
					<tr>
						<td class="sq-backend-table-cell"><?php asset_finder($prefix.'_nested_assetid', '', Array('page' => 'D')); ?></td>
					</tr>
				</table>
				<?php
			}
		}//end if
		return TRUE;

	}//end paintBackend()


	/**
	* Prints a link to the asset if it is being viewed from an edit interface, otherwise
	* just calls printBody on the nested asset and lets it take care of things
	*
	* @param object		&$asset		the nest content content type object
	* @param boolean	$editing	are we printing an editing interface
	* @param boolean	$generating	are we genenerating the cache file
	*
	* @return boolean
	* @access public
	*/
	function paint(&$asset, $editing=FALSE, $generating=FALSE)
	{
		// get the asset - we can't pass the type in because it could be anything
		// we also mute errors because we check later on
		$nested_asset =& $this->_getNestedAsset($asset);
		if (is_null($nested_asset)) return FALSE;

		if ($generating) {
			$this->paintGenerated($asset, $nested_asset);
			return TRUE;
		}

		if ($editing) {
			// We are in the edit interface, so just print a link
			$asset_link = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($nested_asset->id);
			if (empty($asset_link)) {
				$asset_link = strip_url(current_url(FALSE, TRUE)).'/?a='.$nested_asset->id;
			}
			$asset_contents = '<strong>'.translate('cms_ct_nest_content_nested_content', $nested_asset->name, $nested_asset->id).'</strong> <a href="'.$asset_link.'" id="sq_preview_new_window" target="_blank">'.translate('cms_ct_nest_content_click_to_preview').'</a>.';
		} else {
			// This is the real deal, so we want to actually print the asset

			// Check for recursion
			// We use the cache key where available to distinguish differently-behaving
			// copies of the same asset (Bug Fix #1601)
			if (method_exists($nested_asset, '_getCacheKey')) {
				$asset_cache_key = $nested_asset->_getCacheKey();
			} else {
				$asset_cache_key = (string) $nested_asset->id;
			}
			if (isset($GLOBALS['SQ_NEST_CONTENT_PAINTED'][$asset_cache_key])) {
				unset($GLOBALS['SQ_NEST_CONTENT_PAINTED'][$asset_cache_key]);
				echo '<span class="sq-backend-warning"><b>[ ** '.translate('cms_ct_nest_content_recursion_warning').' ** ]<br/>'.translate('cms_ct_nest_content_recursion_body').'</b></span>';
				return TRUE;
			}

			// Record that we have printed this asset
			// OKAY! so we havn't actually printed it yet, but we need to record that
			// we have or we might just fall endlessly into printBody functions and
			// never get back here again
			$GLOBALS['SQ_NEST_CONTENT_PAINTED'][$asset_cache_key] = 1;

			// Ask the asset to do its thing
			ob_start();
				$nested_asset->printBody();
				$asset_contents = ob_get_contents();
			ob_end_clean();

			unset($GLOBALS['SQ_NEST_CONTENT_PAINTED'][$asset_cache_key]);
		}

		echo $asset_contents;

		return TRUE;

	}//end paint()


	/**
	* Paints the content type as it should be cached
	*
	* @param object	&$asset			the table whose interface we are painting
	* @param object	&$nested_asset	the asset to nest
	*
	* @return void
	* @access public
	*/
	function paintGenerated(&$asset, &$nested_asset)
	{
		echo '<'.'?php'."\n";
		?>

		$nested_assetid = '<?php echo $nested_asset->id; ?>';
		$nested_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($nested_assetid);
		if (method_exists($nested_asset, '_getCacheKey')) {
			$asset_cache_key = $nested_asset->_getCacheKey();
		} else {
			$asset_cache_key = (string) $nested_asset->id;
		}

		if (!isset($GLOBALS['SQ_NEST_CONTENT_PAINTED'])) $GLOBALS['SQ_NEST_CONTENT_PAINTED'] = Array();
		if (!isset($GLOBALS['SQ_NEST_CONTENT_PAINTED'][$asset_cache_key])) {
			$GLOBALS['SQ_NEST_CONTENT_PAINTED'][$asset_cache_key] = 1;
			$nested_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($nested_assetid, '', TRUE);
			if ($nested_asset->id && $nested_asset->readAccess()) $nested_asset->printBody();
			unset($GLOBALS['SQ_NEST_CONTENT_PAINTED'][$asset_cache_key]);
		} else {
			echo '<font color="red"><b>[ ** '.translate('cms_ct_nest_content_recursion_warning').' ** ]<br />'.translate('cms_ct_nest_content_recursion_body').'</b></font>';
		}

		<?php
		echo '?'.'>'."\n";

	}//end paintGenerated()


	/**
	* Processes the backend interface for this WYSIWYG editor
	*
	* @param array	$link	information used to create the initial link
	* @param object	&$asset	the nest content content type object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processBackend($link, &$asset, $prefix)
	{
		$nested_asset =& $this->_getNestedAsset($asset);
		$current_assetid = (!is_null($nested_asset)) ? $nested_asset->id : 0;
		$assetid = (isset($_POST[$prefix.'_nested_assetid']['assetid'])) ? (int)$_POST[$prefix.'_nested_assetid']['assetid'] : 0;

		if ($assetid) {

			if ($assetid != $current_assetid) {

				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

				if ($current_assetid) {
					$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', TRUE, 'nested_asset');
					if (!$asset->deleteLink($link['linkid'])) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						return FALSE;
					}
				}

				// create a 'hidden' link for delete notification
				$nested_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (!$asset->createLink($nested_asset, SQ_LINK_NOTICE, 'nested_asset')) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}

				// get the html contents of the nested asset so
				// we change the type of the cell later - other content
				// types can use the html

				// if we dont have the frontend asset set just buffer nothing
				if (!isset($GLOBALS['SQ_SYSTEM']->frontend_asset)) {
					$html = '';
				} else {
					ob_start();
						$this->paint($asset, $prefix);
						$html = ob_get_contents();
					ob_end_clean();
				}

				$asset->setAttrValue('html', $html);

				// Bug Fix 1541
				if (!$current_assetid && $assetid) {
					// Normally you are not supposed to call linksUpdated() directly, however,
					// createAssetLink() does not call linksUpdated() to the major unless its dependant link.
					// let's manually update the paint layout bodycopy if there is any
					$asset->linksUpdated();
				}

				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return TRUE;

			}//end if

		} else if ($current_assetid) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', TRUE, 'nested_asset');
			return $asset->deleteLink($link['linkid']);
		}
		return FALSE;

	}//end processBackend()


	/**
	* Get a reference to the asset the content type is nesting
	*
	* @param object	&$asset	the nest content content type object
	*
	* @return mixed object|NULL
	* @access private
	*/
	function &_getNestedAsset(&$asset)
	{
		$nested_asset = NULL;

		$nested_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', TRUE, 'nested_asset');
		if (!empty($nested_link)) {
			$nested_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($nested_link['minorid'], $nested_link['minor_type_code'], TRUE);
		}

		return $nested_asset;

	}//end _getNestedAsset()


}//end class

?>
