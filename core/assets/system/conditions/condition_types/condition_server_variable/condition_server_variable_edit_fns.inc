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
* $Id: condition_server_variable_edit_fns.inc,v 1.7 2012/08/30 01:09:10 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition_edit_fns.inc';
require_once SQ_ATTRIBUTES_PATH.'/selection/selection.inc';

/**
* Condition_Server_Variable_Edit_Fns
*
* Edit functions for the user agent condition
*
* @see Asset_Edit_Fns
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_Server_Variable_Edit_Fns extends Condition_Edit_Fns
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
	* @param Backend_Outputter	$o					Backend outputter to use
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
				<th class="sq-conditions-options-th"><?php echo translate('server_variable'); ?></th>
				<td>
					<?php
					if ($write_access) {
						text_box($prefix.'[server_variable]', array_get_index($condition_data, 'server_variable', ''), 50);
					} else {
						$current_variable = array_get_index($condition_data, 'server_variable', '');
						if (strlen($current_variable)){
							echo $current_variable;
						} else {
							echo '<em>'.translate('condition_no_variable_entered').'</em>';
						}
					}
					?>
				</td>
			</tr>
			<tr>
				<th><?php echo translate('pattern'); ?></th>
				<td>
					<?php
					if ($write_access) {
						text_box($prefix.'[server_variable_match]', array_get_index($condition_data, 'server_variable_match'), 80);
					} else {
						$current_pattern = array_get_index($condition_data, 'server_variable_match', '');
						if (strlen($current_pattern)){
							echo $current_pattern;
						} else {
							echo '<em>'.translate('condition_no_pattern_entered').'</em>';
						}
					}
					?>
				</td>
			</tr>
		</table>
		<?php
		$o->note(translate('condition_server_variable_is_regular_expression_enabled'));

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
		$results = Array(
					'server_variable'		=> $_POST[$prefix]['server_variable'],
					'server_variable_match'	=> $_POST[$prefix]['server_variable_match'],
				   );
		return $results;

	}//end processEditInterface()


}//end class

?>