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
* $Id: remap_manager_edit_fns.inc,v 1.3.2.2 2004/02/21 12:25:41 brobertson Exp $
* $Name: not supported by cvs2svn $
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
* @version $Revision: 1.3.2.2 $ - 1.0
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

		$this->static_screens['details']['force_unlock'] = false;
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
	* @param &object Remap_Manager		$asset		the remap manager
	* @param &object Backend_Outputter	$o			the backend outputter
	* @param string						$prefix		a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintEditRemapURLs(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('all');

		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT url, remap_url, expires
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_lookup_remap';
		$remap_urls = $db->getAll($sql);
		if (DB::isError($remap_urls)) {
			trigger_error($remap_urls->getMessage().'<br/>'.$remap_urls->getUserInfo(), E_USER_ERROR);
		}

		if (empty($remap_urls)) {
			echo 'There are no URL remappings in the system';
			return false;
		}

		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header">URL</td>
				<td class="sq-backend-table-header">Remap URL</td>
				<td class="sq-backend-table-header">Expires</td>
				<?php
					if ($write_access) {
						?>
						<td class="sq-backend-table-header">
							<?php check_box($prefix.'_select_all', '1', false, "check_all(document.main_form, '{$prefix}_delete_remap', this.checked);"); ?>
							Delete ?
						</td>
						<?php
					}
				?>
			</tr>
			<?php
				foreach ($remap_urls as $url_info) {
					if (empty($url_info['expires'])) {
						$expires = 'Never';
					} else {
					echo $url_info['expires'];
						$expires = $GLOBALS['SQ_SYSTEM']->datetime(iso8601_ts($url_info['expires']));
					}

					// any warning text to show for this remapped URL (any possible problems)
					$warning = '';

					// check to see if the URL is actually being used by a real asset within the system
					// because if it is, this remap will never be used
					if (strpos($url_info['url'], '://') !== false) {
						list($protocol, $url) = explode('://', $url_info['url']);
						$url_asset = &$GLOBALS['SQ_SYSTEM']->am->getAssetFromURL($protocol, $url, true, true);
						if (!is_null($url_asset)) {
							$warning .= 'This URL is currently in use by '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($url_asset->type(), 'name').' "'.$url_asset->name.'" [#'.$url_asset->id.']. This remap is not currently being used. ';
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
				}
			?>
		</table>
		<?php

		return $write_access;

	}//end paintEditRemapURLs()


	/**
	* Accepts requests to add a type_code to define layouts for
	*
	* @param &object Remap_Manager		$asset		the remap manager
	* @param &object Backend_Outputter	$o			the backend outputter
	* @param string						$prefix		a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processEditRemapURLs(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('all')) return false;

		$db =& $GLOBALS['SQ_SYSTEM']->db;

		if (isset($_POST[$prefix.'_delete_remap'])) {
			$deletes = Array();
			foreach ($_POST[$prefix.'_delete_remap'] as $url => $on) $deletes[] = $db->quote($url);

			$where_cond = 'url IN ('.implode(',', $deletes).')';
			if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_lookup_remap', $where_cond)) {
				return false;
			}
		}
	
	}//end processEditRemapURLs()


	/**
	* Paints the interface to add new type_codes to define layouts for
	*
	* @param &object Remap_Manager		$asset		the remap manager
	* @param &object Backend_Outputter	$o			the backend outputter
	* @param string						$prefix		a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintAddRemapURLs(&$asset, &$o, $prefix)
	{
		?>
		The <i>Old URL</i> is the URL that you want remapped. The <i>New URL</i> is the URL that a user will be redirected to when they visit the <i>Old URL</i>.
		<?php
		$o->closeField();
		
		$o->openField('Old URL');
			text_box($prefix.'_add_remap_old_url', '', 50);
		$o->closeField();

		$o->openField('New URL');
			text_box($prefix.'_add_remap_new_url', '', 50);
		$o->closeField();

		$o->openField('Expires');
			text_box($prefix.'_add_remap_expires_days', '', 5);
			echo '&nbsp;days<br/><br/>';
		$o->closeField();

		$o->openField('CSV Upload');
			file_upload($prefix.'_add_remap_csv');
			?>
			<br/>
			You can also upload a CSV file of remap URLs to add to the system. The format for each line must be:</br>
			<b>oldURL, newURL, expiry date (yyyy-mm-dd hh:mm:ss)</b><br/><br/>
			<i><b>Example 1:</b></i> http://www.mysite.com/page_1, http://www.mysite.com/home, 2003-17-12 10:27:00<br/>
			<i><b>Example 2 (no expiry date):</b></i> http://www.mysite.com/page_1, http://www.mysite.com/home,<br/>
			<?php
		$o->closeField();

	}//end paintAddRemapURLs()


	/**
	* Accepts requests to add a type_code to define layouts for
	*
	* @param &object Remap_Manager		$asset		the remap manager
	* @param &object Backend_Outputter	$o			the backend outputter
	* @param string						$prefix		a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processAddRemapURLs(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('all')) return false;

		// do the CSV upload first
		$file_info = get_file_upload_info($prefix.'_add_remap_csv');
		if (!empty($file_info)) {
			require_once SQ_FUDGE_PATH.'/csv/csv.inc';
			$csv = new CSV($file_info['tmp_name']);
			$csv->import();

			foreach ($csv->values as $line_value) {
				$old_url     = trim($line_value[0]);
				$new_url     = trim($line_value[1]);
				if (empty($old_url) || empty($new_url)) return false;

				$expires = trim($line_value[2]);

				if (empty($expires)) {
					$int_expires = null;
				} else {
					$int_expires = iso8601_ts($expires);
				}

				$asset->addRemapURL($old_url, $new_url, $int_expires);
			}
		}


		// now add a manually set remap if we have on to set
		if (!isset($_POST[$prefix.'_add_remap_old_url']) || !isset($_POST[$prefix.'_add_remap_new_url'])) return false;
		$old_url = trim($_POST[$prefix.'_add_remap_old_url']);
		$new_url = trim($_POST[$prefix.'_add_remap_new_url']);
		$expires = null;

		if (empty($old_url) || empty($new_url)) return false;

		if (isset($_POST[$prefix.'_add_remap_expires_days'])) {
			$expires_days = (int)$_POST[$prefix.'_add_remap_expires_days'];
			if ($expires_days > 0) $expires = strtotime('+ '.$expires_days.' days');
		}

		$asset->addRemapURL($old_url, $new_url, $expires);

		return true;

	}//end processAddRemapURLs()


}//end class

?>