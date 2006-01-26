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
* $Id: condition_logical_edit_fns.inc,v 1.5 2006/01/26 22:34:05 lwright Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition_edit_fns.inc';

/**
* Condition_Logical_Edit_Fns
*
* Edit functions for the logical condition
*
* @see Condition_Edit_Fns
*
* @author  Geoffroy Noel <gnoel@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_Logical_Edit_Fns extends Condition_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::Asset_Edit_Fns
	*/
	function Condition_Logical_Edit_Fns()
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
	* @param array		$allowed_keywords	Allowed keywords to use
	*
	* @return void
	* @access public
	*/
	function paintEditInterface(&$condition_data, &$o, $prefix, $write_access, $allowed_keywords)
	{
		if (empty($condition_data)) {
			$condition_data =  Array('logical_keywords' => Array('logical_keyword_1' => ''), 'operator' => 'ALL');
		}
		?>
		<table class="no-borders">
			<?php
			$selected_keywords = array_keys($condition_data['logical_keywords']);
			$selected_keywords['-- none --'] = '-- '.strtolower(translate('none')).' --';

			$keyword_options = Array();
			foreach ($allowed_keywords as $keyword) {
				$keyword_options[$keyword] = $keyword;
			}

			$i = 0;
			if (!empty($allowed_keywords)) {
				foreach ($selected_keywords as $keyword) {
					$special_option = Array();
					if ($i == (count($selected_keywords) - 1)) {
						$special_option = Array('none' => '-- '.strtolower(translate('none')).' --');
					} else if ($i > 0) {
						$special_option = Array('none' => '-- '.strtolower(translate('remove')).' --');
					}

					$keyword_info = $this->_getKeywordInfo($keyword, $allowed_keywords);
					?>
					<tr>
						<td><strong><?php echo translate('logical_keyword').' '.$i; ?></strong>&nbsp;</td>
						<td>
							<?php
							if ($write_access) {
								combo_box($prefix.'[keyword'.$i.']', array_merge($special_option, $keyword_options), FALSE, $keyword_info['constant_part']);
								// This is a special case for generic/variable keywords (like asset_metadata_)
								// which name must be completed.
								// For those keywords, we show an additional box to type the rest of the name
								if ($keyword_info['is_variable']) {
									text_box($prefix.'[name_extension_'.$i.']', $keyword_info['variable_part'], 30);
								}
							} else {
								echo ($keyword_info['constant_part']).array_get_index($keyword_info, 'variable_part', '');
							}
							?>
						</td>
					</tr>
					<?php
					$i++;
				}
			} else {
				echo(translate('core_keyword_list_empty'));
			}
			?>
			<tr>
				<td><strong><?php echo translate('operator'); ?></strong></td>
				<td>
					<?php
					if ($write_access) {
						combo_box($prefix.'[operator]', Array('ALL' => 'AND', 'ANY' => 'OR'), FALSE, $condition_data['operator']);
					} else {
						echo $condition_data['operator'];
					}
					?>
				</td>
			</tr>
		</table>
		Keywords are considered TRUE if their value is non-empty and nonzero.  Keywords are considered FALSE if their value is empty or zero.
		<?php

	}//end paintEditInterface()


	/**
	* Process the backend editing interface for this condition
	*
	* @param object	&$o					Backend outputter to use
	* @param string	$prefix				Prefix for form elements
	* @param array	$allowed_keywords	Allowed keywords to use
	*
	* @return array
	* @access public
	*/
	function processEditInterface(&$o, $prefix, $allowed_keywords)
	{
		$logical_keywords = Array();

		for ($i = 0; $i < 100; $i++) {
			if (isset($_POST[$prefix]['keyword'.$i])) {
				$keyword_name = $_POST[$prefix]['keyword'.$i];
				if ($keyword_name != 'none') {
					$keyword_info = $this->_getKeywordInfo($keyword_name, $allowed_keywords);
					if ($keyword_info['is_variable']) {
						if (!empty($_POST[$prefix]['name_extension_'.$i])) {
							$name_extension = trim($_POST[$prefix]['name_extension_'.$i]);
							if (!empty($name_extension)) {
								$keyword_name = $keyword_info['constant_part'].$name_extension;
							}
						}
					}
					$logical_keywords[$keyword_name] = '';
				}
			}
		}

		if (empty($logical_keywords)) {
			$logical_keywords = Array('logical_keyword_1' => '');
		}

		$results = Array(
					'logical_keywords'	=> $logical_keywords,
					'operator'			=> $_POST[$prefix]['operator'],
				   );

		return $results;

	}//end processEditInterface()


	/**
	* Returns an array containing information about a keyword name
	*
	* We consider a keyword variable if it contains a part that is defined by
	* the user.
	* Example: asset_metadata_XXX -
	*	constant part= asset_metadata_
	*	variable part= XXX
	* Array(
	* 		'is_variable' => boolean,
	* 		'constant_part' => string,
	* 		'variable_part' => string,
	* 		)
	*
	* @param string	$keyword_name		the keyword to analyse
	* @param array	&$allowed_keywords	allowed keywords, this is the list of
	*									keywords as returned by getAssetKeywords().
	*
	* @return array
	* @access private
	*/
	function _getKeywordInfo($keyword_name, &$allowed_keywords)
	{
		$ret = Array('is_variable' => FALSE, 'constant_part' => $keyword_name, 'variable_part' => '');

		$keyword_parts = Array();
		foreach ($allowed_keywords as $allowed_keyword) {
			if (preg_match('/_$/', $allowed_keyword)) {
				// $allowed_keyword is a variable keyword
				if (preg_match('/^'.$allowed_keyword.'(.*)$/', $keyword_name, $keyword_parts)) {
					// $keyword_name is a variable keyword
					$ret = Array('is_variable' => TRUE, 'constant_part' => $allowed_keyword, 'variable_part' => $keyword_parts[1]);
				}
			}
		}

		return $ret;

	}//end _getKeywordInfo()


}//end class

?>