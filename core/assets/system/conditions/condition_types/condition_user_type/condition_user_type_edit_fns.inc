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
* $Id: condition_user_type_edit_fns.inc,v 1.1 2006/12/17 23:59:41 bcaldwell Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition_edit_fns.inc';

/**
* Condition_User_type_Edit_Fns
*
* Edit functions for the user type condition
*
* @see Asset_Edit_Fns
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_User_type_Edit_Fns extends Condition_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::Asset_Edit_Fns
	*/
	function Condition_User_type_Edit_Fns()
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
		<table style="width: auto" border="0" class="no-borders">
			<tr>
				<td><?php echo translate('condition_user_type_list'); ?>: </td>
				<td>
					<?php

					$user_types = array_get_index($condition_data, 'user_types', '');

					if ($write_access) {
						text_box($prefix.'[type_list]', $user_types, 30);
					} else if (!empty($user_types)) {
						echo '<ul><li>';
						echo implode('</li><li>', $user_types);
						echo '</li></ul>';
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
		// construct and return data required by this condition
		// as found in the post data, array required consists of
		// 'match' and 'condition_data'
		$results = Array(
					'user_types'	=> $_POST[$prefix]['type_list'],
				   );

		return $results;

	}//end processEditInterface()


}//end class

?>