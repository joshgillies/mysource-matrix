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
* $Id: condition_has_children_edit_fns.inc,v 1.1 2013/08/29 06:48:33 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition_edit_fns.inc';

/**
* Condition_Has_Children_Edit_Fns
*
* Edit functions for the has children condition
*
* @see Asset_Edit_Fns
*
* Based on by condition_asset_type
* @author Anh Ta <ata@squiz.co.uk>
* @author  Keith Tait <keith.tait@ed.ac.uk>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_Has_Children_Edit_Fns extends Condition_Edit_Fns
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
		$asset_types = array_get_index($condition_data, 'types', '');

		$type_list = Array();
		if (!empty($asset_types)) {
			$type_list = explode(',', $asset_types);
		}

		if ($write_access) {
			?><table style="width: auto" border="0" class="no-borders">
				<tr>
					<td><?php
						asset_type_chooser($prefix.'[type_list]', TRUE, $type_list, NULL, FALSE, FALSE, Array('asset'));
					?></td>
				</tr>
			</table><?php
		} elseif (!empty($type_list)) {
			$type_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_list, 'name');
			?><table class="sq-backend-table">
				<tr>
					<th><?php echo translate('type') ?></th>
				</tr><?php
				foreach ($type_list as $type) {
					?><tr>
						<td><?php
							echo get_asset_type_icon($type);
							echo $type_names[$type];
						?></td>
					</tr><?php
				}
			?></table><?php
		}

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
		if (!isset($_POST[$prefix]['type_list']) || empty($_POST[$prefix]['type_list'])) {
			return Array();
		}

		// construct and return data required by this condition
		// as found in the post data, array required consists of
		// 'match' and 'condition_data'
		$results = Array(
					'types'	=> trim(implode(',', $_POST[$prefix]['type_list']),','),
				   );

		return $results;

	}//end processEditInterface()


}//end class

?>