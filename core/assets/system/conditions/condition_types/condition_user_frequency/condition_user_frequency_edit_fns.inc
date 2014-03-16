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
* $Id: condition_user_frequency_edit_fns.inc,v 1.10 2012/08/30 01:09:10 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition_edit_fns.inc';

/**
* Condition_User_Frequency_Edit_Fns
*
* Edit functions for the user Frequency condition
*
* @see Asset_Edit_Fns
*
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_User_Frequency_Edit_Fns extends Condition_Edit_Fns
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
		$value = array_get_index($condition_data, 'value', 0);
		$units = array_get_index($condition_data, 'units', 'hits');
		$options = Array(
					'hits'		=> 'Hits',
					'session'	=> 'Session',
					'days'		=> 'Days',
					'hours'		=> 'Hours',
					'minutes'	=> 'Minutes',
				   );
		?>
		<table class="sq-backend-table">
			<tr>
				<th class="sq-conditions-options-th">Value</th>
				<td>
					<?php
					if ($write_access) {
						text_box($prefix.'[value]', $value, 10);
					} else {
						if (strlen($value)){
							echo $value;
						} else {
							echo '<em>'.translate('condition_no_value_entered').'</em>';
						}
					}
					?>
				</td>
			</tr>
			<tr>
				<th><strong>Units</th>
				<td>
					<?php
					if ($write_access) {
						combo_box($prefix.'[units]', $options, 0, $units);
					} else {
						echo $options[$units];
					}
					?>
				</td>
			</tr>
		</table>
		<?php
		$o->note(translate('condition_user_frequency_note'));

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
					'units'	=> $_REQUEST[$prefix]['units'],
					'value'	=> $_REQUEST[$prefix]['value'],
				   );

		return $results;

	}//end processEditInterface()


}//end class

?>
