<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: authentication_ipb_edit_fns.inc,v 1.6 2012/08/30 00:58:55 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/authentication/authentication/authentication_edit_fns.inc';

/**
* Authentication_IPB_Edit_Fns
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage ipb
*/
class Authentication_IPB_Edit_Fns extends Authentication_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Authentication_IPB_Edit_Fns()
	{
		$this->Authentication_Edit_Fns();
		unset($this->static_screens['settings']);
		unset($this->static_screens['preview']);
		unset($this->static_screens['lookupValues']);
		unset($this->static_screens['metadata']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['workflow']);
		unset($this->static_screens['tagging']);
		unset($this->static_screens['dependants']);
		unset($this->static_screens['linking']);
		unset($this->static_screens['roles']);
		unset($this->static_screens['permissions']);
		unset($this->static_screens['layouts']);

	}//end constructor


	/**
	* Paint the interface for choosing the banned list user options
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAllowBannedList(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$allow_banned = $asset->attr('allow_banned');
		if ($write_access) {
			?>
			<script type="text/javascript">
				<!--
				function chooseOne(caller_id, target_id) {
					caller_checkbox = document.getElementById(caller_id).checked = true;
					target_checkbox = document.getElementById(target_id).checked = false;
				}
				-->
			</script>
			<?php
			$o->openField(translate('Allow Banned Users'));

				echo 'Yes';
				$onclick = 'chooseOne(\''.$prefix.'_allow_banned_user\', \''.$prefix.'_disallow_banned_user\')';
				check_box($prefix.'_allow_banned_user', '1', $allow_banned, $onclick);
				echo '&nbsp;No';
				$onclick = 'chooseOne(\''.$prefix.'_disallow_banned_user\', \''.$prefix.'_allow_banned_user\' )';
				check_box($prefix.'_disallow_banned_user', '1', !$allow_banned, $onclick);
			$o->closeField();

		} else {
			$o->openField(translate('Allow Banned Users'));

			if ($allow_banned) {
				echo '<b>'.translate('Yes').'</b>';

			} else {
				echo '<b>'.translate('No').'</b>';

			}
			$o->closeField();
		}

		return TRUE;

	}//end paintAllowBannedList()


	/**
	* Process the interface for choosing the banned list user options
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAllowBannedList(&$asset, &$o, $prefix)
	{
		if (!empty($_POST[$prefix.'_allow_banned_user'])) {
			$asset->setAttrValue('allow_banned', TRUE);
		}
		if (!empty($_POST[$prefix.'_disallow_banned_user'])) {
			$asset->setAttrValue('allow_banned', FALSE);
		}
		return TRUE;

	}//end processAllowBannedList()


}//end class

?>