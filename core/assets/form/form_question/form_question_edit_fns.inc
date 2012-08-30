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
* $Id: form_question_edit_fns.inc,v 1.10 2012/08/30 01:09:07 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Form_Question_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage __core__
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
		$this->static_screens = Array(
									'details'	=> Array(
													'name'			=> 'Details',
													'force_unlock'	=> FALSE,
													'lock_type'		=> 'none',
												   ),
								);

	}//end constructor


	/**
	* Paint the current rules
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintRules(&$asset, &$o, $prefix)
	{
		if ($asset->getRuleCount() == 0) {
			$o->openRaw();
			echo translate('core_form_no_rules_set');
			$o->closeRaw();
			return FALSE;
		}

		$rule_codes = $asset->getAllowedRules();
		$type_codes = Array();
		$write_access = $asset->writeAccess('attributes');

		foreach ($rule_codes as $rule_code) {
			$type_codes[] = 'form_question_rule_type_'.$rule_code;
		}

		$desc = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_codes, 'description');

		$rules = $asset->getRules();

		for (reset($rules); NULL !== ($k = key($rules)); next($rules)) {
			$rule = $rules[$k];
			// let the rule paint itself here!!
			$type_code = 'form_question_rule_type_'.$rule['rule_code'];
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
			$rule_asset = new $type_code();
			$edit_fns = $rule_asset->getEditFns();

			$o->openSection($desc[$type_code]);
			$edit_fns->paintRule($rule_asset, $o, $prefix.'_rule_'.$k, $rule, $write_access, $this);

			if ($write_access) {
				$o->openField(translate('core_form_custom_error_text'));
				text_box($prefix.'_rule_'.$k.'[custom_text]', array_get_index($rule, 'custom_text', ''), 30);
				$o->closeField();

				$o->openField(translate('delete_question'));
				check_box($prefix.'_rule_'.$k.'[delete]');
				$o->closeField();
			} else {
				$o->openField(translate('core_form_custom_error_text'));
				$custom_text = array_get_index($rule, 'custom_text', '');
				if (empty($custom_text)) {
					?><i><?php echo translate('core_form_no_custom_error_text_set') ?></i><?php
				} else {
					echo $rule['custom_text'];
				}
				$o->closeField();
			}

			$o->closeSection();
		}//end for

		return $write_access;

	}//end paintRules()


	/**
	* get the rules to process their parameters
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processRules(&$asset, &$o, $prefix)
	{
		if ($asset->getRuleCount() == 0) return FALSE;

		if (!$asset->writeAccess('attributes')) return FALSE;

		$rule_codes = $asset->getAllowedRules();
		$type_codes = Array();

		foreach ($rule_codes as $rule_code) {
			$type_codes[] = 'form_question_rule_type_'.$rule_code;
		}

		$desc = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_codes, 'description');

		$rules = $asset->getRules();

		// delete rules first
		foreach (array_keys($rules) as $k) {
			$rule_prefix = $prefix.'_rule_'.$k;
			if (!isset($_POST[$rule_prefix])) continue;
			if (array_get_index($_POST[$rule_prefix], 'delete', 0)) {
				unset($rules[$k]);
			}
		}

		// modify what's left
		for (reset($rules); NULL !== ($k = key($rules)); next($rules)) {
			$rule =& $rules[$k];
			$rule_prefix = $prefix.'_rule_'.$k;
			// let the rule paint itself here!!
			$type_code = 'form_question_rule_type_'.$rule['rule_code'];
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
			$rule_asset = new $type_code();
			$edit_fns = $rule_asset->getEditFns();

			$edit_fns->processRule($rule_asset, $o, $prefix.'_rule_'.$k, $rule);

			// set custom text
			if (isset($_POST[$rule_prefix]['custom_text'])) {
				$rule['custom_text'] = $_POST[$rule_prefix]['custom_text'];
			}
		}

		$asset->setAttrValue('rules', $rules);
		return $asset->saveAttributes();

	}//end processRules()


	/**
	* Paint the dropdowns to add rules to this question
	*
	* Retruns false if no questions in this section, true otherwise
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddRule(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		$rule_codes = $asset->getAllowedRules();
		$type_codes = Array();

		foreach ($rule_codes as $rule_code) {
			$type_code = 'form_question_rule_type_'.$rule_code;
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
			$val_rule = eval('return '.$type_code.'::isValidationRule();');
			if ($val_rule) $type_codes[] = $type_code;
		}

		$desc = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_codes, 'description');
		combo_box('rule', array_merge(Array(' ' => '------ '.translate('please_select').' ------'), $desc), FALSE, '');

		return TRUE;

	}//end paintAddRule()


	/**
	* Process the newly added questions
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddRule(&$asset, &$o, $prefix)
	{
		$type_code = $_POST['rule'];
		$rule_code = substr($type_code,strlen('form_question_rule_type_'));
		if (!trim($rule_code)) return FALSE;

		$rules = $asset->getRules();
		$rules[] = Array('rule_code' => $rule_code);

		$asset->setAttrValue('rules', $rules);
		return $asset->saveAttributes();

	}//end processAddRule()


	/**
	* Paint lock message telling the user to go to our parent to lock me
	*
	* Returns false if no questions in this section, true otherwise
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintLockMessage(&$asset, &$o, $prefix)
	{
		$section_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->_formid);

		$locks = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($section_asset->id, 'attributes', TRUE);
		$lock  = $locks['attributes'];

		$full_write_access = $section_asset->writeAccess('');
		$acquire_lock_access = $section_asset->canLiveEdit('attributes');

		if (empty($lock)) {
			if ($full_write_access && $acquire_lock_access) {
				echo '<p class="sq-backend-unlocked">The "Attributes" lock to the '.(($section_asset instanceof Form_Section) ? 'Form Section' : 'Form').' is not currently held.</p>';
			} else {
				echo 'You do not have access to edit "'.$section_asset->name.'".';
			}

		} else {
			// this asset is currently locked so display message to the user
			$user    = $GLOBALS['SQ_SYSTEM']->am->getAsset($lock['userid']);
			$editing = $GLOBALS['SQ_SYSTEM']->am->getAsset($lock['source_assetid']);

			if (SQ_ROLLBACK_VIEW) {
				$now = strtotime($_SESSION['sq_rollback_view']['rollback_time']);
			} else {
				$now = time();
			}

			if (is_null($lock['expires'])) {
				$expires_in = 'The lock is being held indefinitely and will not expire. The lock must be released manually or be forceably acquired by a System Administrator.';
			} else {
				require_once SQ_FUDGE_PATH.'/general/datetime.inc';
				$expires_in = easy_time_total(($lock['expires'] - $now), TRUE);
				if (!$expires_in) $expires_in = '1 second';
				$expires_in = 'The lock is due to expire in '.$expires_in;
			}

			?>
			<p class="sq-backend-locked-by-<?php echo ($GLOBALS['SQ_SYSTEM']->currentUser($user)) ? 'user' : 'someone-else'; ?>">
			The "Attributes" lock to the <?php echo ($section_asset instanceof Form_Section) ? 'Form Section' : 'Form' ?> is held by user "<?php echo htmlspecialchars($user->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET); ?>" at "<?php echo htmlspecialchars($editing->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET); ?>".<br/>
			<?php echo $expires_in; ?>
			</p>
			<?php

		}//end if
		?>

		<p>Locking for this asset is controlled by its parent form<?php
			if ($section_asset instanceof Form_Section) {
				echo ' section';
			}
		?>. Click to access this asset: <?php $href = $section_asset->getBackendHref(!($section_asset instanceof Form) ? 'details' : 'contents'); ?><a href="<?php echo $href ?>"><?php echo $section_asset->_getName() ?></a></p><?php
		return TRUE;

	}//end paintLockMessage()


	/**
	* Process lock message (no implementation)
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processLockMessage(&$asset, &$o, $prefix)
	{
		return TRUE;

	}//end processLockMessage()


}//end class
?>
