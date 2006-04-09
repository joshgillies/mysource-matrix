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
* $Id: lookup_value.inc,v 1.4.2.1 2006/04/09 23:22:11 sdanis Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Lookup Value
*
* Parent Asset for all different types of lookup value interfaces needed by different parts of the system
*
*
* @author	Blair Robertson <brobertson@squiz.co.uk>
* @version $Revision: 1.4.2.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Lookup_Value extends Asset
{
	/**
	* Return the lookup name to use in the interface
	*
	* @static
	* @return string
	* @access public
	*/
	function lookupValueName()
	{
		return translate('core_default_layout_name');

	}//end lookupValueName()


	/**
	* Return the prefix that these values use in their name (minus the '::')
	*
	* @static
	* @return string
	* @access public
	*/
	function namePrefix()
	{
		trigger_localised_error('CORE0123', E_USER_ERROR, __CLASS__, __FUNCTION__);

	}//end namePrefix()


	/**
	* Paint the HTML editing interface for this condition.
	*
	* Returns TRUE if any editing fields were printed
	*
	* @param object	&$owner			the asset whose interface we are painting
	* @param object	&$o				the outputter class
	* @param array	$lookups		array of available lookups
	* @param array	$lookup_values	array of current values
	*								<pre>
	*								"[url]" => array (
	*									"[name]" => array (
	*										"value" => "[value]",
	*										"inhd" => "[inherited]",
	*									),
	*								),
	*								</pre>
	* @param string	$prefix			unique prefix to use for the form elements
	*
	* @static
	* @return boolean
	* @access public
	*/
	function paintInterface(&$owner, &$o, $lookups, $lookup_values, $prefix)
	{
		$wa = ($owner->writeAccess('lookups'));

		foreach ($lookups as $url) {

			$o->openSection($url);

			if (empty($lookup_values[$url])) {
				$lookup_values[$url] = Array();
			}

			// add blank entry if we have write access
			if ($wa) {
				$lookup_values[$url][''] =  Array('value' => '', 'inhd' => '0');
			}

			$i = 0;
			foreach ($lookup_values[$url] as $value_name => $data) {
				$o->openSection(($value_name == '') ? 'New' : $value_name);

					$o->openField(translate('current'));
						echo '<i>';
						if ($data['value'] == '') {
							echo translate('none');
						} else {
							echo $data['value'];
							echo '<br><b>'.translate('inherited_question').' </b>', $data['inhd'];
						}
						echo '</i>';
					$o->closeField();

					if ($wa) {
						$o->openField(($data['value'] == '') ? translate('change_question') : translate('new_question'));
							echo ''.translate('core_value_name').': ';
							text_box($prefix.'['.$url.']['.$i.'][value_name]', $value_name, 50);
							echo '<br>'.translate('value').': ';
							text_box($prefix.'['.$url.']['.$i.'][value]', $data['value'], 50);
						$o->closeField();
					}

				$o->closeSection();
				$i++;

			}//end foreach

			$o->closeSection();

		}//end foreach

		return $wa;

	}//end paintInterface()


	/**
	* Process the editing interface for this lookup value, returns TRUE on success
	*
	* @param object	&$owner			the asset whose interface we are painting
	* @param object	&$o				the outputter class
	* @param array	$lookups		array of available lookups
	* @param Array 	&$lookup_values	array of the values that we want to save
	*								<pre>
	*								"[url]" => array (
	*									"[name]" => array (
	*										"value" => "[value]",
	*										"inhd" => "[inherited]",
	*									),
	*								),
	*								</pre>
	* @param string	$prefix			unique prefix to use for the form elements
	*
	* @static
	* @return boolean
	* @access public
	*/
	function processInterface(&$owner, &$o, $lookups, &$lookup_values, $prefix)
	{
		return FALSE;

	}//end processInterface()


}//end class

?>
