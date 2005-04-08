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
* $Id: locale_manager.inc,v 1.3 2005/04/08 05:39:09 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


/**
* Locale_Manager
*
* Purpose
*    Handles localisation functions such as translations and
*    number/date formats
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix
*/
class Locale_Manager extends MySource_Object
{


	/**
	* Holds a stack of locale settings - most current first
	*
	* @var Array
	* @access public
	*/
	var $locale_stack = Array();


	/**
	* Holds a stack of locale settings - most current first
	*
	* @var Array
	* @access public
	*/
	var $_strings = Array();


	/**
	* Constructor
	*
	* @return void
	* @access public
	*/
	function Locale_Manager()
	{
		$this->MySource_Object();

	}//end constructor


	/**
	* Gets the code of the current locale
	*
	* Returns the code of the current language, or boolean FALSE if locale stack underflowed
	*
	* @return mixed	(string|boolean)
	* @access public
	*/
	function getCurrentLocale()
	{
		// no locale is current
		if (count($this->locale_stack) <= 0) {
			return false;
		}

		return $this->locale_stack[0];

	}//end getCurrentLocale()


	/**
	* Gets the full name of a locale
	*
	* @param string $locale_code	the locale code to check (if omitted, uses
	*								the current locale)
	*
	* @return string the name of the current locale
	* @access public
	*/
	function getLocaleName($locale_code=null)
	{
		if (is_null($locale_code)) $locale_code = $this->getCurrentLocale();
		require SQ_FUDGE_PATH.'/standards_lists/locales.inc';
		return array_get_index($standards_lists_locales, $locale_code, 'Unknown');

	}//end getLocaleName()


	/**
	* Sets the current locale, loading the language files if that is required
	*
	* @param string $locale the locale to set
	*
	* @return boolean	indicating success
	* @access public
	*/
	function setCurrentLocale($locale)
	{
		//if (!isset($this->_lang_array[$locale])) {
			//$this->_loadLocale($locale);
		//}

		array_unshift($this->locale_stack, $locale);
		$this->includeCoreStrings($locale);
		//bam($this->_strings);
		return true;

	}//end setCurrentLocale()


	/**
	* Restore the previously set locale from the top of the stack
	*
	* @return boolean	indicating success - will return FALSE if the
	*					locale stack would underflow
	* @access public
	*/
	function restoreCurrentLocale()
	{
		// stack would underflow if we tried to shift this out
		if (count($this->locale_stack) <= 0) {
			return false;
		}

		array_shift($this->locale_stack);
		return true;

	}//end restoreCurrentLocale()


	/**
	* Split a full locale into its parts
	*
	* Can be called using, eg:
	* list($country, $language, $variant) =
	*	$GLOBALS['SQ_SYSTEM']->lm->getLocaleParts('en_AU');
	*
	* @param string	$locale	the full locale
	*
	* @return Array	three element array with country, language and variant
	*		(if any are missing they will be NULL)
	* @access public
	*/
	function getLocaleParts($locale)
	{
		$locale_parts = Array(null, null, null);

		preg_match('|([A-Za-z]{2})(\_([A-Za-z]{2})(\@([A-Za-z]+))?)?|', $locale, $matches);

		// switch on the number of match expansions we get - if we have all 3
		// parts we will have 6 matches, if we have language and country we will
		// have 4, if just the language we will have 2. This means the time can
		// be ripe for a fall-through switch

		switch (count($matches)) {
			case 6:
				$locale_parts[2] = $matches[5];
			// no break; fall through to case 4

			case 4:
				$locale_parts[1] = $matches[3];
			// no break; fall through to case 2

			case 2:
				$locale_parts[0] = $matches[1];
			break;

			default:
				trigger_error('Locale code "'.$locale.'" does not seem to be valid', E_USER_WARNING);
				return false;
			break;
		}//end switch

		return $locale_parts;

	}//end getLocaleParts()


