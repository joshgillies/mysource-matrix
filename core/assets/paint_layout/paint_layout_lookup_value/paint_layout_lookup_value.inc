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
* $Id: paint_layout_lookup_value.inc,v 1.6 2006/02/07 00:41:31 skim Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/lookup_value/lookup_value.inc';

/**
* Lookup Value
*
* Parent Asset for all different types of lookup value interfaces needed by different parts of the system
*
*
* @author	Blair Robertson <brobertson@squiz.co.uk>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Paint_Layout_Lookup_Value extends Lookup_Value
{


	/**
	* Return the lookup name to use in the interface
	*
	* @return string
	* @access public
	* @static
	*/
	function lookupValueName()
	{
		return 'Paint Layout';

	}//end lookupValueName()


	/**
	* Return the prefix that these values use in their name (minus the '::')
	*
	* @return string
	* @access public
	* @static
	*/
	function namePrefix()
	{
		return 'paint_layout';

	}//end namePrefix()


	/**
	* Paint the HTML editing interface for this lookup value.
	*
	* @param object	&$owner			the asset whose interface we are painting
	* @param object	&$o				the backend outputter object
	* @param array	$lookups		array of available lookups
	* @param array	$lookup_values	array of the values that we want to save
	*								<pre>
	*									"[url]" => array (
	*										"[name]" => array (
	*											"value" => "[value]",
	*											"inhd" => "[inherited]",
	*										),
	*									),
	*								</pre>
	* @param string	$prefix			unique prefix to use for the form elements
	*
	* @return boolean
	* @access public
	* @static
	*/
	function paintInterface(&$owner, &$o, $lookups, $lookup_values, $prefix)
	{
		$wa = ($owner->writeAccess('lookups') && $owner->writeAccess('links'));
		$name_prefix = Paint_Layout_Lookup_Value::namePrefix();

		$sort_fn_code = '
			preg_match('.var_export('/^'.$name_prefix.'::(system|user)::(.*)$/', 1).', $a, $a_matches=Array());
			preg_match('.var_export('/^'.$name_prefix.'::(system|user)::(.*)$/', 1).', $b, $b_matches=Array());
			if ($a_matches[1] == "system" && $a_matches[2] == "frontend") return -1;
			if ($b_matches[1] == "system" && $b_matches[2] == "frontend") return  1;

			if ($a_matches[1] == "system" && $b_matches[1] == "user")   return -1;
			if ($a_matches[1] == "user"   && $b_matches[1] == "system") return  1;

			return strcmp($a_matches[2], $b_matches[2]);
		';

		$sort_fn = create_function('$a,$b', $sort_fn_code);

		foreach ($lookups as $url) {

			$o->openSection($url);
			$o->openField(translate('lineage'));
			$lineage = $GLOBALS['SQ_SYSTEM']->am->getLineageFromURL(NULL, $url);
			foreach ($lineage as $i => $asset) {
				if ($i > 0) echo '<br />&nbsp;&gt;&nbsp;<br />';
				echo get_asset_tag_line($asset['assetid']);
			}
			unset($lineage);
			$o->closeField();

			if (empty($lookup_values[$url])) {
				$lookup_values[$url] = Array();
			}

			// need to make sure the default is printed.
			if (empty($lookup_values[$url][$name_prefix.'::system::frontend'])) {
				$lookup_values[$url][$name_prefix.'::system::frontend'] =  Array('value' => '', 'inhd' => '0');
			}

			uksort($lookup_values[$url], $sort_fn);

			// add blank entry if we have write access
			if ($wa) {
				$lookup_values[$url][$name_prefix.'::user::'] =  Array('value' => '', 'inhd' => '0');
			}

			foreach ($lookup_values[$url] as $layout_name => $data) {

				$matches = Array();
				preg_match('/^'.$name_prefix.'::(system|user)::(.*)$/', $layout_name, $matches);

				$layout_type     = $matches[1];
				$layout_code     = $matches[2];
				$new_user_layout = ($layout_type == 'user' && $layout_code == '' && empty($data['value']));

				if ($new_user_layout) {
					$display_name = translate('create_user_defined_layout');
				} else {
					$display_name = translate('defined_layout_display', ucwords($layout_type), ucwords(str_replace('_', ' ', $layout_code)));
				}


				$o->openSection($display_name);

					if ($new_user_layout) {
						$o->openField(translate('layout_code'));
							text_box($prefix.'['.$url.']['.$layout_name.'][name]', '', 20, 30);
						$o->closeField();
					} else {
						if ($layout_type == 'user') {
							$o->openField(translate('layout_code'));
								echo $layout_code;
							$o->closeField();
						}
					}

					$o->openField(translate('current'));
						echo '<i>';
						if ($data['value'] == '') {
							echo translate('none');
						} else {
							$layout =& $GLOBALS['SQ_SYSTEM']->am->getAsset($data['value'], '', TRUE);
							if (!is_null($layout)) {
								echo get_asset_tag_line($layout->id);
								if ($data['inhd']) {
									echo '<br><b>('.translate('inherited').')</b>';
								}
							}
						}
						echo '</i>';
					$o->closeField();

					if ($wa) {
						// if we have inherited this value, then we haven't set it ourselves have we ? :)
						$layoutid = (empty($data['inhd'])) ? $data['value'] : 0;
						$o->openField(($layoutid) ? translate('change_question') : translate('new_question'));
							$buttons = Array('clear');
							if (!$new_user_layout) $buttons[] = 'reset';
							asset_finder($prefix.'['.$url.']['.$layout_name.'][value]', $layoutid, Array('paint_layout_page' => 'D'), 'sq_sidenav', FALSE, 'null', $buttons);
						$o->closeField();
					}

				$o->closeSection();

			}//end foreach

			$o->closeSection();

		}//end foreach

		return $wa;

	}//end paintInterface()


	/**
	* Process the editing interface for this lookup value
	*
	* @param object	&$owner			the asset whose interface we are painting
	* @param object	&$o				the backend outputter object
	* @param array	$lookups		array of available lookups
	* @param array	&$lookup_values	array of the values that we want to save
	*								<pre>
	*									"[url]" => array (
	*										"[name]" => array (
	*											"value" => "[value]",
	*											"inhd" => "[inherited]",
	*										),
	*									),
	*								</pre>
	* @param string	$prefix			unique prefix to use for the form elements
	*
	* @return boolean
	* @access public
	* @static
	*/
	function processInterface(&$owner, &$o, $lookups, &$lookup_values, $prefix)
	{
		if (!isset($_POST[$prefix]) || !is_array($_POST[$prefix])) {
			return FALSE;
		}

		$lookup_values = Array();
		$name_prefix = Paint_Layout_Lookup_Value::namePrefix();

		foreach ($_POST[$prefix] as $url => $layouts) {
			foreach ($layouts as $layout_name => $layout_data) {

				// new user defined layout ?
				if ($layout_name == $name_prefix.'::user::') {
					if (empty($layout_data['name'])) continue;
					if (empty($layout_data['value']['assetid'])) continue;
					$layout_name .= strtolower($layout_data['name']);
					if (isset($lookup_values[$url][$layout_name])) {
						trigger_error('Layout Name "'.$layout_data['name'].'" already in use for "'.$url.'"', E_USER_WARNING);
						continue;
					}
				}// end if

				// if it's empty set to blank strring so it get's deleted
				$lookup_values[$url][$layout_name]['value'] = (empty($layout_data['value']['assetid'])) ? '' : $layout_data['value']['assetid'];

			}
		}

		return TRUE;

	}//end processInterface()


}//end class

?>