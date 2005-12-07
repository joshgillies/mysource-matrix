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
* $Id: condition_keyword_regexp_edit_fns.inc,v 1.1 2005/12/07 05:11:02 lwright Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition_edit_fns.inc';
require_once SQ_ATTRIBUTES_PATH.'/selection/selection.inc';

/**
* Condition_Keyword_Regexp_Edit_Fns
*
* Edit functions for the keyword regular expression condition
*
* @see Asset_Edit_Fns
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_Keyword_Regexp_Edit_Fns extends Condition_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::Asset_Edit_Fns
	*/
	function Condition_Keyword_Regexp_Edit_Fns()
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
	function paintEditInterface(&$condition_data, &$o, $prefix, $write_access, $allowed_keywords)
	{
		if (empty($condition_data)) {
			$condition_data =  Array('keyword' => '', 'operator' => 'ALL');
		}
		?>
		<table style="width: auto" class="no-borders">
			<tr>
				<td><strong><?php echo translate('keyword'); ?></strong> </td>
				<td>
					<?php
					$selected_keyword = $condition_data['keyword'];

					$keyword_options = Array();
					foreach ($allowed_keywords as $keyword) {
						$keyword_options[$keyword] = $keyword;
					}

					$i = 0;
					if (!empty($allowed_keywords)) {

						$keyword_info = $this->_getKeywordInfo($selected_keyword, $allowed_keywords);
						if ($write_access) {
							combo_box($prefix.'[keyword]', array_merge($keyword_options), false, $keyword_info['constant_part']);
							// This is a special case for generic/variable keywords (like asset_metadata_)
							// which name must be completed.
							// For those keywords, we show an additional box to type the rest of the name
							if ($keyword_info['is_variable']) {
								text_box($prefix.'[keyword_extension]', $keyword_info['variable_part'], 30);
							}
						} else {
							echo ($keyword_info['constant_part']).array_get_index($keyword_info, 'variable_part', '');
						}
					} else {
						echo(translate('core_keyword_list_empty'));
					}
					?>
				</td>
			</tr>
			<tr>
				<td><strong><?php echo translate('pattern'); ?></strong> </td>
				<td>
					<?php
					if ($write_access) {
						text_box($prefix.'[keyword_match]', array_get_index($condition_data, 'keyword_match'), 30);
					} else {
						echo array_get_index($condition_data, 'keyword_match', '');
					}
					?>
				</td>
			</tr>
		</table>
		<?php // echo translate('condition_server_variable_is_regular_expression_enabled'); ?>
		<?php

	}//end paintEditInterface()


	/**
	* Process the backend editing interface for this condition
	*
	* @param object		&$o		Backend outputter to use
	* @param string		$prefix	Prefix for form elements
	*
	* @return array
	* @access public
	*/
	function processEditInterface(&$o, $prefix)
	{
		$results = Array(
					'keyword'		=> $_POST[$prefix]['keyword'],
					'keyword_match'	=> $_POST[$prefix]['keyword_match'],
				   );

		if (!empty($_POST[$prefix]['keyword_extension'])) {
			$name_extension = trim($_POST[$prefix]['keyword_extension']);
			if (!empty($name_extension)) {
				$results['keyword'] .= $name_extension;
			}
		}

		return $results;

	}//end processEditInterface()


	/**
	* Returns an array containing information about a keyword name
	* We consider a keyword variable if it contains a part that is defined by
	* the user. Example: asset_metadata_XXX.
	*
	* constant part= asset_metadata_
	* variable part= XXX
	*
	* Array(
	* 		'is_variable' => boolean,
	* 		'constant_part' => string,
	* 		'variable_part' => string,
	* 		)
	*
	* @param string			$keyword_name		the keyword to analyse
	* @param Array(string)	$allowed_keywords	allowed keywords, this is the
	* list of keywords as returned by getAssetKeywords().
	*
	* @return Array
	* @access private
	*/
	function _getKeywordInfo($keyword_name, &$allowed_keywords)
	{
		$ret = Array('is_variable' => false, 'constant_part' => $keyword_name, 'variable_part' => '');

		$keyword_parts = Array();
		foreach ($allowed_keywords as $allowed_keyword) {
			if (preg_match('/_$/', $allowed_keyword)) {
				// $allowed_keyword is a variable keyword
				if (preg_match('/^' . $allowed_keyword . '(.*)$/', $keyword_name, $keyword_parts)) {
					// $keyword_name is a variable keyword
					$ret = Array('is_variable' => true, 'constant_part' => $allowed_keyword, 'variable_part' => $keyword_parts[1]);
				}
			}
		}

		return $ret;

	}


}//end class

?>