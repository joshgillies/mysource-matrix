<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: condition_asset_status_edit_fns.inc,v 1.2 2007/09/21 04:38:37 lwright Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition_edit_fns.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Condition_Asset_Status_Edit_Fns
*
* Edit functions for the write access conditional
*
* @see Asset_Edit_Fns
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_Asset_Status_Edit_Fns extends Condition_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::Asset_Edit_Fns
	*/
	function Condition_Asset_Status_Edit_Fns()
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

		foreach (get_constant_values('SQ_STATUS_') as $status_value) {
			$statii[$status_value] = get_status_description($status_value);
		}//end foreach

		if (!isset($condition_data['status'])) {
			$selected_statii = Array();
		} else {

			if (!is_array($condition_data['status'])) {
				$selected_statii = explode(',', $condition_data['status']);

				foreach ($selected_statii as $key => $status) {
					if (!is_numeric($status)) {
						$selected_statii[$key] = get_status_value(trim($status));
					}//end if

				}//end foreach

			} else {
				$selected_statii = $condition_data['status'];
			}//end if

		}



		?>
		<table style="width: auto" class="no-borders">
			<tr>
				<td>
					<?php
					if ($write_access) {
						// text_box($prefix.'[status]', array_get_index($condition_data, 'status', ''), 30);
						combo_box($prefix.'[status]', $statii, TRUE, $selected_statii);
					} else {
						foreach ($selected_statii as $status) {
								echo get_status_description($status).'<br />';
						}//end foreach
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
		if (!isset($_POST[$prefix])) return Array();

		$results = Array(
					'status'		=> implode(',', $_POST[$prefix]['status']),
				   );
		return $results;

	}//end processEditInterface()


}//end class

?>