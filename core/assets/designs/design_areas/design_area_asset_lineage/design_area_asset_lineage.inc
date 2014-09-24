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
* $Id: design_area_asset_lineage.inc,v 1.28 2013/03/19 04:41:07 cupreti Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Asset_Lineage
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.28 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Asset_Lineage extends Design_Area
{


	/**
	* Constructor
	*
	* @param string	$assetid	The asset ID to load
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Outputs the var either as straight HTML or PHP code, depending on the type
	*
	* @param string	$var			the name of the var to print out
	* @param array	$args			array of any extra arguments that existed in the print command
	* @param string	$value_prefix	PHP code to wrap put before the value before outputting it
	* @param string	$value_suffix	PHP code to wrap put after  the value before outputting it
	*
	* @return void
	* @access public
	*/
	public function printVar($var, Array $args, $value_prefix='', $value_suffix='')
	{
		switch ($var) {
			case 'assetid' :
				echo '<', '?php echo $ASSET_LINEAGE[$'.$this->attr('id_name').'_i][\'assetid\']; ?', '>';
			break;

			case 'asset_link' :
				echo '<', '?php echo $ASSET_LINEAGE[$'.$this->attr('id_name').'_i][\'protocol\'].\'://\'.$ASSET_LINEAGE[$'.$this->attr('id_name').'_i][\'url\']; ?', '>';
			break;

			case 'asset_short_name' :
			case 'asset_name' :
				echo '<', '?php echo ', $this->_escapeVar($value_prefix.'htmlspecialchars($ASSET_LINEAGE[$'.$this->attr('id_name').'_i]['.var_export((string) substr($var, 6), TRUE).'])'.$value_suffix, $args), '; ?', '>';
			break;

			case 'lineage_level' :
				echo '<', '?php echo ', '$'.$this->attr('id_name').'_i; ?', '>';
			break;

			case (preg_match('/^asset_/i', $var) ? $var : !$var):
					echo '<', '?php ';
					echo '$keyword = \'',$var, '\';';
					echo '$ASSET1 = isset($ASSET_LINEAGE[$'.$this->attr('id_name').'_i][\'assetid\']) ? $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($ASSET_LINEAGE[$'.$this->attr('id_name').'_i][\'assetid\']) : NULL;';
					echo 'if (!is_null($ASSET1)) {';
					echo '$replacement = '.$this->_escapeVar('$ASSET1->getKeywordReplacement($keyword)', $args).';';
					echo 'if ($replacement == $keyword) {';
					echo '	echo \'\';';
					echo '} else {';
					echo '	echo $replacement;';
					echo '}';
					echo '$GLOBALS[\'SQ_SYSTEM\']->am->forgetAsset($ASSET1);';
					echo '} else {';
					echo '  echo \'\';';
					echo '}';
					echo '?', '>';
			break;

			default :
				parent::printVar($var, $args);

		}// end switch

	}//end printVar()


	/**
	* Print code that outputs a given element from this DA's contents
	*
	* @param array	$element	An entry from our contents array
	*
	* @return void
	* @access private
	*/
	protected function _printContentItemCode(Array $element)
	{
		switch (array_get_index($element, 'operation')) {
			case 'asset' :
				// Make lineage working with ?a=xxx
				echo '<?php
				if (!empty($ASSET_LINEAGE) && isset($_GET[\'a\']) && isset($GLOBALS[\'SQ_SYSTEM\']->frontend_asset) && $_GET[\'a\'] == $GLOBALS[\'SQ_SYSTEM\']->frontend_asset->id){
					$last_ele_index = count($ASSET_LINEAGE) - 1;
					$ASSET_LINEAGE[] = array (
										\'url\' => $ASSET_LINEAGE[$last_ele_index][\'url\'].\'?a=\'.$GLOBALS[\'SQ_SYSTEM\']->frontend_asset->id,
										\'assetid\' => $GLOBALS[\'SQ_SYSTEM\']->frontend_asset->id,
										\'name\' => $GLOBALS[\'SQ_SYSTEM\']->frontend_asset->name,
										\'short_name\' => $GLOBALS[\'SQ_SYSTEM\']->frontend_asset->short_name,
										\'type_code\' => get_class_lower($GLOBALS[\'SQ_SYSTEM\']->frontend_asset),
										\'protocol\' => $ASSET_LINEAGE[$last_ele_index][\'protocol\'],
										\'link_type\' => $ASSET_LINEAGE[$last_ele_index][\'link_type\'],
									);
				}
				?>';

				// because the page lineage array contains the site index, all we need to do to have or not have
				// the home link is start the loop at a different position
				$start_index = ($this->attr('prefix_with_home_link')) ? 0 : 1;

				$start_at_level = $this->attr('start_at_level');
				if (!is_null($start_at_level) && $start_at_level !== '') {
					$start_index = $start_at_level >= 0 ? $start_at_level : 'max(0, (count($ASSET_LINEAGE) - 1 - '.abs($start_at_level).'))';
				}//end if

				$total = 'count($ASSET_LINEAGE)';
				// because the page lineage array contains the current page, all we need to do to have or not have
				// the current link is end the loop at a different position
				if (!$this->attr('suffix_with_current_link')) {
					$total .= ' - 1';
				}

				$levels_to_print = (int) $this->attr('levels_to_print');

				if (is_int($levels_to_print)) {
					// if we have pages to print, we just adjust the total count factor
					if ($levels_to_print > 0) {
						$total = 'min('.$total.', '.($levels_to_print.' + '.$start_index).')';
					} else if ($levels_to_print < 0) {
						// need to minus 1 because the current link is included in the lineage count
						$start_index = 'max(0, (count($ASSET_LINEAGE) - 1 - '.abs($levels_to_print).'))';
					}
				}

				if ($divider_contents = $this->attr('divider_contents')) {
					ob_start();
					foreach ($divider_contents as $item) {
						$this->_printContentItemCode($item);
					}
					$divider = ob_get_contents();
					ob_end_clean();
				} else {
					$divider = '';
				}

				if ($divider != '' && $this->attr('prefix_with_divider')) {
					echo $divider;
				}

				echo '<', '?php

				';

				if ($this->attr('reverse_lineage')) {
					echo 'for ($'.$this->attr('id_name').'_i = '.$total.' - 1, $'.$this->attr('id_name').'_total = '.$total.';
							$'.$this->attr('id_name').'_i >= '.$start_index.';
							$'.$this->attr('id_name').'_i--) {
							';
				} else {
					echo 'for ($'.$this->attr('id_name').'_i = '.$start_index.', $'.$this->attr('id_name').'_total = '.$total.';
							$'.$this->attr('id_name').'_i < $'.$this->attr('id_name').'_total;
							$'.$this->attr('id_name').'_i++) {
							';
				}

				if (!$this->attr('show_hidden_links')) {
					echo 'if ($ASSET_LINEAGE[$'.$this->attr('id_name').'_i][\'link_type\'] != SQ_LINK_TYPE_1) continue;
					';
				}

				$unwanted_types = $this->attr('unwanted_asset_types');
				$all_type_codes = Array();
				foreach ($unwanted_types as $type) {
					if (!empty($type)) {
						$all_type_codes = array_merge($all_type_codes, $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type, TRUE));
					}
				}
				$unwanted_types = $all_type_codes;

				if (!empty($unwanted_types)) {
					$unwanted_types_literal_array = 'Array("'.implode('", "', $unwanted_types).'")';
					if ($this->attr('prefix_with_home_link')) {
						// make the home-link-prefix setting override
						// the unwanted-types setting
						echo 'if (($'.$this->attr('id_name').'_i != 0) && in_array($ASSET_LINEAGE[$'.$this->attr('id_name').'_i][\'type_code\'], '.$unwanted_types_literal_array.')) continue;
						';
					} else {
						echo 'if (in_array($ASSET_LINEAGE[$'.$this->attr('id_name').'_i][\'type_code\'], '.$unwanted_types_literal_array.')) continue;
						';
					}

				}

				if ($asset_contents = $this->attr('asset_contents')) {
					echo '?','>';
					foreach ($asset_contents as $item) {
						$this->_printContentItemCode($item);
					}
					echo '<','?php ';
				}

				// if there is a divider to deal with
				if ($divider != '') {

					// if there is not divider at the end we need to check that we aren't at the end of the list
					if (!$this->attr('suffix_with_divider')) {
						if ($this->attr('reverse_lineage')) {
							echo '
								if ($'.$this->attr('id_name').'_i != '.$start_index.') {
							';
						} else {
							echo '
								if ($'.$this->attr('id_name').'_i != $'.$this->attr('id_name').'_total - 1) {
							';
						}
					}

					echo '?','>', $divider, '<','?php ';

					// if there is not divider at the end we need to check that we aren't at the end of the list
					if (!$this->attr('suffix_with_divider')) {
					echo '
						}//end if';
					}

				}//end if divider

				echo '
				}//end for
				?','>';

				return TRUE;

			default :
				return parent::_printContentItemCode($element);
		}//end switch

	}//end _printContentItemCode()


	/**
	* Get the names of attributes for this DA that cannot be set in the parse file
	*
	* Design Area attributes that can be set in the parse file but aren't are set to defaults
	* when parsing the design.  Attributes returned by this function are exempted from this process.
	*
	* @return array
	* @access public
	*/
	public function getProtectedAttrs()
	{
		$res = parent::getProtectedAttrs();
		$res[] = 'asset_contents';
		$res[] = 'divider_contents';
		return $res;

	}//end getProtectedAttrs()


	/**
	* Queue the setting of the value of an attribute
	*
	* This function sets the value of the attribute locally, but does not database updating.
	* The change is queued awaiting the call of saveAttributes()
	*
	* @param string	$name	the name of the attribute
	* @param string	$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	* @see saveAttributes()
	*/
	public function setAttrValue($name, $value)
	{
		if (($name == 'unwanted_asset_types') && is_scalar($value)) {
			$value = preg_split('/[ ,]+/', (string)$value);
			foreach ($value as $type_code) {
				if (!$GLOBALS['SQ_SYSTEM']->am->installed($type_code)) {
					trigger_localised_error('CORE0145', sprintf(translate('Asset Type "%s" is not installed in this system, unable to set "type_codes"'), $type_code), E_USER_WARNING);
					return FALSE;
				}
			}
		}
		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


}//end class
?>