	/**
	* gets a string translation
	*
	* @param string	$string_code	the string code
	*
	* @return string	the translated string or '' if not found
	* @access public
	*/
	function getString($string_code)
	{
		$locale = $this->getCurrentLocale();

		// if second parameter is an array, use that as our list of params
		if (!is_array($func_args = func_get_arg(1))) {
			$func_args = func_get_args();
			$func_args = array_slice($func_args,1);
		}

		if (!isset($this->_strings[$locale])) {
			return '';
		}

		if (!isset($this->_strings[$locale][$string_code])) {
			return '';
		}

		return vsprintf($this->_strings[$locale][$string_code],$func_args);

	}//end getString()


	/**
	* Includes the compiled strings for a specified asset, if they haven't already
	*
	* @param string	$type_code	the type code to include strings for
	* @param string	$locale		the locale to use (default to current)
	*
	* @return void
	* @access public
	*/
	function includeAssetStrings($type_code, $locale=null)
	{
		// keep track of which assets have been included so we don't include
		// a certain set of strings more than once
		static $assets_included = Array();

		if (!isset($assets_included[$type_code])) {
			$assets_included[$type_code] = true;
			if ($locale == null) {
				$locale = $this->getCurrentLocale();
			}

			list($lang, $country, $variant) = $this->getLocaleParts($locale);
			$locales = Array($lang);
			if (!empty($country)) {
				$locales[] = $lang.'_'.$country;
				if (!empty($variant)) {
					$locales[] = $lang.'_'.$country.'@'.$variant;
				}
			}

			foreach ($locales as $this_locale) {
				$file = SQ_DATA_PATH.'/private/asset_types/'.$type_code.'/strings.'.$this_locale;

				if (file_exists($file)) {
					$strings = unserialize(file_get_contents($file));

					if (!isset($this->_strings[$locale])) {
						$this->_strings[$locale] = Array();
					}
					$this->_strings[$locale] = array_merge($this->_strings[$locale], $strings);
				}
			}
		}

	}//end includeAssetStrings()


	/**
	* Includes the strings from a package, if they haven't already
	*
	* @param string	$package_name	the name of the package
	* @param string	$locale		the locale to use (default = current)
	*
	* @return void
	* @access public
	*/
	function includePackageStrings($package_name, $locale=null)
	{
		// keep track of which packages have been included so we don't include
		// a certain set of strings more than once
		static $packages_included = Array();

		if (!isset($packages_included[$package_name])) {
			$packages_included[$package_name] = true;

			if ($locale == null) {
				$locale = $this->getCurrentLocale();
			}

			$file = SQ_DATA_PATH.'/private/packages/'.$package_name.'/strings.'.$locale;
			if (file_exists($file)) {
				$strings = unserialize(file_get_contents($file));

				if (!isset($this->_strings[$locale])) {
					$this->_strings[$locale] = Array();
				}
				$this->_strings[$locale] = array_merge($this->_strings[$locale], $strings);
			}
		}

	}//end includePackageStrings()


	/**
	* Include the system-wide core strings, if they haven't already
	*
	* @param string	$locale	the locale to include (default to current locale)
	*
	* @return void
	* @access public
	*/
	function includeCoreStrings($locale=null)
	{
		// keep track of whether we've included the core strings already
		static $core_included = false;

		if (!$core_included) {
			$core_included = true;

			if ($locale == null) {
				$locale = $this->getCurrentLocale();
			}

			$file = SQ_DATA_PATH.'/private/asset_types/asset/strings.'.$locale;
			if (file_exists($file)) {
				$strings = unserialize(file_get_contents($file));

				if (!isset($this->_strings[$locale])) {
					$this->_strings[$locale] = Array();
				}
				$this->_strings[$locale] = array_merge($this->_strings[$locale], $strings);
			}
		}

	}//end includeCoreStrings()


}//end class
?>
