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
* $Id: condition_user_frequency_edit_fns.inc,v 1.5 2006/01/26 22:34:05 lwright Exp $
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
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_User_Frequency_Edit_Fns extends Condition_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::Asset_Edit_Fns
	*/
	function Condition_User_Frequency_Edit_Fns()
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
		$options = Array(
					'hits'		=> 'Hits',
					'session'	=> 'Session',
					'days'		=> 'Days',
					'hours'		=> 'Hours',
					'minutes'	=> 'Minutes',
				   );
		?>
		<table>
		<tr>
			<td><strong>Value</strong></td>
			<td>
				<?php
				if ($write_access) {
					text_box($prefix.'[value]', $condition_data['value'], 10);
				} else {
					echo $condition_data['value'];
				}
				?>
			</td>
		</tr>
		<tr>
			<td><strong>Units</strong></td>
			<td>
				<?php
				if ($write_access) {
					combo_box($prefix.'[units]', $options, 0, $condition_data['units']);
				} else {
					echo $options[$condition_data['units']];
				}
				?>
			</td>
		</tr>
		</table>
		<?php
		echo translate('condition_user_frequency_note');

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
		$results = Array(
					'units'	=> $_REQUEST[$prefix]['units'],
					'value'	=> $_REQUEST[$prefix]['value'],
				   );

		return $results;

	}//end processEditInterface()


}//end class

?>