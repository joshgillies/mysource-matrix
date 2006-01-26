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
* $Id: condition_in_user_group_edit_fns.inc,v 1.5 2006/01/26 22:34:05 lwright Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition_edit_fns.inc';

/**
* Condition_In_User_Group_Edit_Fns
*
* Edit functions for the in user group conditional
*
* @see Asset_Edit_Fns
*
* @author  Blair Robertson <brobertson@squiz.co.uk>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_In_User_Group_Edit_Fns extends Condition_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::Asset_Edit_Fns
	*/
	function Condition_In_User_Group_Edit_Fns()
	{
		$this->Condition_Edit_Fns();

	}//end constructor


	/**
	* Paint the backend editing interface for this condition
	*
	* @param array		&$condition_data	Array of settings for this condition
	* @param object		&$o					Backend outputter to use
	* @param string		$prefix				Prefix for form elements
	* @param boolean	$write_access		Whether we have access to edit the field values
	*
	* @return void
	* @access public
	*/
	function paintEditInterface(&$condition_data, &$o, $prefix, $write_access)
	{
		?>
		<table class="no-borders">
			<tr>
				<td valign="top"><strong><?php echo translate('user_groups'); ?></strong>&nbsp;</td>
				<td valign="top">
					<?php
						if ($write_access) {
							multiple_asset_finder($prefix.'[groupids]', array_get_index($condition_data, 'groupids', Array()), Array('user_group' => 'D'));
						} else {
							if (empty($condition_data['groupids'])) {
								echo translate('no_user_groups_selected');
							} else {
								$names = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($condition_data['groupids'], 'user_group', FALSE, 'name');
								?>
								<ul>
								<?php
								foreach ($condition_data['groupids'] as $id) {
									?>
									<li><?php echo get_asset_tag_line($id); ?></li>
									<?php
								}
								?>
								</ul>
								<?php
							}
						}
					?>
				</td>
			</tr>
		</table>
		<?php

	}//end paintEditInterface()


	/**
	* Process the backend editing interface for this condition
	*
	* @param object	&$o		Backend outputter to use
	* @param string	$prefix	Prefix for form elements
	*
	* @return array
	* @access public
	*/
	function processEditInterface(&$o, $prefix)
	{
		if (empty($_POST[$prefix])) return Array();

		$groupids = Array();
		foreach ($_POST[$prefix]['groupids'] as $data) {
			if (empty($data['assetid'])) continue;
			$groupids[] = $data['assetid'];
		}

		return Array('groupids' => $groupids);

	}//end processEditInterface()


}//end class
?>