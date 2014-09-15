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
* $Id: page_account_manager_edit_fns.inc,v 1.29 2013/09/05 03:11:38 akarelia Exp $
*
*/


require_once dirname(__FILE__).'/../../page_templates/page_asset_builder/page_asset_builder_edit_fns.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Page_Account_Manager_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.29 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Account_Manager_Edit_Fns extends Page_Asset_Builder_Edit_Fns
{

	/**
	* An array of asset types that can be passed to asset_finder to define what types of create locations are allowed
	* @var array(string => string)
	*/
	var $location_types = Array('user_group' => 'D');


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Paint the interface for choosing the user type that is created
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddCreateType(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) {
			parent::paintAssetTypes($asset, $o, $prefix);
		} else {
			$user_types = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('user');

			$options = Array('user' => 'User');
			foreach ($user_types as $type_code) {
				$info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
				if ($info['allowed_access'] != 'system' && $info['instantiable']) {
					$options[$type_code] = $info['name'];
				}
			}
			combo_box($prefix.'_create_type', $options, FALSE, array_keys($asset->attr('create_type')));
		}

	}//end paintAddCreateType()


	/**
	* Process the section for choosing the user type that is created
	*
	* This is overridden from Asset Builder because it makes sense for the account
	* manager to still be only allowed to create one type of user from the frontend interface.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddCreateType(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_create_type'])) {
			$type = trim($_POST[$prefix.'_create_type']);
			if (empty($type)) return FALSE;
			$types = Array($type => TRUE);
			$asset->setAttrValue('create_type', $types);
			return TRUE;
		}
		return FALSE;

	}//end processAddCreateType()


	/**
	* Paint post validation redirect
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintPostValidationRedirect(&$asset, &$o, $prefix)
	{
		return $this->paintNoticeLinkedAsset($asset, 'login_redirect');

	}//end paintPostValidationRedirect()


	/**
	* Process post validation redirect
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processPostValidationRedirect(&$asset, &$o, $prefix)
	{
		return $this->processNoticeLinkedAsset($asset, 'login_redirect');

	}//end processPostValidationRedirect()


	/**
	* Returns true if the account manager uses email validation
	*
	* @param object	&$asset	The asset that we are using for this interface
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access public
	*/
	function usingEmailValidation(&$asset, $prefix)
	{
		return $asset->attr('use_email_validation');

	}//end usingEmailValidation()


	/**
	* Paint the interface for the manual verification of accounts
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintValidateAccounts(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		// grab the list of users currently waiting validation
		$all_users = $asset->_getPendingUserIds();

		// pagination
		$total_assets = count($all_users);
		$num_per_page = 15;
		$num_pages    = ceil($total_assets / $num_per_page);

		$asset_offset  = array_get_index($_POST, $prefix.'_asset_offset', 0);
		hidden_field($prefix.'_asset_offset', $asset_offset);

		if ($num_pages <= 1) {
			$users = $all_users;
		} else {
			$users = array_slice($all_users, $asset_offset, $num_per_page);
		}

		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header-header" <?php echo ($write_access ? 'colspan="4"' : ''); ?>>
				<?php
					if ($asset_offset > 0) {
						?><a title="<?php echo translate('pagination_go_to_first') ?>" style="text-decoration: none; color: #fff" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_asset_offset', '0'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
					} else {
						?><span title="<?php echo translate('pagination_cannot_go_further_back') ?>" style="color: #333"><?php
					}
					?>
					&lt;&lt;
					<?php
					if ($asset_offset > 0) {
						?></a><?php
					} else {
						?></span><?php
					}
					?> &nbsp; &nbsp; <?php
					if ($asset_offset > 0) {
						?><a title="<?php echo translate('pagination_go_to_previous') ?>" style="text-decoration: none; color: #fff" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_asset_offset', '<?php echo $asset_offset - $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
					} else {
						?><span title="<?php echo translate('pagination_cannot_go_further_back') ?>" style="color: #333"><?php
					}
					?>
					&lt;
					<?php
					if ($asset_offset > 0) {
						?></a><?php
					} else {
						?></span><?php
					}
					?>
					&nbsp; &nbsp; <?php echo sprintf(translate('page_number'), round(($asset_offset + $num_per_page) / $num_per_page), ($total_assets > 0 ? ceil($total_assets / $num_per_page) : 1)); ?> &nbsp; &nbsp;
					<?php
					if (($asset_offset + $num_per_page) < $total_assets) {
						?><a title="<?php echo translate('pagination_go_to_next') ?>" style="text-decoration: none; color: #fff" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_asset_offset', '<?php echo $asset_offset + $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
					} else {
						?><span title="<?php echo translate('pagination_cannot_go_further_forward') ?>" style="color: #333"><?php
					}
					?>
					&gt;
					<?php
					if (($asset_offset + $num_per_page) < $total_assets) {
						?></a><?php
					} else {
						?></span><?php
					}
					?> &nbsp; &nbsp; <?php
					if (($asset_offset + $num_per_page) < $total_assets) {
						?><a title="<?php echo translate('pagination_go_to_last') ?>" style="text-decoration: none; color: #fff" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_asset_offset', '<?php echo ($num_pages - 1) * $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
					} else {
						?><span title="<?php echo translate('pagination_cannot_go_further_forward') ?>" style="color: #333"><?php
					}
					?>
					&gt;&gt;
					<?php
					if (($asset_offset + $num_per_page) < $total_assets) {
						?></a><?php
					} else {
						?></span><?php
					}
				?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-header"><?php echo translate('user'); ?></td>
				<?php
					if ($write_access) {
						?><td class="sq-backend-table-header"><?php echo translate('cms_account_manager_validate'); ?></td><?php
						?><td class="sq-backend-table-header"><?php echo translate('delete_question'); ?></td><?php
					}
				?>
			</tr>
		<?php

		$user_details = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($users, Array(), TRUE, 'name');
		foreach ($users as $userid) {
			?><tr>
				<td class="sq-backend-table-cell"><?php echo get_asset_tag_line($userid, 'details'); ?></td>
				<?php
					if ($write_access) {
						// javascript to uncheck the other option when one is checked
						$name = $prefix.'_validate_action['.$userid.']';
						?><td width="10%" class="sq-backend-table-cell" style="text-align: center;"><?php
							check_box($name, 'validate', FALSE, 'document.getElementById(\''.$prefix.'_delete_'.$userid.'\').checked=0', 'id="'.$prefix.'_validate_'.$userid.'"');
						?></td><?php
						?><td width="10%" class="sq-backend-table-cell" style="text-align: center;"><?php
							check_box($name, 'delete', FALSE, 'document.getElementById(\''.$prefix.'_validate_'.$userid.'\').checked=0', 'id="'.$prefix.'_delete_'.$userid.'"');
						?></td><?php
					}
				?>

			</tr><?php
		}

		?>
		</table>
		<?php


		return $write_access;

	}//end paintValidateAccounts()


	/**
	* Process the interface for the manual verification of accounts
	*
	* @param object	&$asset	the asset whose interface we are processing
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processValidateAccounts(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		if ($write_access) {
			$pending_users = array_get_index($_REQUEST, $prefix.'_validate_action', Array());

			$user_actions = Array();
			foreach ($pending_users as $userid => $action) {
				if (empty($action)) continue;
				$user_actions[$action][] = $userid;
			}

			// validate some users
			if (!empty($user_actions['validate'])) {
				// queue up a hipo to loop through the users, validating each one
				quick_hipo($asset->id, 'hipoValidateUser', $user_actions['validate'], Array(), TRUE);
			}

			// delete some users
			if (!empty($user_actions['delete'])) {
				// queue up a hipo to loop through the users, deleting them
				quick_hipo($asset->id, 'hipoDeleteUser', $user_actions['delete'], Array(), TRUE);
			}
		}
		return $write_access;

	}//end processValidateAccounts()


	/**
	* Paints the interface for choosing create types whose form should be customised
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintCreateFormCustomisation(&$asset, &$o, $prefix)
	{
		$customised_types = $asset->attr('create_form_customisations');
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			hidden_field($prefix.'_customise_form_submitted', 1);
			echo check_box($prefix.'_customise_form', 1, !empty($customised_types));
			label(translate('cms_account_manager_customise_create_form'), $prefix.'_customise_form');
		} else {
			$fn = !empty($customised_types) ? 'tick' : 'cross';
			echo '<img src="'.sq_web_path('lib').'/web/images/'.$fn.'.gif" />';
			echo translate('cms_account_manager_customise_create_form');
		}

	}//end paintCreateFormCustomisation()


	/**
	* Processes the interface for choosing create types whose form should be customised
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processCreateFormCustomisation(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes') && isset($_POST[$prefix.'_customise_form_submitted'])) {
			$customise = array_get_index($_POST, $prefix.'_customise_form');
			$customised_types = $customise ? array_keys($asset->attr('create_type')) : Array();
			return $asset->setAttrValue('create_form_customisations', $customised_types);
		}
		return FALSE;

	}//end processCreateFormCustomisation()


	/**
	* Paints the interface for Use Email Validation option
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintUseEmailValidation(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('links');
		$current_value = $asset->attr('use_email_validation');
		$options = Array(1 => 'Yes', 0 => 'No');

		if ($write_access) {
			combo_box($prefix.'_use_email_validation', $options, FALSE,  $current_value);
		} else {
			echo $options[$current_value];
		}

		return $write_access;

	}//end paintEmailValidation()


	/**
	* Process the interface for Use Email Validation option
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processUseEmailValidation(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_use_email_validation'])) {

			$use_email_validation = $_POST[$prefix.'_use_email_validation'];
			if ($asset->setAttrValue('use_email_validation', $use_email_validation) && $use_email_validation) {

				// if we are using email validation - create validation assets
				$pending_accounts_groupid = $asset->_getPendingAccountsGroupId();
				if (empty($pending_accounts_groupid)) {
					$asset->_createPendingAccountsGroup();
				}
			}

		}

		return TRUE;

	}//end processEmailValidation()


	/**
	* Paints the interface for choosing create types whose form should be customised
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintLoggedInFormCustomisation(&$asset, &$o, $prefix)
	{
		$customised_types = $asset->attr('customise_logged_in');
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			hidden_field($prefix.'_customise_logged_in_submitted', 1);
			echo check_box($prefix.'_customise_logged_in', 1, !empty($customised_types));
			label(translate('cms_account_manager_customise_logged_in'), $prefix.'_customise_logged_in');
		} else {
			$fn = !empty($customised_types) ? 'tick' : 'cross';
			echo '<img src="'.sq_web_path('lib').'/web/images/'.$fn.'.gif" />';
			echo translate('cms_account_manager_customise_logged_in');
		}

	}//end paintCreateFormCustomisation()


	/**
	* Processes the interface for choosing create types whose form should be customised
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processLoggedInFormCustomisation(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes') && isset($_POST[$prefix.'_customise_logged_in_submitted'])) {
			$customise = array_get_index($_POST, $prefix.'_customise_logged_in');
			$customised_types = $customise ? array_keys($asset->attr('create_type')) : Array();
			return $asset->setAttrValue('customise_logged_in', $customised_types);
		}
		return FALSE;

	}//end processCreateFormCustomisation()


}//end class

?>
