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
* $Id: form_question_edit_fns.inc,v 1.25.2.2 2004/10/22 05:55:42 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Form_Question_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Form_Question_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		// override static screens - only display details because the others don't apply
		$this->static_screens = Array(	'details'		=> Array(
															'name'			=> 'Details',
															'force_unlock'	=> false,
															'lock_type'		=> 'none',
															),
									);

	}//end Form_Question_Edit_Fns()


	/**
	* paint the current rules
	*
	* @param object Form_Section		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintRules(&$asset, &$o, $prefix)
	{
		if ($asset->getRuleCount() == 0) return false;

		$rule_codes = $asset->getAllowedRules();
		$type_codes = Array();

		foreach	($rule_codes as $rule_code) {
			$type_codes[] = 'form_question_rule_type_'.$rule_code;
		}

		$desc = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_codes, 'description');

		$admin_access = $asset->writeAccess('attributes');
		$prefix = $asset->getPrefix();
		?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header">
							Rule
						</td>
						<td class="sq-backend-table-header">
							Comparison Value
						</td>
						<?php
						if ($admin_access) {
							?><td align="center" width="100" class="sq-backend-table-header" style="font-weight: bold;">Delete ?</td>
							<?php
						}
						?>
					</tr>
				<?php
				$allowed_rules = &$asset->getAllowedRules();
				$rules = &$asset->getRules();
				if(!empty($rules)) {

					// sort order is based upon shadow asset id (eg. '87:q1'), not just question id - this 	
					// will allow sections to be sorted in later
					$i = 0;
					for (reset($rules); null !== ($k = key($rules)); next($rules)) {
						$rule = &$rules[$k];
						$type_code = 'form_question_rule_type_'.$rule['rule_code'];

						// get operators and attributes from a static call
						$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
						$operators = eval('return '.$type_code.'::getOperators();');
						$attributes = eval('return '.$type_code.'::getAttributes();');

						// obtain a rule prefix
						$rule_prefix = $prefix.'_r'.$k;
					?>
						<tr>
							<td class="sq-backend-table-cell">
								<?php if ($asset->writeAccess('attributes')) {
									combo_box($rule_prefix.'_operator', $operators, 
									false, $rule['operator']); 
								} else {
									echo $operators[$rule['operator']];
								} ?>
							</td>
							<td class="sq-backend-table-cell">
								<?php if (!$attributes['no_comparison']) {
									// only print if we need to compare with something
									if ($asset->writeAccess('attributes')) {
										if ($asset->isSelection() && !$attributes['force_text']) {
											$options = $asset->getOptions();
											combo_box($rule_prefix.'_value', array_merge(Array(' ' => '------ Please Select ------'), $options), 
									false, $rule['value']); 
										} else {
											text_box($rule_prefix.'_value', $rule['value'], 30, 255);
										}
										// showing the asset finder?
										if ($attributes['show_asset_finder']) {
											echo ' Or<br />';
											asset_finder($rule_prefix.'_cq_id', $rule['comparison_question_id'], Array('form_question' => 'D'));
										}
									} else {
										// comparison question ID
										if ($cqid = $rule['comparison_question_id']) {
											$cq_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($cqid);
											echo 'Value of "'.$cq_asset->name.'" ['.$cq_asset->type().' : '.$cqid . ']';
										} else {
											echo $rule['value'];
										}
									}
								}?>
							</td>
						<?php
							if ($admin_access) {
								?><td align="center" width="100" class="sq-backend-table-cell"><?php
								check_box($rule_prefix.'_delete');
								?></td>
							<?php
						}
						?>
						</tr>
					<?php
					$i++;
					}//end foreach data
				}
				?>
				</table>
		<?php

		return true;

	}//end paintRules()
	
	
	/**
	* get the rules to process their parameters
	*
	* @param object Form_Section		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processRules(&$asset, &$o, $prefix)
	{
		if ($asset->getRuleCount() == 0) return false;

		$rules = &$asset->getRules();
		$prefix = $asset->getPrefix();

		// delete rules first
		foreach (array_keys($rules) as $k) {
			$rule_prefix = $prefix.'_r'.$k;
			if (isset($_POST[$rule_prefix.'_delete'])) {
				unset($rules[$k]);
			}
		}

		// modify what's left
		for (reset($rules); null !== ($k = key($rules)); next($rules)) {
			$rule = &$rules[$k];
			$type = 'form_question_rule_type_'.$rule['rule_code'];
			$rule_prefix = $prefix.'_r'.$k;

			if (isset($_POST[$rule_prefix.'_operator'])) {
				$rule['operator'] = $_POST[$rule_prefix.'_operator'];
			}

			if (isset($_POST[$rule_prefix.'_cq_id']) && ($_POST[$rule_prefix.'_cq_id']['assetid'] != 0)) {
				// comparison question id overrides the static value!
				if ($_POST[$rule_prefix.'_cq_id']['assetid'] == $asset->id) {
				  trigger_error('Cannot compare "'.$asset->name.'" (# '.$asset->id.') with itself in a rule',  E_USER_WARNING);
				  return false;
				}

				$rule['comparison_question_id'] = $_POST[$rule_prefix.'_cq_id']['assetid'];
				$rule['value'] = '';
			} else {
				// static value only specified - set c.q. ID to a duck
				$rule['comparison_question_id'] = 0;
				$rule['value'] = (isset($_POST[$rule_prefix.'_value'])) ? $_POST[$rule_prefix.'_value'] : '';
			}



		}
		$asset->setAttrValue('rules', $rules);
		return $asset->saveAttributes();

	}//end processRules()


	/**
	* Paint the dropdowns to add rules to this question
	*
	* @param object Form_Section		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean false if no questions in this section, true otherwise
	* @access public
	*/
	function paintAddRule(&$asset, &$o, $prefix)
	{
		if(!$asset->writeAccess('attributes')) return false;

		$rule_codes = $asset->getAllowedRules();
		$type_codes = Array();

		foreach	($rule_codes as $rule_code) {
			$type_code = 'form_question_rule_type_'.$rule_code;
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
			$attributes = eval('return '.$type_code.'::getAttributes();');
			if ($attributes['validation_rule']) $type_codes[] = $type_code;
		}

		$desc = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_codes, 'description');
		combo_box('rule', array_merge(Array(' ' => '------ Please Select ------'), $desc), false, '');
		
		return true;

	}//end paintAddRule()


	/**
	* Process the newly added questions
	*
	* @param object Form_Section		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddRule(&$asset, &$o, $prefix)
	{
		$type_code = $_POST['rule'];
		$rule_code = substr($type_code,24);
		if (!trim($rule_code)) return false;

		$rules = &$asset->getRules();
		
		$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
		$operators = array_keys(eval('return '.$type_code.'::getOperators();'));

		$rules[] = Array('rule_code' => $rule_code,
						 'operator' => $operators[0],
						 'value' => '',
						 'comparison_question_id' => 0);
						 
		$asset->setAttrValue('rules', $rules);
		return $asset->saveAttributes();

	}//end processAddRule()


	/**
	* Paint lock message telling the user to go to our parent to lock me
	*
	* @param object Form_Section		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean false if no questions in this section, true otherwise
	* @access public
	*/
	function paintLockMessage(&$asset, &$o, $prefix)
	{
		$section_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->_formid);

		$locks = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($section_asset->id, 'attributes', true);
		$lock  = $locks['attributes'];

		$full_write_access = $section_asset->writeAccess('');
		$acquire_lock_access = $section_asset->canLiveEdit('attributes');

		if (empty($lock)) {
			if ($full_write_access && $acquire_lock_access) {
				echo '<p class="sq-backend-unlocked">The "Attributes" lock to the '.(is_a($section_asset, 'form_section') ? 'Form Section' : 'Form').' is currently not held.</p>';
			} else {
				echo 'You do not have access to edit "'.$owner->name.'".';
			}

		} else {
			// this asset is currently locked so display message to the user
			$user    = &$GLOBALS['SQ_SYSTEM']->am->getAsset($lock['userid']);
			$editing = &$GLOBALS['SQ_SYSTEM']->am->getAsset($lock['source_assetid']);
	
			if (SQ_ROLLBACK_VIEW) $now = strtotime($_SESSION['sq_rollback_view']['rollback_time']);
			else $now = time();

			if (is_null($lock['expires'])) {
				$expires_in = 'The lock is being held indefinitely and will not expire. The lock must be released manually or be forceably acquired by a System Administrator.';
			} else {
				require_once SQ_FUDGE_PATH.'/general/datetime.inc';
				$expires_in = easy_time_total(($lock['expires'] - $now), true);
				if (!$expires_in) $expires_in = '1 second';
				$expires_in = 'The lock is due to expire in '.$expires_in;
			}

			?>
			<p class="sq-backend-locked-by-<?php echo ($GLOBALS['SQ_SYSTEM']->currentUser($user)) ? 'user' : 'someone-else'; ?>">
			The "Attributes" lock to the <?php echo is_a($section_asset, 'form_section') ? 'Form Section' : 'Form' ?> is held by user "<?php echo $user->name; ?>" at "<?php echo $editing->name; ?>".<br/>
			<?php echo $expires_in; ?>
			</p>
			<?php

		}//end if
		?>

		<p>Locking for this asset is controlled by its parent form<?php if (is_a($section_asset, 'form_section')) { echo ' section'; } ?>. Click to access this asset: <?php $href = $section_asset->getBackendHref(is_a($section_asset, 'form_section') ? 'details' : 'contents'); ?><a href="<?php echo $href?>"><?php echo $section_asset->_getName() ?></a></p><?php
		return true;

	}//end paintLockMessage()


	/**
	* Process lock message (no implementation)
	*
	* @param object Form_Section		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean always true
	* @access public
	*/
	function processLockMessage(&$asset, &$o, $prefix)
	{
		return true;

	}//end processLockMessage()


}//end class
?>
