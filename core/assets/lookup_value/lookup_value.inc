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
* $Id: lookup_value.inc,v 1.1 2005/02/21 11:18:44 brobertson Exp $
*
*/


/**
* Lookup Value
*
* Parent Asset for all different types of lookup value interfaces needed by different parts of the system
*
*
* @author	Blair Robertson <brobertson@squiz.co.uk>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Lookup_Value extends Asset
{
	/**
	* Return the lookup name to use in the interface
	*
	* @access public
	* @return string
	* @static
	*/
	function lookupValueName()
	{
		return 'Default Layout Name';

	}//end lookupValueName()


	/**
	* Return the prefix that these values use in their name (minus the '::')
	*
	* @access public
	* @return string
	* @static
	*/
	function namePrefix()
	{
		trigger_error('You need to override '.__CLASS__.'::'.__FUNCTION__.'()', E_USER_ERROR);

	}//end namePrefix()


	/**
	* Paint the HTML editing interface for this condition.
	*
	* @param object	Asset				&$owner			the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o				the outputter class
	* @param Array 						$lookups		array of available lookups
	* @param Array 						$lookup_values	array of current values
	*													<pre>
	*													"[url]" => array (
	*														"[name]" => array (
	*															"value" => "[value]",
	*															"inhd" => "[inherited]",
	*														),
	*													),
	*													</pre>
	* @param string 					$prefix			unique prefix to use for the form elements
	*
	* @access public
	* @return boolean		indicating if any editing fields were printed
	* @static
	*/
	function paintInterface(&$owner, &$o, $lookups, $lookup_values, $prefix)
	{
		$wa = ($owner->writeAccess('lookups'));

		foreach ($lookups as $url) {

			$o->openSection($url);

			if (empty($lookup_values[$url])) $lookup_values[$url] = Array();
			
			// add blank entry if we have write access
			if ($wa) {
				$lookup_values[$url][''] =  Array('value' => '', 'inhd' => '0');
			}

			$i = 0;
			foreach ($lookup_values[$url] as $value_name => $data) {
				$o->openSection(($value_name == '') ? 'New' : $value_name);

					$o->openField('Current');
						echo '<i>';
						if ($data['value'] == '') {
							echo 'None';
						} else {
							echo $data['value'];
							echo '<br><b>Inherited ? </b>', $data['inhd'];
						}
						echo '</i>';
					$o->closeField();

					if ($wa) {
						$o->openField(($data['value'] == '') ? 'Change ?' : 'New ?');
							echo 'Value Name: ';
							text_box($prefix.'['.$url.']['.$i.'][value_name]', $value_name, 50);
							echo '<br>Value: ';
							text_box($prefix.'['.$url.']['.$i.'][value]', $data['value'], 50);
						$o->closeField();
					}

				$o->closeSection();
				$i++;

			}// end foreach

			$o->closeSection();

		}// end foreach

		return $wa;

	}//end paintInterface()


	/**
	* Process the editing interface for this lookup value
	*
	* @param object	Asset				&$owner			the asset whose interface we are painting
	* @param object	Backend_Outputter	$o				the outputter class
	* @param Array 						$lookups		array of available lookups
	* @param Array 						$lookup_values	array of the values that we want to save
	*													<pre>
	*													"[url]" => array (
	*														"[name]" => array (
	*															"value" => "[value]",
	*															"inhd" => "[inherited]",
	*														),
	*													),
	*													</pre>
	* @param string 					$prefix			unique prefix to use for the form elements
	*
	* @access public
	* @return boolean		indicating success or abort
	* @static
	*/
	function processInterface(&$owner, &$o, $lookups, &$lookup_values, $prefix)
	{
		return false;

	}//end processInterface()

}//end class

?>
