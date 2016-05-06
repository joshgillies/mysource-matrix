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
* $Id: condition_logical_edit_fns.inc,v 1.12 2012/08/30 01:09:10 ewang Exp $
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
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_Logical_Edit_Fns extends Condition_Edit_Fns
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
	* @param array				$allowed_keywords	Allowed keywords to use
	*
	* @return void
	* @access public
	*/
	public static function paintEditInterface(Array $condition_data, Backend_Outputter $o, $prefix, $write_access, Array $allowed_keywords)
    {
        if (empty($condition_data)) {
			$condition_data =  Array('logical_keywords' => Array('logical_keyword_1' => ''), 'operator' => 'ALL');
        }

		if (empty($allowed_keywords)) {
		    // we don't know the type of the asset here so we add the keywords of the generic Asset object
		    $dummy_asset = new Asset();
		    $allowed_keywords =     array_keys($dummy_asset->getAvailableKeywords());
		}
		$allowed_keywords = array_merge($allowed_keywords, array_keys(get_available_global_keywords()));

		// add the Custom Keyword option to the head of array
		array_push($allowed_keywords, 'Custom Keyword');

		?>
		<table class="sq-backend-table">
			<?php
			$selected_keywords = array();
			if (isset($condition_data['logical_keywords'])) {
                $selected_keywords = array_keys($condition_data['logical_keywords']);
			}
			if ($write_access) {
			    $selected_keywords['-- none --'] = '-- '.strtolower(translate('None')).' --';

			}

			$keyword_options = Array();
			foreach ($allowed_keywords as $keyword) {
				$keyword_options[$keyword] = $keyword;
			}

			$i = 0;
			?>

			<tr>
				<th><?php echo translate('Logical Grouping'); ?></th>

				<td>
					<?php
					$operator = '';
					if(isset($condition_data['operator'])) {
					   $operator = $condition_data['operator'];
					}
					if ($write_access) {
						combo_box($prefix.'[operator]', Array('ALL' => sprintf(translate('Match on %s of the following keywords'),strtoupper(translate('All'))), 'ANY' => sprintf(translate('Match on %s of the following keywords'),strtoupper(translate('Any')))), FALSE, $operator);


					} else {
						echo sprintf(translate('Match on %s of the following keywords'), $operator);

					}
					?>
				</td>
			</tr>

			<?php
			if (!empty($allowed_keywords)) {
				foreach ($selected_keywords as $keyword) {
					$special_option = Array();
					if ($i == (count($selected_keywords) - 1)) {
						$special_option = Array('none' => '-- '.strtolower(translate('select a keyword to add it')).' --');

					} else if ($i > 0) {
						$special_option = Array('none' => '-- '.strtolower(translate('remove this keyword')).' --');

					}

					$keyword_info = self::_getKeywordInfo($keyword, $allowed_keywords);
					?>
					<tr>
						<th class="sq-conditions-options-th"><?php echo translate('Keyword').' '.($i+1); ?></th>

						<td>
							<?php
							echo '<span class="sq-inline-fields-wrapper">%';
							if ($write_access) {
								if(self::_isCustomKeyword($keyword, $allowed_keywords)) {
									if(strpos($keyword, 'Custom Keyword') === 0) {
										$keyword = '';
									}
									text_box($prefix.'[keyword'.$i.']', $keyword, 80);
								}
								else {
									combo_box($prefix.'[keyword'.$i.']', array_merge($special_option, $keyword_options), FALSE, $keyword_info['constant_part']);
									// This is a special case for generic/variable keywords (like asset_metadata_)
									// which name must be completed.
									// For those keywords, we show an additional box to type the rest of the name
									if ($keyword_info['is_variable']) {
										text_box($prefix.'[name_extension_'.$i.']', $keyword_info['variable_part'], 30);
									}
								}
							} else {
								echo ($keyword_info['constant_part']).array_get_index($keyword_info, 'variable_part', '');
							}
							echo '%</span>';
							?>
						</td>
					</tr>
					<?php
					$i++;
				}
			} else {
				echo(translate('The keyword list is empty.'));

			}
			?>
		</table>
		<?php
		$o->note(translate('Keywords are considered TRUE if their value is NOT empty or NOT zero. Keywords are considered FALSE if their value is empty or zero.'));


	}//end paintEditInterface()


	/**
	* Process the backend editing interface for this condition
	*
	* @param Backend_Outputter	$o					Backend outputter to use
	* @param string				$prefix				Prefix for form elements
	* @param array				$allowed_keywords	Allowed keywords to use
	*
	* @return array
	* @access public
	*/
	public static function processEditInterface(Backend_Outputter $o, $prefix)
	{
		$j = count(preg_grep("/keyword[0-9]/", array_keys($_POST[$prefix])));
		$logical_keywords = Array();

		for ($i = 0; $i < $j; $i++) {
			if (isset($_POST[$prefix]['keyword'.$i])) {
				$keyword_name = isset($_POST[$prefix]['keyword'.$i]) ? $_POST[$prefix]['keyword'.$i] : '';
				if ($keyword_name != 'none' && !empty($keyword_name)) {
						if (!empty($_POST[$prefix]['name_extension_'.$i])) {
							$name_extension = trim($_POST[$prefix]['name_extension_'.$i]);
							if (!empty($name_extension)) {
								$keyword_name .= $name_extension;
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

		// if the option value is -- None -- ignore it
		if(strpos($keyword_name, '-- ') === 0) {
			return FALSE;
		}
		if(empty($keyword_name)) {
			return FALSE;
		}		
		if(strpos($keyword_name, 'Custom Keyword') === 0) {
			return TRUE;
		}


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