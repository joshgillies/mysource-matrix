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
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/csv/csv.inc';
require_once dirname(__FILE__).'/condition_visited_pages.inc';


/**
* Condition_Pages_Visited_Edit_Fns
*
* Edit functions for the Pages Visited condition
*
* @see Asset_Edit_Fns
*
* @author Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_Visited_Pages_Edit_Fns extends Condition_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::__construct()
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Paint the backend editing interface for this condition
	*
	* @param array				&$condition_data	Array of settings for this condition
	* @param Backend_Outputter	$o					Backend outputter to use
	* @param string				$prefix				Prefix for form elements
	* @param boolean			$write_access		Whether we have access to edit the field values
	*
	* @return void
	* @access public
	*/
	public static function paintEditInterface(Array $condition_data, Backend_Outputter $o, $prefix, $write_access)
	{
		if (!isset($condition_data['url_patterns'])) {
			$condition_data['url_patterns'] = Array();
		}

		if (!isset($condition_data['assetids'])) {
			$condition_data['assetids'] = Array();
		}

		if ($write_access) {
			$condition_data['url_patterns']['new'] = '';
		}

		?>
		<table class="sq-backend-table">
			<tr>
				<th class="sq-conditions-options-th"><?php echo translate('url_patterns'); ?></th>
				<td>
				<?php
				if (!$write_access && empty($condition_data['url_patterns'])) {
					?><em><?php echo translate("condition_no_patterns_entered"); ?></em><?php
				} else {
					?>
					<table class="sq-backend-table" style="margin-bottom: 0;">
						<tr>
						<?php
						if ($write_access) {
							echo '<th></th>';
						}
						?>
						<th><?php echo translate('pattern'); ?></th>
						<?php if ($write_access) {
						?>
							<th style="text-align: center;"><?php echo translate('delete_question'); ?></th>
						<?php
						}
						?>
						</tr>
						<?php foreach ($condition_data['url_patterns'] as $index => $val) {
						?>
							<tr>
								<?php if ($write_access) {
								?>
								<td>
									<?php
									if ($index === 'new') {
										echo '<strong>'.translate('new_question').'</strong>';
									}
									?>
								</td>
								<?php
								}
								?>
								<td>
									<?php
									if ($write_access) {
										text_box($prefix.'[url_patterns]['.$index.']', $val, 80);
									} else {
										echo $val;
									}
									?>
								</td>
								<?php
									if ($write_access && ($index !== 'new')) {
										echo '<td style="text-align: center;">';
										check_box($prefix.'['.$index.'][delete]', FALSE);
										echo '</td>';
									} else if ($write_access) {
										echo '<td></td>';
									}
								?>
							</tr>
						<?php
						}//end foreach
						?>
					</table>
				<?php
				}//end else
				?>
				</td>
			<tr>
				<th><?php echo translate('asset_ids'); ?></th>
				<td>
				<?php
				if ( $write_access) {
					multiple_asset_finder($prefix.'[assetids]', $condition_data['assetids']);
				} else {
					if (empty($condition_data['assetids'])) {
						echo '<em>'.translate('condition_no_assetids_selected').'</em>';
					} else {
						echo '<ul class="sq-asset-id-list">';
						foreach ($condition_data['assetids'] as $assetid) {
							echo '<li>'.get_asset_tag_line($assetid).'</li>';
						}
						echo '</ul>';
					}
				}
				?>
				</td>
			</tr>
		</table>
		<?php
		$o->note(translate('condition_site_referrer_note', SQ_CONF_VISITED_PAGES_MAX_ENTRIES));

	}//end paintEditInterface()


	/**
	* Process the backend editing interface for this condition
	*
	* @param Backend_Outputter	$o		Backend outputter to use
	* @param string				$prefix	Prefix for form elements
	*
	* @return array
	* @access public
	*/
	public static function processEditInterface(Backend_Outputter $o, $prefix)
	{
		if (empty($_POST[$prefix])) {
			return Array();
		}

		// Process URL patterns
		$url_patterns = Array();
		foreach($_POST[$prefix]['url_patterns'] as $index => $value) {
			if (!empty($value) && !isset($_POST[$prefix][$index]['delete']) && !in_array($value, $url_patterns)) {
				$url_patterns[] = $value;
			}
		}

		// Process assetids
		$assetids = Array();
		foreach ($_POST[$prefix]['assetids'] as $data) {
			if (!empty($data['assetid']) && !in_array($data['assetid'], $assetids)) {
				$assetids[] = $data['assetid'];
			}
		}//end foreach

		$result = Array(
					'url_patterns' => $url_patterns,
					'assetids'	=> $assetids,
				);

		return $result;

	}//end processEditInterface()

}//end class

?>
