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
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Regular_Expresion_Edit_Fns
*
* Purpose
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Regular_Expression_Edit_Fns extends Asset_Edit_Fns
{

    /**
    * Constructor
    *
    */
    function __construct()
    {
		$this->_ser_attrs = TRUE;
        parent::__construct();

    }//end constructor


	/**
	* Paint the regex input data
	*
	* @param Asset              $asset  the asset whose interface we are painting
	* @param Backend_Outputter  $o      the Backend Outputter object
	* @param string             $prefix prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintRegexDataForm(Asset $asset, Backend_Outputter $o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');

		$regex_data = $asset->attr('regex_data');
		$prefix = $asset->getPrefix();
		echo '<table class ="sq-backend-table">';
		echo '<tr><th>Regex</th><th>Replacement</th>'.($wa ? '<th>Delete</th>' : '').'</tr>';
		// Existing regex pairs
		$count = 0;
		foreach($regex_data as $data) {
			echo '<tr>';
			
			echo '<td>';
			text_box($prefix.'_regex['.$count.'][regex]', $data['regex'], 40, '', FALSE, $wa ? '' : 'disabled');
			echo '</td>';
			
			echo '<td>';
			text_box($prefix.'_regex['.$count.'][replacement]', $data['replacement'], 30, '', FALSE, $wa ? '' : 'disabled');
			echo '</td>';

			if ($wa) {
				echo '<td>';
				check_box($prefix.'_regex['.$count.'][delete]');
				echo '</td>';
			}
			echo '</tr>';
			$count++;
		}

		// New regex pair
		if ($wa) {
			echo '<tr>';
			echo '<td>';
			text_box($prefix.'_regex['.$count.'][regex]', '', 40);
			echo '</td>';
			echo '<td>';
			text_box($prefix.'_regex['.$count.'][replacement]', '', 30);
			echo '</td>';
			echo '<td></td>';
			echo '</tr>';
		}

		echo '</table>';
		
		return TRUE;

	}//end paintReplaceFrom()


	/**
	* Process the regex input data
	*
	* @param Asset              $asset  the asset whose interface we are painting
	* @param Backend_Outputter  $o      the Backend Outputter object
	* @param string             $prefix prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processRegexDataForm(Asset $asset, Backend_Outputter $o, $prefix)
	{
		$regex_data = Array();;

		if (!$asset->writeAccess('attributes')) {
			return FALSE;
		}
		
		$prefix = $asset->getPrefix();
		if(!empty($_POST[$prefix.'_regex'])) {
			foreach($_POST[$prefix.'_regex'] as $data) {
				if (isset($data['delete'])) {
					continue;
				}

				$regex = trim(array_get_index($data, 'regex', ''));
				$replacement = trim(array_get_index($data, 'replacement', ''));
				if (empty($regex)) continue;

				// Strip out Matrix keyword for the testing
				$test_regex = str_replace('\/', '/', preg_replace('/%[^\s]+%/', '', str_replace('/', '\/', $regex)));
				if (preg_match($test_regex, '') === FALSE) {
					// Warn user if invalid regex string
					trigger_error('Regular expression "'.$regex.'" is not valid. It will be ignored when applying the keyword modifier', E_USER_WARNING);
				}
				// Warn if 'e' regex modifier is used
				$regex_delimiter = substr($test_regex, 0, 1);
				$ending_delimiter_index = strrpos($test_regex, $regex_delimiter);
				if ($ending_delimiter_index > 0) {
					// Get the modifier part in the regex
					$regex_modifier_str = substr($test_regex, $ending_delimiter_index);
					if (strpos($regex_modifier_str, 'e') !== FALSE) {
						trigger_error('"e" (PREG_REPLACE_EVAL) modifier is not allowed. This modifier will be ignored when applying the keyword modifier', E_USER_WARNING);
					}
				}

				$regex_data[] = Array(
									'regex'			=> $regex,
									'replacement'	=> $replacement,
								);
			}//end foreach

			return $asset->setAttrValue('regex_data', $regex_data);
		}//end if

		return TRUE;

	}//end processRegexDataForm()


}//end class
?>
