<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: page_account_manager_edit_fns.inc,v 1.19 2006/04/10 05:00:28 rhoward Exp $
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
* @version $Revision: 1.19 $
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
	function Page_Account_Manager_Edit_Fns()
	{
		$this->Page_Asset_Builder_Edit_Fns();

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
		$num_per_page = 5;
		$num_pages    = ceil($total_assets / $num_per_page);

		$asset_offset  = array_get_index($_POST, $prefix.'_asset_offset', 0);
		hidden_field($prefix.'_asset_offset', $asset_offset);

		if ($num_pages <= 1) {
			$users = $all_users;
		} else {
			$users = array_slice($all_users, $asset_offset, $asset_offset + $num_per_page);
		}

		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header-header" <?php echo ($write_access ? 'colspan="2"' : ''); ?>>
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
					&nbsp; &nbsp; <?php echo translate('page_number', round(($asset_offset + $num_per_page) / $num_per_page), ($total_assets > 0 ? ceil($total_assets / $num_per_page) : 1)); ?> &nbsp; &nbsp;
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
					}
				?>
			</tr>
		<?php

		$user_details = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($users, Array(), TRUE, 'name');
		foreach ($users as $userid) {
			?><tr>
				<td class="sq-backend-table-cell"><?php $this->_paintPendingUserDetails($userid, $user_details); ?></td>
				<?php
					if ($write_access) {
						?><td width="10%" class="sq-backend-table-cell"><?php check_box($prefix.'_validate['.$userid.']'); ?></td><?php
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
	* Paints the line used to represent the user in the Pending Accounts screen
	*
	* @param string	$userid			the id of the user to paint
	* @param string	$user_details	an array of userid => name
	*
	* @return boolean
	* @access private
	*/
	function _paintPendingUserDetails($userid, $user_details)
	{
		echo $user_details[$userid].' (#'.$userid.')';
		return TRUE;

	}//end _paintPendingUserDetails()


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
			$selected_users = array_get_index($_REQUEST, $prefix.'_validate', Array());
			if (!empty($selected_users)) {
				// queue up a hipo to loop through the users, validating each one
				quick_hipo($asset->id, 'hipoValidateUser', array_keys($selected_users), Array(), TRUE);
			}
		}
		return $write_access;

	}//end processValidateAccounts()


}//end class

?>