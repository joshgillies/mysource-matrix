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

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_condition_types/trigger_condition_asset_type/trigger_condition_asset_type.inc';

/**
* Trigger Condition
*
* Checks whether an asset is of a particular type
*
*
* @author   Maki Maruyama <mmaruyama@squiz.co.uk>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Condition_Advanced_Keyword_Match extends Trigger_Condition_Asset_Type
{

	/**
	* Evaluate this condition
	*
	* The settings used by this condition are in the form:
	* <PRE>
	* Array(
	*       'keyword'		=> string   // keyword to apply (asset/globals)
	*       'replacement_value'	=> boolean  // compare string > after the keyword above is replaced, the replacement value will be compared
	*						with this value
	*	'match_type'		=> string   // how the values are compared - exact match, partial match or regex
	*       );
	* </PRE>
	*
	* @param array	$settings	the stored settings for this condition
	* @param array	&$state		the state that describes the asset the condition is executing on
	*
	* @return boolean
	* @access public
	*/
	public static function evaluate($settings, &$state)
	{
		if (empty($settings['conditions'])) return TRUE;

		if (empty($state['asset'])) {
			if (empty($state['assetid'])) return FALSE;
			$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
		}

		$group_logic = $settings['group_logic'];
		$conditions  = $settings['conditions'];

		// we need to make sure that the triggering asset is used to repalce global keywords,
		// so, temporarily switch the 'current asset' to the triggering asset
		$original_current_asset = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('CURRENT_ASSET', NULL);
		$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $state['asset']);
		$trigger_sum_result = TRUE;
		$trigger_diff_result = FALSE;
		$trigger_result = FALSE;

		foreach ($conditions as $groups) {
			$logic = $groups['condition_logic'];
			unset ($groups['condition_logic']);

			$group_result = FALSE;
			$sum_result = TRUE;

			// evaluate conditions in groups
			foreach ($groups as $condition) {
				$result = Trigger_Condition_Advanced_Keyword_Match::evaluateCondition($state['asset'], $condition['keyword'], $condition['pattern']);

				$sum_result = $sum_result && $result;
				$diff_result = $sum_result || $result;
				if ($logic === 'one_match' && $result) {
					// if one match, any true will be true for all
					$group_result = TRUE;
					break;
				} else if ($logic === 'all_match' && !$result) {
					// if all match, any false will be false for all
					$group_result = FALSE;
					break;
				} else if ($logic === 'none_match' && $result) {
					// if none match, any true will be a false for all
					$group_result = FALSE;
					break;
				} else if ($logic === 'one_not_match' && !$result) {
					// if one doesn't match, any false will be a true for all
					$group_result = TRUE;
					break;
				}
			}

			// if all match and all conditions are true, then the group is true, otherwise just use default value
			if ($logic === 'all_match' && $sum_result) $group_result = TRUE;

			// if none match and all conditions are false, then the group is true, otherwise just use default value
			if ($logic === 'none_match' && !$diff_result) $group_result = TRUE;


			// sum up results of groups
			$trigger_sum_result  = $trigger_sum_result && $group_result;
			$trigger_diff_result = $trigger_diff_result || $group_result;
			if($group_logic === 'one_match' && $group_result) {
				// if one match is required, any group is true would be true for all
				$trigger_result = TRUE;
				break;
			}
		}

		// we are done with the temporary current asset, so, let's put it back to the original state
		if (is_null($original_current_asset)) {
			$GLOBALS['SQ_SYSTEM']->unsetGlobalDefine('CURRENT_ASSET');
		} else {
			$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $original_current_asset);
		}

		// if all match and all of groups is true, then final result is true, otherwise, default to false
		if ($group_logic === 'all_match' && $trigger_sum_result) $trigger_result = TRUE;

		return $trigger_result;

	}//end evaluate()


	/**
	* Get the HTML editing interface for this condition. Returns the string instead of printing it
	*
	* @param array		$settings	settings that this condition save in processInterface()
	* @param string		$prefix		unique prefix
	* @param boolean	$write_access	write access flag
	*
	* @return boolean
	* @access public
	*/
	public static function getInterface($settings, $prefix, $write_access=FALSE)
	{
		// check if we already have values set so that we can pre-set the values where possible below
		$group_logic = array_get_index($settings, 'group_logic', 'all_match');
		$conditions  = array_get_index($settings, 'conditions', Array());

		$am = $GLOBALS['SQ_SYSTEM']->am;

		ob_start();
			echo '<button type="button" class="delete-button" title="Delete Condition" onclick="this.parentNode.parentNode.parentNode.removeChild(this.parentNode.parentNode);return false;"></button>';
		$delete_icon_condition = ob_get_clean();

		ob_start();
			echo '<button type="button" class="delete-button" title="Delete Group" onclick="this.parentNode.parentNode.parentNode.parentNode.removeChild(this.parentNode.parentNode.parentNode);return false;"></button>';
		$delete_icon_group = ob_get_clean();


		$logic_values = Array (
							'all_match'		=> translate('all groups are matched'),
							'one_match'		=> translate('at least 1 group is matched'),
						);

		// keyword input box
		ob_start();
			if ($write_access) Trigger_Condition_Advanced_Keyword_Match::_includeJS($prefix);
			$disabled = $write_access ? '' : ' disabled="disabled"'
				?>
					<p class="sq-backend-section-subheading"><?php echo translate('Group & Condition Settings'); ?></p>
					<table class="sq-backend-table" id="condition_main_table">
						<tr>
							<th style="width:75px">
								<?php echo translate('Group Logic'); ?>
							</th>
							<td>
								<span class="sq-inline-fields-wrapper">
								<?php 
									echo translate('This trigger condition will be evaluated as True if ');
									combo_box($prefix.'[group_logic]', $logic_values, FALSE, $group_logic, 0, $disabled);
								?>
								</span>
							</td>
						</tr>
						<?php
						// add the other options for our logical match for conditions within the group
						$condition_logic = Array(
											'all_match'		=> translate('all conditions are matched'),
											'one_match'     => translate('at least 1 condition is matched'),
											'none_match'	=> translate('all conditions are NOT matched'),
											'one_not_match' => translate('at least 1 condition is NOT matched')
										   );

						foreach ($conditions as $index => $group) {
							$logic = $group['condition_logic'];
							unset ($group['condition_logic']);
						?>
						<tr>
							<th>
								<?php echo translate('Group'); ?>
							</th>
							<td>
								<span class="sq-inline-fields-wrapper">
								<?php
								echo translate('This group will be matched if ');
								combo_box($prefix.'[conditions]['.$index.'][condition_logic]', $condition_logic, FALSE, $logic, 0, $disabled);
								?>
								</span>
								<?php
								if ($write_access) {
								?>
								<span style="float:right"><?php echo $delete_icon_group; ?></span>
								<?php
								}
								?>
									<table style="margin: 5px 0;" id="condition_groups_table_<?php echo $index; ?>" class="sq-backend-table">
										<thead>
											<tr>
												<th width="47%">
													<?php echo translate('Keyword'); ?>
												</th>
												<th width="47%">
													<?php echo translate('Pattern'); ?>
												</th>
												<?php
												if ($write_access) {
												?>
												<th style="text-align:center;">
													<?php echo translate('Delete?'); ?>
												</th>
												<?php
												}
												?>
											</tr>
										</thead>
										<tbody>
									<?php
									foreach ($group as $idx => $condition) {
										if (empty($condition['keyword'])) continue;
									?>
										<tr>
											<td>
												<?php text_box($prefix.'[conditions]['.$index.']['.$idx.'][keyword]', $condition['keyword'], '', '', FALSE, 'class="sq-cell-text-box-width"'.$disabled); ?>
											</td>
											<td>
												<?php text_box($prefix.'[conditions]['.$index.']['.$idx.'][pattern]', $condition['pattern'], '', '', FALSE, 'class="sq-cell-text-box-width"'.$disabled); ?>
											</td>
											<?php
											if ($write_access) {
											?>
											<td style="text-align:center;">
													<?php echo $delete_icon_condition; ?>
											</td>
											<?php
											}
											?>
										</tr>
											<?php
										}
										?>
										</tbody>
									</table>
									<?php
									if ($write_access) {
									?>
									<input type="button" name="add_condition_<?php echo $index; ?>" id="add_condition_<?php echo $index; ?>" value="Add Condition" onclick="trigger_condition_advanced_keyword_match.add_condition('<?php echo $index; ?>'); return false;"/>
									<?php
									}
									?>
								</td>
							</tr>
							<?php
							}

							if ($write_access) {
							?>
							<tr>
								<td colspan="2">
									<input type="button" name="add_group" id="add_group" value="Add Group" onclick="trigger_condition_advanced_keyword_match.add_group(); return false;"/>
								</td>
							</tr>
							<?php
							}
							?>
					</table>
				<div class="sq-backend-note">
					<strong>Note:</strong> A condition will match if the keyword replacement matches the pattern. Regular expressions and global keywords are supported in patterns.
				</div>
			<?php

			$content = ob_get_contents();
		ob_end_clean();

		return $content;

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	*
	* @param array	&$settings		a container for any data the condition might want to save
	* @param array	$request_data	array of data corresponding to this condition
	*								as specified by the $prefix in the getInterface.
	*
	* @return mixed boolean|string
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		if (empty($request_data)) {
			return translate('Input Data is missing');
		}

		$settings['conditions'] = array_get_index($request_data, 'conditions', Array());
		$settings['group_logic'] = array_get_index($request_data, 'group_logic', 'all_match');

		return FALSE;

	}//end processInterface()


	/**
	* Determines whether a condition is allowed to be used multiple times in a trigger
	* Default to false, children can override if necessary
	*
	* @return boolean
	* @access public
	*/
	public static function allowMultiple()
	{
		// default to to disallow
		return TRUE;

	}//end allowMultiple()


	/**
	* Evaluates the keyword passed along to the expected value against the asset
	*
	*
	* @param object	$asset		The asset we need to evaluate the keywords against
	* @param string	$keyword	The keyword to evaluate
	* @param string $pattern	The pattern to matcht he keyword against
	*
	* @return boolean
	* @access protected
	*/
	protected static function evaluateCondition($asset, $keyword, $pattern)
	{
		// try replacing the asset keyword
		// if the keyword is a globals keyword, this function simply returns the globals keyword as it is
		// note: we are currently allowing only one keyword to be specified here, so, we strip '%' from the
		//       specified keyword, in case multiple keywords are set here
		$keyword = str_replace('%', '', $keyword);
		$asset_keyword_replacement = $asset->getKeywordReplacement($keyword);

		// now try replacing the globals keyword
		replace_global_keywords($asset_keyword_replacement);
		
		if (strpos($pattern, '%') !== FALSE) {
			// we need to run keyword replacement on the pattern as well
			replace_global_keywords($pattern);
		}

		return preg_match('@'.$pattern.'@', $asset_keyword_replacement);

	}//end evaluateCondition()


	/**
	* Includes the JS to add/remove groups/conditions
	*
	*
	* @return void
	* @access protected
	*/
	protected static function _includeJS($prefix)
	{
		?>
			<script type="text/javascript">

			trigger_condition_advanced_keyword_match = {
				// add a condition group to the table
				add_group: function() {

					// add group row
					var table = document.getElementById('condition_main_table');
					var tbody = table.getElementsByTagName('tbody')[0];
					var current_tr = tbody.rows.length;

					// our last row is 'add group' button so we need to ignore that row
					current_tr--;

					var tr = tbody.insertRow(current_tr);

					// the row id in matrix are zero index based and js length isn't
					// so we will need to decrement the length by one for things to work correctly
					current_tr--;

					var th_label = document.createElement('th');
					var td_content = document.createElement('td');

					var name = document.createTextNode('<?php echo translate('Group'); ?>');
					th_label.appendChild(name);

					// add table for conditions
					var condition_table = document.createElement('table');
					condition_table.setAttribute('id', 'condition_groups_table_' + current_tr);
					condition_table.setAttribute('class', 'sq-backend-table');
					condition_table.setAttribute('style', 'margin: 5px 0;');

					var thd =document.createElement('thead');
					var tbd =document.createElement('tbody');
					condition_table.style.width='100%';

					var tr_inner = document.createElement('tr');
					var th_keyword = document.createElement('th');
					var th_pattern = document.createElement('th');
					var th_delete  = document.createElement('th');

					var th_keyword_name = document.createTextNode('<?php echo translate('Keyword'); ?>');
					var th_pattern_name = document.createTextNode('<?php echo translate('Pattern'); ?>');
					var th_delete_name  = document.createTextNode('<?php echo translate('Delete?'); ?>');

					th_keyword.appendChild(th_keyword_name);
					th_pattern.appendChild(th_pattern_name);
					th_delete.appendChild(th_delete_name);

					th_pattern.setAttribute("width", "47%");
					th_keyword.setAttribute("width", "47%");

					th_delete.style.textAlign = 'center';

					tr_inner.appendChild(th_keyword);
					tr_inner.appendChild(th_pattern);
					tr_inner.appendChild(th_delete);
					thd.appendChild(tr_inner);
					condition_table.appendChild(thd);
					condition_table.appendChild(tbd);

					td_innerspan = document.createElement("span");
					td_innerspan.setAttribute("class", "sq-inline-fields-wrapper");
					// add the logical grouping option
					td_innerspan.innerHTML = '<?php echo 'This group will be matched if '; ?>';


					var select = document.createElement("select");
					select.setAttribute("class", "logical_op_conditions");
					select.setAttribute("id", '<?php echo $prefix; ?>[conditions][' + current_tr + '][condition_logic]');
					select.setAttribute("name", '<?php echo $prefix; ?>[conditions][' + current_tr + '][condition_logic]');

					var option_1 = document.createElement("option");
					option_1.setAttribute('value', 'all_match');
					option_1.innerHTML = '<?php echo translate('all conditions are matched'); ?>';
					select.appendChild(option_1);
					var option_2 = document.createElement("option");
					option_2.setAttribute('value', 'one_match');
					option_2.innerHTML = '<?php echo translate('at least 1 condition is matched'); ?>';
					select.appendChild(option_2);
					var option_3 = document.createElement("option");
					option_3.setAttribute('value', 'none_match');
					option_3.innerHTML = '<?php echo translate('all conditions are NOT matched'); ?>';
					select.appendChild(option_3);
					var option_4 = document.createElement("option");
					option_4.setAttribute('value', 'one_mismatch');
					option_4.innerHTML = '<?php echo translate('at least 1 condition is NOT matched'); ?>';
					select.appendChild(option_4);

					// print delete condition icon
					var deleteIcon = document.createElement('button');
					deleteIcon.alt = '<?php echo translate('Delete Group'); ?>';
					deleteIcon.title = '<?php echo translate('Delete Group'); ?>';
					deleteIcon.setAttribute('type', 'button');
					deleteIcon.setAttribute('class', 'delete-button');

					deleteIcon.onclick = function () {
						this.parentNode.parentNode.parentNode.parentNode.removeChild(this.parentNode.parentNode.parentNode);
						return false;
					};

					var floating_span = document.createElement("span");
					floating_span.setAttribute('style', 'float:right');
					floating_span.appendChild(deleteIcon);

					var add_cond_btn = document.createElement('input');
					add_cond_btn.setAttribute('type', 'button');
					add_cond_btn.setAttribute('name', 'add_condition_' + current_tr);
					add_cond_btn.setAttribute('id', 'add_condition_' + current_tr);
					add_cond_btn.setAttribute('value', 'Add Condition');
					add_cond_btn.setAttribute('onclick', 'trigger_condition_advanced_keyword_match.add_condition(' + current_tr + '); return false;');

					td_innerspan.appendChild(select);
					td_content.appendChild(td_innerspan);
					td_content.appendChild(floating_span);
					td_content.appendChild(condition_table);
					td_content.appendChild(add_cond_btn);
					tr.appendChild(th_label);
					tr.appendChild(td_content);

					trigger_condition_advanced_keyword_match.add_condition(current_tr);

					return false;
				},


				add_condition: function(parent_id) {
					var table = document.getElementById('condition_groups_table_' + parent_id);
					var tbody = table.getElementsByTagName('tbody')[0];
					var current_tr = tbody.rows.length;
					// row length is not zero based index
					// where as our text box has zero based index
					// if (parent_id != 0) parent_id--;

					var tr =document.createElement('tr');
					var td_keyword = document.createElement('td');
					var td_pattern = document.createElement('td');
					var td_delete  = document.createElement('td');

					var text_field_keyword = document.createElement('input');
					var text_field_pattern = document.createElement('input');
					text_field_keyword.setAttribute('type', 'text');
					text_field_pattern.setAttribute('type', 'text');
					text_field_keyword.setAttribute('size', '50');
					text_field_pattern.setAttribute('size', '50');

					text_field_keyword.setAttribute('id', '<?php echo $prefix; ?>[conditions][' + parent_id + '][' + current_tr + '][keyword]');
					text_field_pattern.setAttribute('id', '<?php echo $prefix; ?>[conditions][' + parent_id + '][' + current_tr + '][pattern]');

					text_field_keyword.setAttribute('name', '<?php echo $prefix; ?>[conditions][' + parent_id + '][' + current_tr + '][keyword]');
					text_field_pattern.setAttribute('name', '<?php echo $prefix; ?>[conditions][' + parent_id + '][' + current_tr + '][pattern]');

					text_field_keyword.setAttribute('class', 'sq-cell-text-box-width');
					text_field_pattern.setAttribute('class', 'sq-cell-text-box-width');

					// print delete condition icon
					var deleteIcon=document.createElement('button');
					deleteIcon.src = "<?php echo(sq_web_path('data').'/asset_types/bodycopy/images/icons/delete.png'); ?>";
					deleteIcon.alt = '<?php echo translate('Delete Condition'); ?>';
					deleteIcon.title = '<?php echo translate('Delete Condition'); ?>';
					deleteIcon.setAttribute('type', 'button');
					deleteIcon.setAttribute('class', 'delete-button');

					deleteIcon.onclick = function () {
						this.parentNode.parentNode.parentNode.removeChild(this.parentNode.parentNode);
						return false;
					};

					td_keyword.appendChild(text_field_keyword);
					td_pattern.appendChild(text_field_pattern);
					td_delete.appendChild(deleteIcon);

					td_delete.style.textAlign = 'center';

					tr.appendChild(td_keyword);
					tr.appendChild(td_pattern);
					tr.appendChild(td_delete);

					tbody.appendChild(tr);

					return false;
				}
			}

			</script>
		<?php

	}//end _includeJS


}//end class

?>
