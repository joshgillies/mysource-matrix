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
* $Id: remap_manager_edit_fns.inc,v 1.18 2006/01/26 22:34:06 lwright Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Remap_Manager_Edit_Fns
*
* Edit functions for the remap manager.
*
* @see Asset_Edit_Fns
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.18 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Remap_Manager_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::Asset_Edit_Fns
	*/
	function Remap_Manager_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

		$this->static_screens['details']['force_unlock'] = FALSE;
		$this->static_screens['details']['lock_type'] = 'all';

		unset($this->static_screens['workflow']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['metadata']);
		unset($this->static_screens['linking']);
		unset($this->static_screens['settings']);
		unset($this->static_screens['dependants']);

	}//end constructor


	/**
	* Paints the interface to add new type_codes to define layouts for
	*
	* @param object	&$asset	the remap manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintEditRemapURLs(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('all');

		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT url, remap_url, expires
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup_remap';
		$remap_urls = $db->getAll($sql);
		assert_valid_db_result($remap_urls);

		if (empty($remap_urls)) {
			echo translate('no_url_remappings');
			return FALSE;
		}

		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header"><?php echo translate('url'); ?></td>
				<td class="sq-backend-table-header"><?php echo translate('remap_url'); ?></td>
				<td class="sq-backend-table-header"><?php echo translate('expires'); ?></td>
				<?php
					if ($write_access) {
						?>
						<td class="sq-backend-table-header">
							<?php
							check_box($prefix.'_select_all', '1', FALSE, "check_all(document.main_form, '{$prefix}_delete_remap', this.checked);");
							label(translate('delete_question'), $prefix.'_select_all');
							?>
						</td>
						<?php
					}
				?>
			</tr>
			<?php
				foreach ($remap_urls as $url_info) {
					if (empty($url_info['expires'])) {
						$expires = translate('never');
					} else {
						$expires = $GLOBALS['SQ_SYSTEM']->datetime(iso8601_ts($url_info['expires']));
					}

					// any warning text to show for this remapped URL (any possible problems)
					$warning = '';

					// check to see if the URL is actually being used by a real asset within the system
					// because if it is, this remap will never be used
					if (strpos($url_info['url'], '://') !== FALSE) {
						list($protocol, $url) = explode('://', $url_info['url']);
						$url_asset =& $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL($protocol, $url, TRUE, TRUE);
						if (!is_null($url_asset)) {
							$warning .= translate('url_currently_in_use', $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($url_asset->type(), 'name'), translate('asset_format', $url_asset->name, $url_asset->id));
						}
					}
				?>
				<tr>
					<td class="sq-backend-table-cell"><?php echo $url_info['url']; ?><br/><span style="color: #FF0000;"><b><?php echo $warning; ?></b></span></td>
					<td class="sq-backend-table-cell"><?php echo $url_info['remap_url']; ?></td>
					<td class="sq-backend-table-cell"><?php echo $expires; ?></td>
					<?php
						if ($write_access) {
							?>
							<td class="sq-backend-table-cell">
								<?php
								check_box($prefix.'_delete_remap['.$url_info['url'].']');
								?>
							</td>
							<?php
						}
					?>
				</tr>
				<?php
				}//end foreach
			?>
		</table>
		<?php

		return $write_access;

	}//end paintEditRemapURLs()


	/**
	* Accepts requests to add a type_code to define layouts for
	*
	* @param object	&$asset	the remap manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processEditRemapURLs(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('all')) return FALSE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');


		if (isset($_POST[$prefix.'_delete_remap'])) {
			$deletes = Array();
			foreach ($_POST[$prefix.'_delete_remap'] as $url => $on) {
				$deletes[] = $db->quoteSmart($url);
			}

			$sql = 'DELETE FROM
						sq_ast_lookup_remap
					WHERE
						url IN
							(
								'.implode(',', $deletes).'
							)';

			$result = $db->query($sql);
			assert_valid_db_result($result);
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end processEditRemapURLs()


	/**
	* Paints the interface to add new type_codes to define layouts for
	*
	* @param object	&$asset	the remap manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintAddRemapURLs(&$asset, &$o, $prefix)
	{
		$o->note(translate('url_remap_explanation'));
		$o->closeField();

		$o->openField(translate('old_url'));
			text_box($prefix.'_add_remap_old_url', '', 50);
		$o->closeField();

		$o->openField(translate('new_url'));
			text_box($prefix.'_add_remap_new_url', '', 50);
		$o->closeField();

		$o->openField(translate('expires'));
			text_box($prefix.'_add_remap_expires_days', '', 5);
			label(translate('days'), $prefix.'_add_remap_expires_days');
		$o->closeField();

		$o->openField(translate('csv_upload'));
			file_upload($prefix.'_add_remap_csv');

			// csv upload instructions
			$o->note(translate('csv_upload_instructions'));
			$o->note(translate('csv_upload_instructions_example_1').'<br />'.translate('csv_upload_instructions_example_2'));
		$o->closeField();

	}//end paintAddRemapURLs()


	/**
	* Accepts requests to add a type_code to define layouts for
	*
	* @param object	&$asset	the remap manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processAddRemapURLs(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('all')) return FALSE;

		// do the CSV upload first
		$file_info = get_file_upload_info($prefix.'_add_remap_csv');
		if (!empty($file_info)) {
			require_once SQ_FUDGE_PATH.'/csv/csv.inc';
			$csv =& new CSV($file_info['tmp_name']);
			$csv->import();

			foreach ($csv->values as $line_value) {
				$old_url     = trim($line_value[0]);
				$new_url     = trim($line_value[1]);
				if (empty($old_url) || empty($new_url)) {
					return FALSE;
				}

				$expires = array_key_exists(2, $line_value) ? trim($line_value[2]) : '';

				if (empty($expires)) {
					$int_expires = NULL;
				} else {
					$int_expires = iso8601_ts($expires);
				}

				$asset->addRemapURL($old_url, $new_url, $int_expires);
			}
		}


		// now add a manually set remap if we have on to set
		if (!isset($_POST[$prefix.'_add_remap_old_url']) || !isset($_POST[$prefix.'_add_remap_new_url'])) {
			return FALSE;
		}
		$old_url = trim($_POST[$prefix.'_add_remap_old_url']);
		$new_url = trim($_POST[$prefix.'_add_remap_new_url']);
		$expires = NULL;

		if (empty($old_url) || empty($new_url)) {
			return FALSE;
		}

		if (isset($_POST[$prefix.'_add_remap_expires_days'])) {
			$expires_days = (int)$_POST[$prefix.'_add_remap_expires_days'];
			if ($expires_days > 0) {
				$expires = strtotime('+ '.$expires_days.' days');
			}
		}

		$asset->addRemapURL($old_url, $new_url, $expires);

		return TRUE;

	}//end processAddRemapURLs()


	/**
	* Paints the root node selection box
	*
	* @param object	&$asset	the remap manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintAddPageNotFoundRemap(&$asset, &$o, $prefix)
	{
		$link_to_404 = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'asset', FALSE, '404');
		$write_access = $asset->writeAccess('links');

		if (!$write_access) {
			if (!empty($link_to_404)) {
				echo get_asset_tag_line($link_to_404['minorid']);
			} else {
				echo '<em>'.translate('none').'</em>';
			}
		} else {
			asset_finder($prefix.'_404asset', (!empty($link_to_404)) ? $link_to_404['minorid'] : '');
		}

		return $write_access;

	}//end paintAddPageNotFoundRemap()


	/**
	* Processes the value input from root node selection box
	*
	* @param object	&$asset	the remap manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return void
	* @access public
	*/
	function processAddPageNotFoundRemap(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_404asset']['assetid'])) {
			return FALSE;
		}
		$new_404_asset  = $_POST[$prefix.'_404asset']['assetid'];

		// getting existing root link
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', FALSE, '404');

		// dont process anything if the root node has not changed
		if (!empty($root_link) && $root_link['minorid'] == $new_404_asset) {
			return FALSE;
		}

		$ok = TRUE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// deleting old root link if any found...
		if (!empty($root_link) && !$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($root_link['linkid'])) {
			$ok = FALSE;
		} else if ($new_404_asset) {
			$root =& $GLOBALS['SQ_SYSTEM']->am->getAsset($new_404_asset);
			if (!is_null($root)) {
				if (!$asset->createLink($root, SQ_LINK_NOTICE, '404')) {
					$ok = FALSE;
					trigger_localised_error('CORE0027', E_USER_WARNING, $root->name, $root->id);
				}
			}
		}

		if ($ok) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $ok;

	}//end processAddPageNotFoundRemap()


}//end class

?>