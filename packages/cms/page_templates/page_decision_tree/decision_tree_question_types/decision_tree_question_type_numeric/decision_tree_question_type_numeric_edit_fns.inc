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
* $Id: decision_tree_question_type_numeric_edit_fns.inc,v 1.4 2012/08/30 00:58:08 ewang Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_decision_tree/decision_tree_question/decision_tree_question_edit_fns.inc';

/**
* Decision_Tree_Question_Type_Numeric_Edit_Fns
*
*
* @author  Mohamed Haidar <mhaidar@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Decision_Tree_Question_Type_Numeric_Edit_Fns extends Decision_Tree_Question_Edit_Fns
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
	* Paint a table for the destination data for each key of the 'rules' attribute.
	* Destination data includes the next question, result bodycopy and points value.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDestinations(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$rules = $asset->attr('rules');
		$q_dest = $asset->attr('question_destinations');
		
		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header">
					<?php echo translate('answer_rule'); ?>
				</td>
				<td class="sq-backend-table-header">
					<?php echo translate('next_question'); ?>
				</td>
				<td class="sq-backend-table-header">
					<?php echo translate('result_bodycopy'); ?>
				</td>
				<td class="sq-backend-table-header">
					<?php echo translate('points_value'); ?>
				</td>
			</tr>
		<?php 
		foreach ($rules as $rule => $details){
			?>
			<tr>
				<td class="sq-backend-table-cell">
				<?php
					$asset->printDescription($details);
				?>
				</td>
				<td class="sq-backend-table-cell">
				<?php
				$nxt_q = (!empty($q_dest[$rule]['next_question'])) ? $q_dest[$rule]['next_question'] : '';
				if ($write_access){
					$type_codes = Array ( 
									  'decision_tree_question_type_select' 	=> 'I',
									  'decision_tree_question_type_numeric' => 'I',
									);
					asset_finder($prefix.'_next_question_'.$rule, $nxt_q, $type_codes);
				} else {
					if ($nxt_q) echo get_asset_tag_line($nxt_q, 'details');
				}
				?>
				</td>
				<td class="sq-backend-table-cell">
				<?php
				$res_body = (!empty($q_dest[$rule]['result_bodycopy'])) ? $q_dest[$rule]['result_bodycopy'] : '';
				if ($write_access){
					$type_codes = Array ( 
									  'bodycopy' 	=> 'I',
									);
					asset_finder($prefix.'_bodycopy_'.$rule, $res_body, $type_codes);
				} else {
					if ($res_body) echo get_asset_tag_line($res_body, 'details');
				}
				?>
				</td>
				<td class="sq-backend-table-cell">
				<?php
				$points = (!empty($q_dest[$rule]['points_value'])) ? $q_dest[$rule]['points_value'] : 0;
				if ($write_access){
					text_box($prefix.'_points_value_'.$rule, $points);
				} else {
					echo $points;
				}
				?>
				</td>
			</tr>
		<?php }
		?> 
		</table> 
		<?php
		return TRUE;
	}//end paintDestinations()
	
	
	/**
	* Processes a table for the destination data for each key of the 'rules' attribute.
	* Destination data includes the next question, result bodycopy and points value.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processDestinations(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		$rules = $asset->attr('rules');
		$q_dest = Array();
		
		foreach ($rules as $rule => $details){
			
			$nxt_q = (isset($_POST[$prefix.'_next_question_'.$rule])) ? $_POST[$prefix.'_next_question_'.$rule]['assetid'] : '';
			if (!empty($nxt_q)){
				//check if its a child else create a link under it
				$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($nxt_q, 'decision_tree_question', FALSE, NULL, NULL, TRUE, 1, 1);
				if (!array_key_exists($asset->id, $parents)){
					$minor = $GLOBALS['SQ_SYSTEM']->am->getAsset($nxt_q);
					if(!$GLOBALS['SQ_SYSTEM']->am->createAssetLink($asset, $minor, SQ_LINK_TYPE_2)) $nxt_q = '';
				}
			}
			$q_dest[$rule]['next_question'] = $nxt_q; 
			
			$res_body = (isset($_POST[$prefix.'_bodycopy_'.$rule])) ? $_POST[$prefix.'_bodycopy_'.$rule]['assetid'] : '';
			$q_dest[$rule]['result_bodycopy'] = $res_body;
			
			$points = (isset($_POST[$prefix.'_points_value_'.$rule])) ? $_POST[$prefix.'_points_value_'.$rule] : 0;
			if (!is_numeric($points)) {
				trigger_localised_error('CMS0114', E_USER_NOTICE);
				$points = 0;
			}
			$q_dest[$rule]['points_value'] = $points;
			
		}
		
		$asset->setAttrValue('question_destinations', $q_dest);
		$asset->saveAttributes();
			
		return TRUE;
	}//end processDestinations()
	
	/**
	* Print the interface for adding potential numeric answer ranges.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintOptions(&$asset, &$o, $prefix)
	{
		
		$write_access = $asset->writeAccess('attributes');
		
		$rules = $asset->attr('rules');
		$contents = $asset->getOperators();
		
		if ($write_access) {

			?><table class="sq-backend-table" style="width:auto">
			<tr>
				<th><?php echo translate('range_type'); ?></th>
				<th><?php echo translate('range_values'); ?></th>
				<th><?php echo translate('delete'); ?></th>
			</tr><?php
			foreach ($rules as $key => $item) {
				?><tr><?php
				?><td><?php 
				
				combo_box($prefix.'[RULES][default]['.$key.'][logic]', $contents, FALSE, $item['logic']);

				?></td><?php
				?><td><?php 
				
				text_box($prefix.'[RULES][default]['.$key.'][value_1]', $item['value_1'], 10);
				echo '&nbsp';
				text_box($prefix.'[RULES][default]['.$key.'][value_2]', $item['value_2'], 10);
				
				?></td><?php
				?><td><?php check_box($prefix.'[RULES][default]['.$key.'][delete]', 1, FALSE, 'document.getElementById("'.$prefix.'[RULES][default]['.$key.'][logic]").disabled = document.getElementById("'.$prefix.'[RULES][default]['.$key.'][value_1]").disabled = document.getElementById("'.$prefix.'[RULES][default]['.$key.'][value_2]").disabled = this.checked') ?></td><?php
				?></tr><?php	
			}
			?>
			<tr>
			<td colspan="3"><a href="#" id="<?php echo $prefix?>[RULES][add]" onclick="addAnother(this.parentNode.parentNode); return false;">Add another...</a></td>
			</tr>
			</table>
			<script type="text/javascript">
			nextIndex = <?php echo (int)count($rules); ?>;
			function addAnother(node) {
				
				var options = '<?php foreach ($contents as $value => $desc) {
										echo '<option value="'.$value.'">'.$desc.'</option>';
									}
								?>';
				var prefix = '<?php echo $prefix; ?>';
				var parent = node.parentNode;

				// create a new row
				tr = document.createElement('tr');

				// build the cells
				td1 = document.createElement('td');
				td1.innerHTML = '<select name="'+prefix+'[RULES][default][' + nextIndex + '][logic]" class="sq-form-field">'+options+'</select>';
				td2 = document.createElement('td');
				var box1 = '<input type="text" name="'+prefix+'[RULES][default][' + nextIndex + '][value_1]" value=""  size="10" />&nbsp;';
				td2.innerHTML = box1+'<input type="text" name="'+prefix+'[RULES][default][' + nextIndex + '][value_2]" value=""  size="10" />';
				td3 = document.createElement('td');
				td3.innerHTML = '<em>new</em>';

				// Now put them all together
				tr.appendChild(td1);
				tr.appendChild(td2);
				tr.appendChild(td3);
				parent.insertBefore(tr, node);

				// Go to next index
				nextIndex++;
			}
			</script>
			<?php

		} else {

			?><table class="sq-backend-table" style="width:auto">
			<tr>
				<th>Answer Rules</th>
			</tr><?php
			foreach ($rules as $key => $item) {
				?><tr><?php
				?><td><?php $asset->printDescription($item); ?></td><?php
				?></tr><?php
			}
			?></table><?php
		}
		
		return TRUE;
		
	}//end paintOptions()
	
	/**
	* Processes the interface for adding potential numeric answer ranges.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processOptions(&$asset, &$o, $prefix)
	{
		
		if (!$asset->writeAccess('attributes')) return FALSE;
		
		if (isset($_POST[$prefix]['RULES'])) {
			// Get what we have been given
			$rules = $_POST[$prefix]['RULES']['default'];
			
			foreach ($rules as $key => $value) {
				// Get rid of indexes we don't need, as a cleansing measure
				$allowed_sub_keys = Array('logic', 'value_1', 'value_2', 'delete');
				foreach ($value as $sub_key) {
					if (!in_array($sub_key, $allowed_sub_keys)) {
						unset($rules[$key][$sub_key]);
					}
				}
	
				// If we are set to delete a row, then delete the lot of it
				if (array_get_index($value, 'delete', FALSE)) {
					unset($rules[$key]);
				}
			}
			
			$processed_rules = Array();
			foreach ($rules as &$value) {
				$value['value_1'] = trim($value['value_1']);
				$value['value_2'] = trim($value['value_2']);
				if ($value['value_1'] == '' && $value['value_2'] == '') continue;
				$new_key = strtolower($value['logic'].'_'.$value['value_1'].'_'.$value['value_2']);
				$processed_rules[$new_key] = $value;
			}
			unset($value);
			
			$asset->setAttrValue('rules', $processed_rules);
			$asset->saveAttributes();
		}
		
		return TRUE;
			
	}//end processOptions()


}//end class
?>
