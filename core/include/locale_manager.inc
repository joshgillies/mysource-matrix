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
* $Id: locale_manager.inc,v 1.22 2013/07/25 23:23:49 lwright Exp $
*
*/


require_once SQ_FUDGE_PATH.'/general/text.inc';


/**
* Locale_Manager
*
* Purpose
*    Handles localisation functions such as translations and
*    number/date formats
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.22 $
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
	public $locale_stack = Array();


	/**
	* Holds a stack of strings for each language
	*
	* @var Array
	* @access public
	*/
	public $_strings = Array();


	/**
	* Holds a stack of errors for each language
	*
	* @var Array
	* @access public
	*/
	public $_errors = Array();


	/**
	* Holds a stack of internal messages for each language
	*
	* @var Array
	* @access public
	*/
	public $_internal_messages = Array();


	/**
	 * keep track of which assets have been included so we don't include
	 * a certain set of strings more than once for each locale
	 *
	 * @var Array
	 * @access private
	 */
	private $_assets_included = Array();


	/**
	 * keep track of which packages have been included so we don't include
	 * a certain set of strings more than once for each locale
	 *
	 * @var Array
	 * @access private
	 */
	private $_packages_included = Array();


	/**
	 * keep track of whether we've included the core strings already
	 * (for this locale at least)
	 *
	 * @var Array
	 * @access private
	 */
	private $_core_included = Array();


	/**
	* Constructor
	*
	* @return void
	* @access public
	*/
	function __construct()
	{
		$this->MySource_Object();

	}//end constructor


	/**
	 * Destructor
	 *
	 * Attempts to save state in Deja Vu to improve performance.
	 *
	 * @return void
	 */
	function __destruct()
	{
		if (array_get_index($this->_tmp, 'strings_modified', FALSE) && isset($GLOBALS['SQ_SYSTEM'])) {
			$this->locale_stack = Array();
			// Attempt to store me so that I don't have to go through all that again.
			$deja_vu = $GLOBALS['SQ_SYSTEM']->getDejaVu();
			if ($deja_vu) {
				$deja_vu->remember(SQ_DEJA_VU_LOCALE, NULL, $this);
			}
		}

	}//end destructor


	/**
	* Gets the code of the current locale
	*
	* Returns the code of the current language, or boolean FALSE if locale stack underflowed
	*
	* @return mixed string|boolean
	* @access public
	*/
	public function getCurrentLocale()
	{
		if (count($this->locale_stack) <= 0) return FALSE; // no locale is current
		return $this->locale_stack[0];

	}//end getCurrentLocale()


	/**
	* Gets the full name of a locale
	*
	* @param string	$locale_code	the locale code to check (if omitted, uses
	*								the current locale)
	*
	* @return string
	* @access public
	*/
	public function getLocaleName($locale_code=NULL)
	{
		if (is_null($locale_code)) {
			$locale_code = $this->getCurrentLocale();
		}
		require SQ_FUDGE_PATH.'/standards_lists/locales.inc';
		return array_get_index($standards_lists_locales, $locale_code, 'Unknown');

	}//end getLocaleName()


	/**
	* Sets the current locale, loading the language files if that is required
	*
	* @param string	$locale	the locale to set
	*
	* @return boolean
	* @access public
	*/
	public function setCurrentLocale($locale)
	{
		array_unshift($this->locale_stack, $locale);
		$this->includeCoreStrings($locale);

		return TRUE;

	}//end setCurrentLocale()


	/**
	* Restore the previously set locale from the top of the stack
	*
	* @return boolean
	* @access public
	*/
	public function restoreCurrentLocale()
	{
		if (count($this->locale_stack) <= 0) return FALSE; // stack would underflow if we tried to shift this out
		array_shift($this->locale_stack);
		return TRUE;

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
	* @return array
	* @access public
	*/
	public function getLocaleParts($locale)
	{
		$locale_parts = Array(NULL, NULL, NULL);

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
				trigger_localised_error('SYS0193', sprintf(translate('Locale code "%s" does not seem to be valid'), $locale), E_USER_WARNING)
				return FALSE;
			break;
		}//end switch

		return $locale_parts;

	}//end getLocaleParts()


	/**
	* Splits up the locale passed into language, country, and variant and return
	* an array consisting of all possible partial locales
	*
	* This will return an array of between one and three elements, depending on
	* what parts of the locale are in there. For example, if locale = 'fr_FR@euro',
	* this will return ['fr', 'fr_FR', 'fr_FR@euro']
	*
	* @param string	$locale		the locale that we are going to split
	* @param string	$lang_sep	language seperator character
	* @param string	$var_sep	variant seperator character
	*
	* @return array
	* @access public
	*/
	public function getCumulativeLocaleParts($locale, $lang_sep='_', $var_sep='@')
	{
		$locale_parts = $this->getLocaleParts($locale);

		$cum_locale_parts = Array();
		$cum_locale_parts[] = $locale_parts[0];

		if (count($cum_locale_parts) >= 2) {
			$cum_locale_parts[] = $cum_locale_parts[0].$lang_sep.$locale_parts[1];

			if (count($cum_locale_parts) >= 3) {
				$cum_locale_parts[] = $cum_locale_parts[1].$var_sep.$locale_parts[2];
			}
		}

		return $cum_locale_parts;

	}//end getCumulativeLocaleParts()


	/**
	* Gets a string translation(s).
	*
	* Alias in general.inc: translate().
	*
	* Transitional before full change to gettext-style localisation, supporting
	* both underscored and full untranslated bare strings along with Matrix 3/4
	* style passing of arguments, sprintf-style.
	*
	* Sending sprintf-style arguments to translate() is DEPRECATED, and will be
	* changed in existing code when localise changes are in full swing down
	* the track.
	* Please try to use sprintf() or vsprintf() in conjunction with just sending
	* the string to translate(), instead.
	*
	* If a string is not found, it will return the untranslated string, possibly
	* sprintf-replaced. This is a change from Matrix 3/4, where it would return
	* a long error message.
	*
	* If sent an array of {$string}s, we'll throw back an array.
	*
	* @param mixed	$string		(string|array) The untranslated string(s)
	*
	* @see    translate()
	* @return string|array
	* @access public
	*/
	public function getString($strings)
	{
        $locale = $this->getCurrentLocale();

	    $return_array = TRUE;
	    if (is_array($strings) === FALSE) {
	        $return_array = FALSE;
	        $strings      = Array($strings);
	    }

	    $retval = Array();

	    foreach ($strings as $index => $string) {
            if (!isset($this->_strings[$locale])) {
                $retval[$index] = $string;
            } else {
                $retval[$index] = array_get_index($this->_strings[$locale], $string, $string);
            }
        }

        if ($return_array === FALSE) {
            $retval = reset($retval);
        }

        return $retval;

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
	public function includeAssetStrings($type_code, $locale=NULL)
	{
		if (!isset($this->_assets_included[$type_code])) {
			$this->_assets_included[$type_code] = Array();
		}

		if ($locale == NULL) {
			$locale = $this->getCurrentLocale();
		}


		if (!array_get_index($this->_assets_included[$type_code], $locale, FALSE)) {
			$this->_tmp['strings_modified'] = TRUE; // So that we know we've changed.
			$this->_assets_included[$type_code][$locale] = TRUE;

			// include packages, changing \ for / so it works on Windows systems
			if ($type_code != 'asset') {
				$asset_dir = str_replace('\\', '/', SQ_SYSTEM_ROOT.'/'.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'dir'));

				if (strpos($asset_dir, str_replace('\\', '/', SQ_CORE_PACKAGE_PATH)) !== FALSE) {
					$this->includePackageStrings('__core__', $locale);
				} else {
					$asset_dir = str_replace(str_replace('\\', '/', SQ_PACKAGES_PATH).'/', '', $asset_dir);
					$package = substr($asset_dir, 0, strpos($asset_dir, '/'));
					$this->includePackageStrings($package, $locale);
				}
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

				if (is_file($file)) {
					$strings = unserialize(file_get_contents($file));

					if (!isset($this->_strings[$locale])) {
						$this->_strings[$locale] = Array();
					}
					$this->_strings[$locale] = array_merge($this->_strings[$locale], $strings);
				}
			}
		}//end if

	}//end includeAssetStrings()


	/**
	* Includes the strings from a package, if they haven't already
	*
	* @param string	$package_name	the name of the package
	* @param string	$locale			the locale to use (default = current)
	*
	* @return void
	* @access public
	*/
	public function includePackageStrings($package_name, $locale=NULL)
	{
		if (!isset($this->_packages_included[$package_name])) {
			$this->_packages_included[$package_name] = Array();
		}

		if ($locale == NULL) {
			$locale = $this->getCurrentLocale();
		}

		if (!array_get_index($this->_packages_included[$package_name], $locale, FALSE)) {
			$this->_tmp['strings_modified'] = TRUE; // So that we know we've changed.
			$this->_packages_included[$package_name][$locale] = TRUE;

			list($lang, $country, $variant) = $this->getLocaleParts($locale);
			$locales = Array($lang);
			if (!empty($country)) {
				$locales[] = $lang.'_'.$country;
				if (!empty($variant)) {
					$locales[] = $lang.'_'.$country.'@'.$variant;
				}
			}

			foreach ($locales as $this_locale) {
				$file = SQ_DATA_PATH.'/private/packages/'.$package_name.'/strings.'.$this_locale;
				if (file_exists($file)) {
					$strings = unserialize(file_get_contents($file));

					if (!isset($this->_strings[$locale])) {
						$this->_strings[$locale] = Array();
					}
					$this->_strings[$locale] = array_merge($this->_strings[$locale], $strings);
				}

				$file = SQ_DATA_PATH.'/private/packages/'.$package_name.'/errors.'.$this_locale;
				if (is_file($file)) {
					$strings = unserialize(file_get_contents($file));

					if (!isset($this->_errors[$locale])) {
						$this->_errors[$locale] = Array();
					}
					$this->_errors[$locale] = array_merge($this->_errors[$locale], $strings);
				}

				$file = SQ_DATA_PATH.'/private/packages/'.$package_name.'/internal_messages.'.$this_locale;
				if (is_file($file)) {
					$strings = unserialize(file_get_contents($file));

					if (!isset($this->_internal_messages[$locale])) {
						$this->_internal_messages[$locale] = Array();
					}
					$this->_internal_messages[$locale] = array_merge($this->_internal_messages[$locale], $strings);
				}
			}
		}//end if

	}//end includePackageStrings()


	/**
	* Include the system-wide core strings, if they haven't already
	*
	* @param string	$locale	the locale to include (default to current locale)
	*
	* @return void
	* @access public
	*/
	public function includeCoreStrings($locale=NULL)
	{
		if ($locale == NULL) {
			$locale = $this->getCurrentLocale();
		}

		if (!array_get_index($this->_core_included, $locale, FALSE)) {
			$this->_tmp['strings_modified'] = TRUE; // So that we know we've changed.
			$this->_core_included[$locale] = TRUE;

			list($lang, $country, $variant) = $this->getLocaleParts($locale);
			$locales = Array($lang);
			if (!empty($country)) {
				$locales[] = $lang.'_'.$country;
				if (!empty($variant)) {
					$locales[] = $lang.'_'.$country.'@'.$variant;
				}
			}

			foreach ($locales as $this_locale) {
				$file = SQ_DATA_PATH.'/private/system/core/strings.'.$this_locale;
				if (is_file($file)) {
					$strings = unserialize(file_get_contents($file));

					if (!isset($this->_strings[$locale])) {
						$this->_strings[$locale] = Array();
					}
					$this->_strings[$locale] = array_merge($this->_strings[$locale], $strings);
				}

				$file = SQ_DATA_PATH.'/private/system/core/errors.'.$this_locale;
				if (is_file($file)) {
					$strings = unserialize(file_get_contents($file));

					if (!isset($this->_errors[$locale])) {
						$this->_errors[$locale] = Array();
					}
					$this->_errors[$locale] = array_merge($this->_errors[$locale], $strings);
				}

				$file = SQ_DATA_PATH.'/private/system/core/internal_messages.'.$this_locale;
				if (is_file($file)) {
					$strings = unserialize(file_get_contents($file));

					if (!isset($this->_internal_messages[$locale])) {
						$this->_internal_messages[$locale] = Array();
					}
					$this->_internal_messages[$locale] = array_merge($this->_internal_messages[$locale], $strings);
				}
			}
		}//end if

	}//end includeCoreStrings()


	/**
	* fetches an internal message subject translation for the current locale
	*
	* @param string	$type		the internal message type
	* @param array	$keywords	array of keywords
	* @param string	$locale		name of the locale
	*
	* @return void
	* @access public
	*/
	public function getInternalMessageSubject($type, $keywords, $locale=NULL)
	{
		if (is_null($locale)) {
			$locale = $this->getCurrentLocale();
		}

		$valid_code = TRUE;
		if (!isset($this->_internal_messages[$locale])) {
			$valid_code = FALSE;
		}

		if (!isset($this->_internal_messages[$locale][$type])) {
			$valid_code = FALSE;
		}

		if ($valid_code) {
			$subject = $this->_internal_messages[$locale][$type]['subject'];
			replace_keywords($subject, $keywords);
		} else {
			$subject = 'Internal message translation not found for this code and locale ['.$type.']';
		}

		return $subject;

	}//end getInternalMessageSubject()


	/**
	* fetches an internal message subject translation for the current locale
	*
	* @param string	$type		the internal message type
	* @param array	$keywords	array of keywords
	* @param string	$locale		name of the locale
	*
	* @return void
	* @access public
	*/
	public function getInternalMessageBody($type, $keywords, $locale=NULL)
	{
		if (is_null($locale)) {
			$locale = $this->getCurrentLocale();
		}

		$valid_code = TRUE;
		if (!isset($this->_internal_messages[$locale])) {
			$valid_code = FALSE;
		}

		if (!isset($this->_internal_messages[$locale][$type])) {
			$valid_code = FALSE;
		}

		if ($valid_code) {
			$body = $this->_internal_messages[$locale][$type]['body'];
			replace_keywords($body, $keywords);
		} else {
			$body    = 'Internal message translation not found for this code and locale ['.$type.']';
		}

		return $body;

	}//end getInternalMessageBody()


	/**
	* returns an array of js includes needed for the current locale
	*
	* @return void
	* @access public
	*/
	public function getJavascriptIncludes()
	{
		$res = Array(sq_web_path('lib').'/js/translation.js'); // we always need this one
		list($lang, $country, $variant) = $this->getLocaleParts($GLOBALS['SQ_SYSTEM']->lm->getCurrentLocale());
		if (file_exists(SQ_DATA_PATH.'/public/system/core/js_strings.'.$lang.'.js')) {
			$res[] = sq_web_path('data').'/system/core/js_strings.'.$lang.'.js';
		}
		if (!empty($country)) {
			if (file_exists(SQ_DATA_PATH.'/public/system/core/js_strings.'.$lang.'_'.$country.'.js')) {
				$res[] = sq_web_path('data').'/system/core/js_strings.'.$lang.'_'.$country.'.js';
			}

			if (!empty($variant)) {
				if (file_exists(SQ_DATA_PATH.'/public/system/core/js_strings.'.$lang.'_'.$country.'@'.$variant.'.js')) {
					$res[] = sq_web_path('data').'/system/core/js_strings.'.$lang.'_'.$country.'@'.$variant.'.js';
				}
			}
		}
		return $res;

	}//end getJavascriptIncludes()


}//end class
?>
