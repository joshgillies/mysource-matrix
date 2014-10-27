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
* $Id: condition_keyword_regexp_edit_fns.inc,v 1.11 2012/08/30 01:09:10 ewang Exp $
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
* @version $Revision: 1.11 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_Keyword_Regexp_Edit_Fns extends Condition_Edit_Fns
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
	* @param array				$condition_data	Array of settings for this condition
	* @param Backend_Outputter	&$o					Backend outputter to use
	* @param string				$prefix				Prefix for form elements
	* @param boolean			$write_access		Whether we have access to edit the field values
	* @param array				$allowed_keywords	Allowed keywords fromg getAssetKeywords()
	*
	* @return void
	* @access public
	*/
	public static function paintEditInterface(Array $condition_data, Backend_Outputter $o, $prefix, $write_access, Array $allowed_keywords)
	{
		if (empty($condition_data)) {
			$condition_data =  Array('keyword' => '', 'operator' => 'ALL');
		}
		?>
		<table class="sq-backend-table">
			<tr>
				<th class="sq-conditions-options-th"><?php echo translate('Keyword'); ?></th>

				<td>
					<?php
					$selected_keyword = '';
					if (isset($condition_data['keyword'])) {
					   $selected_keyword = $condition_data['keyword'];
					}

					// add the Custom Keyword option to the head of array
					array_unshift($allowed_keywords, 'Custom Keyword');

					if (empty($allowed_keywords)) {
						// we don't know the type of the asset here so we add the keywords of the generic Asset object
						$dummy_asset = new Asset();
						$allowed_keywords = array_keys($dummy_asset->getAvailableKeywords());
					}

					$keyword_options = Array();
					$allowed_keywords = array_merge($allowed_keywords, array_keys(get_available_global_keywords()));
					foreach ($allowed_keywords as $keyword) {
						$keyword_options[$keyword] = $keyword;
					}

					ksort($keyword_options);

					$i = 0;
					if (!empty($allowed_keywords)) {
						if(strpos($selected_keyword, 'Custom Keyword') === 0) {
							$selected_keyword = '';
						}
						$keyword_info = self::_getKeywordInfo($selected_keyword, $allowed_keywords);
						if ($write_access) {
							echo '<span class="sq-inline-fields-wrapper">%';
							if(self::_isCustomKeyword($selected_keyword, $allowed_keywords)) {
								text_box($prefix.'[keyword]', $selected_keyword, 80);
							}
							else {
								combo_box($prefix.'[keyword]', array_merge($keyword_options), FALSE, $keyword_info['constant_part']);

								// This is a special case for generic/variable keywords (like asset_metadata_)
								// which name must be completed.
								// For those keywords, we show an additional box to type the rest of the name
								if ($keyword_info['is_variable']) {
									text_box($prefix.'[keyword_extension]', $keyword_info['variable_part'], 50);
								}
							}
							echo '%</span>';
						} else {
							echo '%'.($keyword_info['constant_part']).array_get_index($keyword_info, 'variable_part', '').'%';
						}
					} else {
						echo(translate('The keyword list is empty.'));

					}
					?>
				</td>
			</tr>
			<tr>
				<th><?php echo translate('Pattern'); ?></th>

				<td>
					<?php
					if ($write_access) {
						text_box($prefix.'[keyword_match]', array_get_index($condition_data, 'keyword_match'), 80, '', '', 'style="margin-bottom: 5px;"');
						echo '</br>';
						check_box($prefix.'[replace_keyword_in_match]', '1', array_get_index($condition_data, 'replace_keyword_in_match', '0'), '', ($write_access ? '' : 'disabled="disabled"'));
						label(translate('Replace keywords in pattern'), $prefix.'[replace_keyword_in_match]');

					} else {
						$current_pattern = array_get_index($condition_data, 'keyword_match', '');
						if (strlen($current_pattern)){
							echo $current_pattern;
						} else {
							echo '<em>'.translate('No pattern entered.').'</em>';

						}
						echo '</br>';
						check_box($prefix.'[replace_keyword_in_match]', '1', array_get_index($condition_data, 'replace_keyword_in_match', '0'), '', ($write_access ? '' : 'disabled="disabled"'));
						label(translate('Replace keywords in pattern'), $prefix.'[replace_keyword_in_match]');

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
	* @param Backend_Outputter	$o		Backend outputter to use
	* @param string				$prefix	Prefix for form elements
	*
	* @return array
	* @access public
	*/
	public static function processEditInterface(Backend_Outputter $o, $prefix)
	{
		$results = Array(
					'keyword'					=> isset($_POST[$prefix]['keyword']) ? $_POST[$prefix]['keyword'] : '',
					'keyword_match'				=> $_POST[$prefix]['keyword_match'],
					'replace_keyword_in_match'	=> array_get_index($_POST[$prefix], 'replace_keyword_in_match', '0'),
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
	*
	* We consider a keyword variable if it contains a part that is defined by
	* the user.
	* Example: asset_metadata_XXX -
	*    constant part= asset_metadata_
	*    variable part= XXX
	* Array(
	* 		'is_variable' => boolean,
	* 		'constant_part' => string,
	* 		'variable_part' => string,
	* 		)
	*
	* @param string	$keyword_name		the keyword to analyse
	* @param array	&$allowed_keywords	allowed keywords, this is the list of keywords
	*									as returned by getAssetKeywords().
	*
	* @return array
	* @access private
	*/
	protected static function _getKeywordInfo($keyword_name, Array &$allowed_keywords)
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



	/**
	* Check if a keyword name is a custom keyword ( not in allowed list)
	*
	* @param string	$keyword_name		the keyword to analyse
	* @param array	&$allowed_keywords	allowed keywords, this is the list of keywords
	*									as returned by getAssetKeywords().
	*
	* @return boolean
	* @access protected
	*/
	protected static function _isCustomKeyword($keyword_name, Array &$allowed_keywords)
	{
		$ret = TRUE;

		$keyword_parts = Array();
		foreach ($allowed_keywords as $allowed_keyword) {
				if (preg_match('/^'.$allowed_keyword.'(.*)$/', $keyword_name, $keyword_parts)) {
					if(preg_match('/_$/', $allowed_keyword) || $allowed_keyword === $keyword_name) {
						// there is a match, so it's not a custom keyword
						$ret = FALSE;
						return $ret;
					}
				}
		}

		return $ret;

	}//end _isCustomKeyword()

}//end class

?>
