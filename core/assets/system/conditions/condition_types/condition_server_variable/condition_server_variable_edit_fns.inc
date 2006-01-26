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
* $Id: condition_server_variable_edit_fns.inc,v 1.3 2006/01/26 22:34:05 lwright Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition_edit_fns.inc';
require_once SQ_ATTRIBUTES_PATH.'/selection/selection.inc';

/**
* Condition_server_variable_Edit_Fns
*
* Edit functions for the user agent condition
*
* @see Asset_Edit_Fns
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class condition_server_variable_Edit_Fns extends Condition_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::Asset_Edit_Fns
	*/
	function condition_server_variable_Edit_Fns()
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
		<table style="width: auto" class="no-borders">
			<tr>
				<td><?php echo translate('server_variable'); ?>: </td>
				<td>
					<?php
					if ($write_access) {
						text_box($prefix.'[server_variable]', array_get_index($condition_data, 'server_variable', ''), 30);
					} else {
						echo array_get_index($condition_data, 'server_variable', '');
					}
					?>
				</td>
			</tr>
			<tr>
				<td><?php echo translate('pattern'); ?>: </td>
				<td>
					<?php
					if ($write_access) {
						text_box($prefix.'[server_variable_match]', array_get_index($condition_data, 'server_variable_match'), 30);
					} else {
						echo array_get_index($condition_data, 'server_variable_match', '');
					}
					?>
				</td>
			</tr>
		</table>
		<?php echo translate('condition_server_variable_is_regular_expression_enabled'); ?>
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
		$results = Array(
					'server_variable'		=> $_POST[$prefix]['server_variable'],
					'server_variable_match'	=> $_POST[$prefix]['server_variable_match'],
				   );
		return $results;

	}//end processEditInterface()


}//end class

?>