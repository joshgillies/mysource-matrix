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
* $Id: content_type_nest_content_edit_fns.inc,v 1.51 2007/12/10 06:32:47 rong Exp $
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
* @version $Revision: 1.51 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Nest_Content_Edit_Fns extends Content_Type_Edit_Fns
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
			$nested_asset = $this->_getNestedAsset($asset);
			$paint_with = $asset->attr('paint_with');
			$arbitrary_paint_layout = $this->_getArbitraryPaintLayout($asset);
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
						<td class="sq-backend-table-cell"><?php asset_finder($prefix.'_nested_assetid', $nested_asset->id, Array('asset' => 'D')); ?></td>
					</tr>
					<tr>
						<td class="sq-backend-table-cell"><b><a href="<?php echo $asset_link; ?>" id="sq_preview_new_window" target="_blank"><?php echo translate('cms_ct_nest_content_click_to_preview') ?></a></b></td>
					</tr>
					<tr>
						<td class="sq-backend-table-cell-alt"><b>Paint this asset:</b></td>
					</tr>
					<tr>
						<td class="sq-backend-table-cell">
						<input name="<?php echo $prefix; ?>_paint_with" type="radio" value="raw"<?php if ($paint_with == 'raw') { ?> checked="checked"<?php } ?>> Raw (no Paint Layout)<br/>
						<input name="<?php echo $prefix; ?>_paint_with" type="radio" value="default"<?php if ($paint_with == 'default') { ?> checked="checked"<?php } ?>> Using default Paint Layout (based on lookup rules)<br/>
						<input name="<?php echo $prefix; ?>_paint_with" type="radio" value="arbitrary"<?php if ($paint_with == 'arbitrary') { ?> checked="checked"<?php } ?>> Using an arbitrary Paint Layout: <?php asset_finder($prefix.'_paint_with_layout', $arbitrary_paint_layout->id, Array('paint_layout_page' => 'D')); ?><br/>
						</td>
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
						<td class="sq-backend-table-cell"><?php asset_finder($prefix.'_nested_assetid', '', Array('asset' => 'D')); ?></td>
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
		$nested_asset = $this->_getNestedAsset($asset);
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

			$asset_cache_key = $nested_asset->id;
			if (method_exists($nested_asset, '_getCacheKey')) {
				$asset_cache_key .= $nested_asset->_getCacheKey();
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
				switch ($this->attr('paint_with')) {
					case 'raw':
						// Raw - just paint it
						$nested_asset->printBody();
					break;

					case 'default':
						// Default Rules - use the "normal" method of determining
						// which paint layout gets used
						$nested_asset->printBodyWithPaintLayout();
					break;

					case 'arbitrary':
						// Get the arbitary paint layout; if there is none set,
						// paint it raw, otherwise paint it with the arbitrary layout
						$arbitrary_paint_layout = $this->_getArbitraryPaintLayout($asset);
						if (empty($arbitrary_paint_layout)) {

							$nested_asset->printBody();
						} else {
							// we have found the paint layout to use... we might need to fool the system
							// into thinking we are printing the frontend of that asset
							$switched_frontend = FALSE;
							if ($GLOBALS['SQ_SYSTEM']->frontend_asset->id !== $this->id) {
								$current_frontend_asset =& $GLOBALS['SQ_SYSTEM']->frontend_asset;
								$GLOBALS['SQ_SYSTEM']->frontend_asset =& $nested_asset;
								$switched_frontend = TRUE;
							}
							$arbitrary_paint_layout->paint($nested_asset);

							if ($switched_frontend) {
								$GLOBALS['SQ_SYSTEM']->frontend_asset =& $current_frontend_asset;
							}
						}
					break;
				}//end switch

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
		$nested_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($nested_assetid);
		$asset_cache_key = $nested_assetid;
		if (method_exists($nested_asset, '_getCacheKey')) {
			$asset_cache_key .= $nested_asset->_getCacheKey();
		}

		if (!isset($GLOBALS['SQ_NEST_CONTENT_PAINTED'])) $GLOBALS['SQ_NEST_CONTENT_PAINTED'] = Array();
		if (!isset($GLOBALS['SQ_NEST_CONTENT_PAINTED'][$asset_cache_key])) {
			$GLOBALS['SQ_NEST_CONTENT_PAINTED'][$asset_cache_key] = 1;
			$nested_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($nested_assetid, '', TRUE);
			if ($nested_asset->id && $nested_asset->readAccess()) {
				<?php
				switch ($asset->attr('paint_with')) {
					case 'raw':
						?>$nested_asset->printBody();<?php
					break;

					case 'default':
						?>$nested_asset->printBodyWithPaintLayout();<?php
					break;

					case 'arbitrary':
						$arbitrary_paint_layout = $this->_getArbitraryPaintLayout($asset);
						if (empty($arbitrary_paint_layout)) {
							?>$nested_asset->printBody();<?php
						} else {
							?>
							$arbitrary_paint_layout = $GLOBALS['SQ_SYSTEM']->am->getAsset(<?php echo $arbitrary_paint_layout->id ?>);
							$switched_frontend = FALSE;
							if ($GLOBALS['SQ_SYSTEM']->frontend_asset->id !== $this->id) {
								$current_frontend_asset =& $GLOBALS['SQ_SYSTEM']->frontend_asset;
								$GLOBALS['SQ_SYSTEM']->frontend_asset =& $nested_asset;
								$switched_frontend = TRUE;
							}
							$arbitrary_paint_layout->paint($nested_asset);

							if ($switched_frontend) {
								$GLOBALS['SQ_SYSTEM']->frontend_asset =& $current_frontend_asset;
							}
							<?php
						}
					break;
				}//end switch
				?>
			}
			unset($GLOBALS['SQ_NEST_CONTENT_PAINTED'][$asset_cache_key]);
		} else {
			echo '<font color="red"><b>[ ** '.translate('cms_ct_nest_content_recursion_warning').' ** ]<br />'.translate('cms_ct_nest_content_recursion_body').'</b></font>';
		}

		<?php echo "\n".'?'.'>';

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
		$ok = FALSE;

		$nested_asset = $this->_getNestedAsset($asset);
		$current_assetid = (!is_null($nested_asset)) ? $nested_asset->id : 0;

		$arbitrary_paint_layout = $this->_getArbitraryPaintLayout($asset);
		$current_apl_assetid = (!is_null($arbitrary_paint_layout)) ? $arbitrary_paint_layout->id : 0;

		$assetid = (isset($_POST[$prefix.'_nested_assetid']['assetid'])) ? $_POST[$prefix.'_nested_assetid']['assetid'] : 0;
		$paint_with = array_get_index($_POST, $prefix.'_paint_with', 'raw');
		$paint_with_assetid = (isset($_POST[$prefix.'_paint_with_layout']['assetid'])) ? $_POST[$prefix.'_paint_with_layout']['assetid'] : 0;

		if ($paint_with_assetid && ($paint_with == 'arbitrary')) {
			if ($current_apl_assetid) {
				$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', TRUE, 'paint_with_layout');
				if (!$asset->deleteLink($link['linkid'])) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return FALSE;
				}
			}

			// create a NOTICE link for delete notification
			$arbitrary_paint_layout = $GLOBALS['SQ_SYSTEM']->am->getAsset($paint_with_assetid);
			if (!$asset->createLink($arbitrary_paint_layout, SQ_LINK_NOTICE, 'paint_with_layout')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return FALSE;
			}

			$asset->linksUpdated();

		}

		if ($paint_with != $asset->attr('paint_with')) {
			$asset->setAttrValue('paint_with', $paint_with);

			if (!$asset->saveAttributes()) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return FALSE;
			}

			$ok = TRUE;
		}

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

				// create a NOTICE link for delete notification
				$nested_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (!$asset->createLink($nested_asset, SQ_LINK_NOTICE, 'nested_asset')) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}

				// get the html contents of the nested asset so
				// we change the type of the cell later - other content
				// types can use the html

				// if we dont have the frontend asset set just buffer nothing
				if (SQ_IN_LIMBO && !isset($GLOBALS['SQ_SYSTEM']->frontend_asset)) {
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
				$ok = TRUE;

			}//end if

		} else if ($current_assetid) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', TRUE, 'nested_asset');
			$ok = $asset->deleteLink($link['linkid']);
		}

		return $ok;

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
			$nested_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($nested_link['minorid'], $nested_link['minor_type_code'], TRUE);
		}

		return $nested_asset;

	}//end _getNestedAsset()


	/**
	* Get a reference to the asset the content type is nesting
	*
	* @param object	&$asset	the nest content content type object
	*
	* @return mixed object|NULL
	* @access private
	*/
	function &_getArbitraryPaintLayout(&$asset)
	{
		$paint_layout = NULL;

		$paint_layout_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', TRUE, 'paint_with_layout');
		if (!empty($paint_layout_link)) {
			$paint_layout = $GLOBALS['SQ_SYSTEM']->am->getAsset($paint_layout_link['minorid'], $paint_layout_link['minor_type_code'], TRUE);
		}

		return $paint_layout;

	}//end _getArbitraryPaintLayout()


}//end class

?>
