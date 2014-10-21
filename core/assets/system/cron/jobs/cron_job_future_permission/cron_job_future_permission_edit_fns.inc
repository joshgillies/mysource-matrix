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
* $Id: cron_job_future_permission_edit_fns.inc,v 1.4 2012/08/30 01:09:11 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job_edit_fns.inc';

/**
* Cron_Job_Future_Permission_Edit_Fns
*
* Purpose
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Future_Permission_Edit_Fns extends Cron_Job_Edit_Fns
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
	* Prints the tasks interface in a one line format
	*
	* @param Cron_Job_Future_Permission	$asset					The owning asset
	* @param Backend_Outputter		$o						the Backend Outputter object
	* @param string					$prefix					prefix for form elements
	* @param string					$when					prepared when string
	* @param string					$cascade				cascade the permission change
	* @param string					$form_extra_elements	extra form elements
	* @param array					$exclude_permission			exclude these permissions from the list
	*
	* @return boolean
	* @access public
	*/
	public function paintInLineBackend(Cron_Job_Future_Permission $asset, Backend_Outputter $o, $prefix, $when='', $cascade_to_new=FALSE, $dependants_only=TRUE, $form_extra_elements='', Array $exclude_permission=Array())
	{
		$descs = $asset->getAllPermissionDescs();
		foreach ($exclude_permission as $permission) {
			unset($descs[$permission]);
		}

		$write_access = $asset->writeAccess('attributes');
		echo '<span class="sq-inline-fields-wrapper">';
		if ($write_access) {
			combo_box($prefix.'_add_future_permissions', Array('1' => translate('Add'), '0' => translate('Remove')), FALSE, '1');


			echo '&nbsp;'.translate('the following').'&nbsp;';

			combo_box($prefix.'_future_permission', $descs, FALSE, $asset->attr('permission'), NULL, $form_extra_elements);
		} else {
			echo $asset->attr('Add')? translate('Add') : translate('Remove');


			echo $descs[$asset->attr('permission')];
		}
		echo '&nbsp;'.translate('Permission').'</span><br /><br />';


		if ($write_access) {
				?>
				<table class="sq-simple-table">
					<tr>
						<td colspan="2">
							<span class="sq-inline-fields-wrapper" style="margin-top: 3px;">
							<?php
							combo_box($prefix.'_future_permissions_access[public][granted]', Array('' => '', '1' => translate('Grant'), '0' => translate('Deny')), FALSE, '');


							echo ' '.translate('Public Permission');

							?>
							</span>&nbsp;
						</td>
					</tr>
					<tr>
						<td valign="top">
							<span class="sq-inline-fields-wrapper" style="margin-top: 3px;">
							<?php
							combo_box($prefix.'_future_permissions_access[user][granted]', Array('' => '', '1' => translate('Grant'), '0' => translate('Deny')), FALSE, '');


							echo ' '.translate('Specific Permission');

							?>
							</span>&nbsp;
						</td>
						<td><?php multiple_asset_finder($prefix.'_future_permissions_access[user][userid]', Array(), Array('user' => 'D', 'user_group' => 'D')); ?></td>
					</tr>
				</table>
				<?php

			echo '<br /><span class="sq-inline-fields-wrapper">'.translate('At').'&nbsp;';

			$this->_paintWhenBox($asset, $o, $prefix, $when, $form_extra_elements);
			echo '</span>';

			echo '<br /><br />';
			check_box($prefix.'_cascade_to_new', '1', $cascade_to_new);
			label(translate('Automatically cascade this permission to newly-created children'), $prefix.'_cascade_to_new');


			echo '<br />';
			check_box($prefix.'_cascade_permission_changes', '1', !$dependants_only);
			label(translate('Update this permission to all existing children that fall under this asset in the various hierarchies'), $prefix.'_cascade_permission_changes', '1');

		}//end if

		return TRUE;

	}//end paintInLineBackend()


	/**
	* Process the tasks interface
	*
	* @param Cron_Job_Future_Permission	$asset	The owning asset
	* @param Backend_Outputter		$o		the Backend Outputter object
	* @param string					$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function processInLineBackend(Cron_Job_Future_Permission $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) {
			return FALSE;
		}

		if (!empty($_POST[$prefix.'_future_permission']) && $asset->setAttrValue('permission', $_POST[$prefix.'_future_permission'])) {
			$add_permissions = (!isset($_POST[$prefix.'_add_future_permissions']) || $_POST[$prefix.'_add_future_permissions']) ? TRUE : FALSE;
			if (!$asset->setAttrValue('add', $add_permissions)) {
				return FALSE;
			}

			$userids = Array();
			$permission_access = $_POST[$prefix.'_future_permissions_access'];
			$permission_set = FALSE; //FLAG TO MARK IF PERMISSION IS GRANTED OR DENIED TO AT LEAST ONE USER OR NOT
			if (isset($permission_access['public']['granted']) && ($permission_access['public']['granted'] != '')) {
				$public_userid = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');
				$userids[$public_userid] = (int)$permission_access['public']['granted'];
				$permission_set = TRUE;
			}
			if (isset($permission_access['user']['granted']) && ($permission_access['user']['granted'] != '')) {
				foreach ($permission_access['user']['userid'] as $entry) {
					if (!empty($entry['assetid'])) {
						$userids[$entry['assetid']] = (int)$permission_access['user']['granted'];
						$permission_set = TRUE;
					}
				}
			}
			if (!$permission_set || !$asset->setAttrValue('userids', $userids)) {
				return FALSE;
			}

			$cascade_to_new = (isset($_POST[$prefix.'_cascade_to_new']) && $_POST[$prefix.'_cascade_to_new']) ? TRUE : FALSE;
			$dependants_only = (isset($_POST[$prefix.'_cascade_permission_changes']) && $_POST[$prefix.'_cascade_permission_changes']) ? FALSE : TRUE;
			if (!$asset->setAttrValue('cascade_to_new', $cascade_to_new) || !$asset->setAttrValue('dependants_only', $dependants_only)) {
				return FALSE;
			}

			if ($when = $this->_processWhenBox($asset, $o, $prefix)) {
				return ($asset->setAttrValue('when', $when));
			} else {
				trigger_localised_error('CRON0058', translate('To add a future permission you must specify the date and time at which the permission should be changed'), E_USER_NOTICE);
			}
		}

		return FALSE;

	}//end processInLineBackend()


}//end class

?>
