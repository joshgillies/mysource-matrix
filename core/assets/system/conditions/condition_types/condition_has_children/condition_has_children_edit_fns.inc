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
* $Id: condition_has_children_edit_fns.inc,v 1.2 2013/09/02 01:39:07 ewang Exp $
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
* @version $Revision: 1.2 $
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
		foreach ($type_list as $index => $asset_type) {
		    $type_list[$index] = trim($asset_type);
		}
		?>
		<table class="sq-backend-table">
		<?php
		if ($write_access) {
			?>
			<tr>
				<th style="width: 100px;"><?php echo translate('asset_type'); ?></th>
				<td><?php
					asset_type_chooser($prefix.'[type_list]', TRUE, $type_list, NULL, FALSE, FALSE, Array('asset'));
				?></td>
			</tr>
			<?php
		} else {
			?>
			<tr>
				<th style="width: 100px;"><?php echo translate('asset_type'); ?></th>
				<td>
				<?php
				if (empty($type_list)) {
					echo '<em>'.translate('condition_no_asset_types_selected').'</em>';
				} else {
					$type_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_list, 'name');
					echo '<ul class="sq-asset-id-list">';
					foreach ($type_list as $type) {
						echo '<li>';
							echo get_asset_type_icon($type);
							echo '<strong>'.$type_names[$type].'</strong>';
						echo '</li>';
					}
					echo '</ul>';
				}
				?>
				</td>
			</tr>
			<?php
		}

		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$link_types_names = get_link_type_names();
		$link_types = array_get_index($condition_data, 'link_types', '');

		$link_type_list = Array();
		if (!empty($link_types)) {
		    $link_type_list = explode(',', $link_types);
		    foreach ($link_type_list as $index => $link_type_code) {
			$link_type_list[$index] = trim($link_type_code);
		    }
		}
		else {
		    $link_type_list = Array('1', '2');
		}

		
		if ($write_access) {
			?>
			<tr>
				<th><?php echo translate('link_type'); ?></th>
				<td><?php
					combo_box($prefix.'[link_types]', $link_types_names, TRUE, $link_type_list);
				?></td>
			</tr>
			<?php
		} else {
			?>
			<tr>
				<th><?php echo translate('link_type') ?></th>
				<td>
				<?php
				if (empty($link_type_list)) {
					echo '<em>'.translate('condition_no_link_types_selected').'</em>';
				} else {
					echo '<ul class="sq-no-vertical-margin">';
					foreach ($link_type_list as $link_type) {
						echo '<li>'.$link_types_names[$link_type].'</li>';
					}
					echo '</ul>';
				}
				?>
				</td>
			</tr>
			<?php
		}
		?></table><?php
		
		
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
		
		$link_types = Array('1', '2');
		if (isset($_POST[$prefix]['link_types']) && !empty($_POST[$prefix]['link_types'])) {
			$link_types = $_POST[$prefix]['link_types'];
		}

		// construct and return data required by this condition
		// as found in the post data, array required consists of
		// 'match' and 'condition_data'
		$results = Array(
					'types'	=> trim(implode(',', $_POST[$prefix]['type_list']),','),
					'link_types'	=> trim(implode(',', $link_types),','),
				   );

		return $results;

	}//end processEditInterface()


}//end class

?>