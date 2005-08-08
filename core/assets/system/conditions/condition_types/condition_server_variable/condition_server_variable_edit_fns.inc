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
* $Id: condition_server_variable_edit_fns.inc,v 1.1.2.2 2005/08/08 07:16:59 dmckee Exp $
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
* @version $Revision: 1.1.2.2 $
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
	* @return void
	* @access public
	*/
	function paintEditInterface(&$asset, &$o, $prefix, &$restriction, $write_access)
	{
		$options = $asset->getOptions();

		if (!isset($restriction['condition_data'])) {
			$restriction['condition_data'] = Array();
		}

		if (!$write_access) {
			$o->openField(translate('condition_details'));
				$o->note(translate('condition_server_variable', array_get_index($restriction['condition_data'], 'server_variable', ''), $options['match_options'][array_get_index($restriction, 'match', 1)], array_get_index($restriction['condition_data'], 'server_variable_match', '')));
			$o->closeField();
		} else {
			$o->openField(translate('match_condition'));
				combo_box($prefix.'[match]', $options['match_options'], false, $restriction['match']);
			$o->closeField();

			$o->openField(translate('server_variable'));
				text_box($prefix.'[server_variable]', array_get_index($restriction['condition_data'], 'server_variable', ''), 30);
			$o->closeField();

			$o->openField(translate('condition'));
				text_box($prefix.'[server_variable_match]', array_get_index($restriction['condition_data'], 'server_variable_match'), 30);
			$o->closeField();
		}
		$o->sectionNote(translate('condition_server_variable_is_regular_expression_enabled'));
		return true;

	}//end paintEditInterface()


	/**
	* Process the backend editing interface for this condition
	*
	* @return Array
	* @access public
	*/
	function processEditInterface(&$asset, &$o, $prefix, &$restriction, $write_access)
	{
		// Construct condition_data from Edit Interface
		// Condition needs 'match', 'server_variable' and 'server_variable_match'
		$results = Array(
					'condition_data'	=> Array(
											'server_variable'		=> $_POST[$prefix]['server_variable'],
											'server_variable_match'	=> $_POST[$prefix]['server_variable_match'],
										   ),
					'match'				=> $_POST[$prefix]['match'],
				   );
		return $results;

	}//end processEditInterface()


}//end class

?>