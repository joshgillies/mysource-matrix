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
* $Id: lookup_value.inc,v 1.9 2012/08/30 01:09:08 ewang Exp $
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
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Lookup_Value extends Asset
{


	/**
	* Return the lookup name to use in the interface
	*
	* @return string
	* @access public
	* @static
	*/
	public static function lookupValueName()
	{
		return translate('Default Layout Name');


	}//end lookupValueName()


	/**
	* Return the prefix that these values use in their name (minus the '::')
	*
	* @return string
	* @access public
	* @static
	*/
	public static function namePrefix()
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
	* @return boolean
	* @access public
	* @static
	*/
	public static function paintInterface(&$owner, &$o, $lookups, $lookup_values, $prefix)
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

					$o->openField(translate('Current'));

						echo '<i>';
						if ($data['value'] == '') {
							echo translate('None');

						} else {
							echo $data['value'];
							echo '<br><b>'.translate('Inherited?').' </b>', $data['inhd'];

						}
						echo '</i>';
					$o->closeField();

					if ($wa) {
						$o->openField(($data['value'] == '') ? translate('Change?') : translate('New?'));


							echo ''.translate('Value Name').': ';

							text_box($prefix.'['.$url.']['.$i.'][value_name]', $value_name, 50);
							echo '<br>'.translate('Value').': ';

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
	* @param array	&$lookup_values	array of the values that we want to save
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
	* @return boolean
	* @access public
	* @static
	*/
	public static function processInterface(&$owner, &$o, $lookups, &$lookup_values, $prefix)
	{
		return FALSE;

	}//end processInterface()


}//end class

?>
