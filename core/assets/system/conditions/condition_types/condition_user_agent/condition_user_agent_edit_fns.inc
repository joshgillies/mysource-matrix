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
* $Id: condition_user_agent_edit_fns.inc,v 1.8.16.1 2011/08/17 06:32:46 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition_edit_fns.inc';

/**
* Condition_User_Agent_Edit_Fns
*
* Edit functions for the user agent condition
*
* @see Asset_Edit_Fns
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.8.16.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_User_Agent_Edit_Fns extends Condition_Edit_Fns
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
		<table style="width: auto" border="0" class="no-borders">
			<tr>
				<td><?php echo translate('user_agent_pattern'); ?>: </td>
				<td>
					<?php
					if ($write_access) {
						text_box($prefix.'[compare]', array_get_index($condition_data, 'user_agent', ''), 30);
					} else {
						echo array_get_index($condition_data, 'user_agent', '');
					}
					?>
				</td>
			</tr>
		</table>
		<?php echo translate('condition_user_agent_is_regular_expression_enabled'); ?>
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
		// construct and return data required by this condition
		// as found in the post data, array required consists of
		// 'match' and 'condition_data'
		$results = Array(
					'user_agent'	=> $_POST[$prefix]['compare'],
				   );

		return $results;

	}//end processEditInterface()


}//end class

?>