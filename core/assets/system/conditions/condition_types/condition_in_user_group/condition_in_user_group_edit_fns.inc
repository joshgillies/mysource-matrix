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
* $Id: condition_in_user_group_edit_fns.inc,v 1.9 2012/08/30 01:09:10 ewang Exp $
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
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_In_User_Group_Edit_Fns extends Condition_Edit_Fns
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
	* @param Backend_Outputter	&$o					Backend outputter to use
	* @param string				$prefix				Prefix for form elements
	* @param boolean			$write_access		Whether we have access to edit the field values
	*
	* @return void
	* @access public
	*/
	public static function paintEditInterface(Array $condition_data, Backend_Outputter $o, $prefix, $write_access)
	{
		?>
		<table class="sq-backend-table">
			<tr>
				<th class="sq-conditions-options-th"><?php echo translate('User Groups'); ?></th>

				<td>
					<?php
						if ($write_access) {
							multiple_asset_finder($prefix.'[groupids]', array_get_index($condition_data, 'groupids', Array()), Array('user_group' => 'D'));
						} else {
							if (empty($condition_data['groupids'])) {
								echo '<em>'.translate('No user groups selected.').'</em>';

							} else {
								$names = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($condition_data['groupids'], 'user_group', FALSE, 'name');
								?>
								<ul class="sq-asset-id-list">
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
	* @param Backend_Outputter	$o		Backend outputter to use
	* @param string				$prefix	Prefix for form elements
	*
	* @return array
	* @access public
	*/
	public static function processEditInterface(Backend_Outputter $o, $prefix)
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