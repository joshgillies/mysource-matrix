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
				trigger_localised_error('SYS0193', sprintf(translate('Locale code "%s" does not seem to be valid'), $locale), E_USER_WARNING);
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
	 * Get a plural string.
	 *
	 * This is the equivalent of ngettext() in the GNU Gettext system.
	 * In a locale there will be a header determining how many plurals there
	 * are and which plural is selected for which number.
	 * This can be important for languages where there may not be two number
	 * forms (some have one form for all, some may have three or more).
	 *
	 * If no locale is loaded, it will be treated as an English two number
	 * form, with $string being used if $number is 1, and $plural_str
	 * if $number is not 1. If $number is negative, the absolute is used.
	 *
	 * The intention is to be able to pass NULL as the $number, to represent
	 * positive infinity. Use this where an editable number is being presented as
	 * the replacement instead of a number.
	 *
     * @param string $string     The singular string.
     * @param string $plural_str The default plural string.
     * @param string $number     The number that determines plurality.
     *
     * @return string
	 */
	public function getPlural($string, $plural_str, $number)
	{
		$locale = $this->getCurrentLocale();
		$retval = NULL;
        if (isset($this->_strings[$locale])) {
            // TODO: Hook into not-yet-built plurality functions.
            $str_form = ($number === 1) ? 0 : 1;
            $string   = array_get_index($this->_strings[$locale], $string);

            if (is_array($string) === TRUE) {
            	// We have an array. Give them the correct string form.
            	// If it doesn't exist then default to the singular.
            	$retval = array_get_index($string, $str_form, $string[0]);
        	} else if ($string !== NULL) {
        		// We don't have an array, but we do have a string.
        		$retval = $string;
        	}
        }

        // No strings for this locale. Revert to single and plural dichotomy.
        if ($retval === NULL) {
        	if (($number !== NULL) && (abs($number) === 1)) {
            	$retval = $string;
        	} else {
        		$retval = $plural_str;
        	}
        }

        return $retval;

	}//end getPlural()


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


	/**
	 * Returns the available localisable strings.
	 *
	 * @param string $package The package to search through (omit or '__core__' for core)
	 *
	 * @return array
	 */
	public function getLocalisableStrings($package='__core__')
	{
		if ($package === '__core__') {
			$package_path = realpath(SQ_SYSTEM_ROOT.'/core');
		} else {
			$package_path = realpath(SQ_PACKAGES_PATH.'/'.$package);
		}

		// Get the list of PHP and edit interface files.
		$file_iterator   = new RecursiveDirectoryIterator($package_path);
		$recurs_iterator = new RecursiveIteratorIterator($file_iterator);
		$php_file_list   = new RegexIterator($recurs_iterator, '/^.+\.inc$/i', RecursiveRegexIterator::GET_MATCH);
		$js_file_list    = new RegexIterator($recurs_iterator, '/^.+\.js$/i', RecursiveRegexIterator::GET_MATCH);
		$ei_file_list    = new RegexIterator($recurs_iterator, '/^.+edit_interface_.+\.xml$/i', RecursiveRegexIterator::GET_MATCH);

		$match_list = Array();

		// Ordinary translate calls (both PHP and JS) in the PHP files.
		foreach ($php_file_list as $php_file) {
			$php_file = $php_file[0];
			$contents = file_get_contents($php_file);

			// Get all "translate" calls, ending in an *unescaped* closing quote
			preg_match_all('/(?<!js_)translate\(\'(.*?)(?<!\\\\)\'\)/m', $contents, $matches);
			$match_list = array_merge($match_list, $matches[1]);

			// Get all "js_translate" calls
			preg_match_all('/js_translate\(\'(.*?)(?<!\\\\)\'[,)]/m', $contents, $matches);
			$match_list = array_merge($match_list, $matches[1]);
		}

		// JS translate calls in the JS files.
		foreach ($js_file_list as $js_file) {
			$js_file = $js_file[0];
			$contents = file_get_contents($js_file);

			// Get all "translate" calls, ending in an *unescaped* closing quote
			preg_match_all('/js_translate\(\'(.*?)(?<!\\\\)\'[,)]/m', $contents, $matches);
			$match_list = array_merge($match_list, $matches[1]);
		}

		// TODO: XML edit interface files.
		foreach ($ei_file_list as $xml_file) {
			$xml_file = $xml_file[0];
			$xml_doc = DOMDocument::load($xml_file);
			$xpath   = new DOMXPath($xml_doc);

			$paths = Array(
				'//section/@name',
				'//section/@display_name',
				'//field/display_name',
				'//field/note',
				'//text/@empty_text',
				'//boolean/@true_text',
				'//boolean/@false_text',
				'//boolean/@empty_text',
				'//slider_bar/@lower_text',
				'//slider_bar/@upper_text',
				'//selection/@empty_text',
			);

			foreach ($paths as $path) {
				$results = $xpath->query($path);
				if ($results->length > 0) {
					//echo "\n".$xml_file.' '.$results->length;
					foreach ($results as $result) {
						if ($result instanceof DOMAttr === TRUE) {
							$match_list[] = $result->value;
						} else {
							$match_list[] = $result->nodeValue;
						}
					}
				}
			}
		}

		// Filter out duplicates
		$match_list = array_unique($match_list);

		// Unescape single quotes
		foreach ($match_list as &$match) {
			$match = str_replace('\\\'', '\'', $match);
		}
		unset($match);

		// Plurals in the PHP files.
		foreach ($php_file_list as $php_file) {
			$php_file = $php_file[0];
			$contents = file_get_contents($php_file);

			// Get all "translate_plural" calls
			preg_match_all('/translate_plural\(\'(.*?)(?<!\\\\)\',\s*\'(.*?)(?<!\\\\)\'\,/m', $contents, $matches, PREG_SET_ORDER);
			foreach ($matches as $match) {
				$match_list[] = Array(
					$match[1],
					$match[2]
				);
			}
		}

		return $match_list;

	}//end getLocalisableStrings()


	/**
	 * Build a PO file from a list of strings.
	 *
	 * @param array $strings  List of strings.
	 * @param array $headers  The headers to add to the PO file.
	 * @param int   $nplurals The number of plurals.
	 *
	 * @return string
	 */
	public function buildPoFile(Array $strings, $headers=Array(), $nplurals=2)
	{
		$file = '';

		ob_start();
		echo 'msgid ""'."\n";
		echo 'msgstr ""'."\n";
		foreach ($headers as $header => $value) {
			echo '"'.$header.': '.$value.'"'."\n";
		}

		echo "\n";

		foreach ($strings as $string) {
			if (is_array($string) === TRUE) {
				echo 'msgid "'.$string[0].'"'."\n";
				echo 'msgid_plural "'.$string[1].'"'."\n";
				for ($i = 0; $i < $nplurals; $i++) {
					echo 'msgstr['.$i.'] ""'."\n";
				}
			} else {
				echo 'msgid "'.$string.'"'."\n";
				echo 'msgstr ""'."\n";
			}

			echo "\n";
		}//end foreach

		$file = ob_get_clean();
		$file = trim($file);

		return $file;

	}//end buildPoFile()


	/**
	 * Build a PO file from a list of strings.
	 */
	public function buildXliffFile($strings, $plural_strings, $nplurals=2)
	{
		$file = '';

		ob_start();
		echo '<'.'?xml version="1.0" encoding="utf-8" ?'.'>';
		echo '<xliff version="1.2" xmlns="urn:oasis:names:tc:xliff:document:1.2">'."\n";
		echo '<file original="" datatype="plain" source-language="en" target-language="">'."\n";
		echo '<body>'."\n";
		/*foreach ($headers as $header => $value) {
			echo '"'.$header.': '.$value.'"';
		}*/

		$msgindex = 0;
		foreach ($strings as $source => $string) {
			$msgindex++;
			if (is_array($string) === TRUE) {
				echo '<group restype="x-gettext-plural">'."\n";
				echo '<trans-unit id="'.$msgindex.'[0]" xml:space="preserve" restype="x-singular-form">'."\n";
				echo '<source>'.htmlentities($source, ENT_XML1)."</source>\n";
				echo '<target>'.htmlentities(array_get_index($string, 0, ''), ENT_XML1).'</target>'."\n";
				echo '</trans-unit>'."\n";
				for ($i = 1; $i < $nplurals; $i++) {
					echo '<trans-unit id="'.$msgindex.'['.$i.']" xml:space="preserve" restype="x-plural-form-'.$i.'">'."\n";
					echo '<source>'.htmlentities($plural_strings[$source], ENT_XML1).'</source>'."\n";
					echo '<target>'.htmlentities(array_get_index($string, $i, ''), ENT_XML1).'</target>'."\n";
					echo '</trans-unit>'."\n";
				}
				echo '</group>'."\n";
			} else {
				echo '<trans-unit id="'.$msgindex.'" xml:space="preserve">'."\n";
				echo '<source>'.htmlentities($source, ENT_XML1).'</source>'."\n";
				echo '<target>'.htmlentities($string, ENT_XML1).'</target>'."\n";
				echo '</trans-unit>'."\n";
			}
		}//end foreach

		echo '</body>'."\n";
		echo '</file>'."\n";
		echo '</xliff>';

		$file = ob_get_clean();
		$file = trim($file);

		return $file;

	}//end buildXliffFile()


	/**
	 * Takes a PO string file from the specified package (or core) and turns it
	 * into a Matrix serialised string
	 *
	 * Package name of "__core__" will build the strings of the Core package.
	 * Returns 0 on success, 1 if failed, -1 if nothing was found.
	 *
	 * @param string $package   The name of the package.
	 * @param string $locale    The specific locale to build (NULL = all of them).
	 *
	 * @return integer
	 */
	public function extractSourcesFromPoFile($package, $locale)
	{
	    if ($package === '__core__') {
	        $path = realpath(SQ_SYSTEM_ROOT).'/core/locale';
	    } else {
	        $path = realpath(SQ_SYSTEM_ROOT).'/packages/'.$package.'/locale';
	    }

	    $ok = 0;

        $source_filename = $path.'/'.$locale.'.po';
        if (file_exists($source_filename) === FALSE) {
        	return Array();
        }

        $lines = file($source_filename);
		
        $strings = Array();
        $current      = NULL;
        $line_index   = NULL;
        $flags        = NULL;
        $msgid        = NULL;
        $msgid_plural = NULL;
        $msgstr       = NULL;

        // Create closure to build the strings file.
        $add_to_strings = function($msgid, $msgid_plural=NULL) use (&$strings) {
        	if ($msgid_plural === NULL || $msgid_plural === '') {
            	$strings[] = $msgid;
        	} else {
            	$strings[] = Array($msgid, $msgid_plural);
        	}
        };

        foreach ($lines as $i => $line) {
            $line = trim($line);
            if (strpos($line, 'msgid "') === 0) {
                // Message ID.
                $current = 'msgid';
                $add_to_strings($msgid, $msgid_plural);
                if (empty($msgid) === FALSE) {
                    $flags = '';
                }
                $msgid  = '';
                $msgid_plural = '';
                $msgstr = '';
                $this_str = substr($line, strlen('msgid "'), -1);
            } else if (strpos($line, 'msgstr[') === 0) {
                // Plural option.
                list($line_index, $this_str) = sscanf($line, 'msgstr[%d] %s');
                $current = 'msgstr_plural';
                $msgstr  = Array();
                $this_str = substr(trim($this_str), -1, 1);
            } else if (strpos($line, 'msgid_plural "') === 0) {
                // Plural.
                $current  = 'msgid_plural';
                $msgid_plural = '';
                $this_str = substr($line, strlen('msgid_plural "'), -1);
            } else if (strpos($line, 'msgstr "') === 0) {
                // Single translation.
                $current  = 'msgstr';
                $msgstr   = '';
                $this_str = substr($line, strlen('msgstr "'), -1);
            } else if (strpos($line, '"') === 0) {
                // Continuation of previous string.
                $this_str = substr($line, -1, 1);
            } else {
                $this_str = '';
            }

            // Normalise the string: handle any embedded newlines (remember
            // actual LFs don't count), unescape double quotes and backslashes.
            $this_str = str_replace('\\n', "\n", $this_str);
            $this_str = str_replace('\\"', '"', $this_str);
            $this_str = str_replace('\\\\', '\\', $this_str);

            switch ($current) {
                case 'msgid':
                    $msgid .= $this_str;
                break;

                case 'msgid_plural':
                    $msgid_plural .= $this_str;
                break;

                case 'msgstr':
                    $msgstr .= $this_str;
                break;

                case 'msgstr_plural':
				    if (array_key_exists($line_index, $msgstr) === FALSE) {
					$msgstr[$line_index] = '';
				    }
                    $msgstr[$line_index] .= $this_str;
                break;
            }//end switch
        }//end foreach

        $add_to_strings($msgid, $msgid_plural);

	    return $strings;

	}//end extractSourcesFromPoFile()


	/**
	 * Takes a PO string file from the specified package (or core) and turns it
	 * into a Matrix serialised string
	 *
	 * Package name of "__core__" will build the strings of the Core package.
	 * Returns 0 on success, 1 if failed, -1 if nothing was found.
	 *
	 * @param string $package   The name of the package.
	 * @param string $locale    The specific locale to build (NULL = all of them).
	 *
	 * @return array
	 */
	public function extractPoFile($package, $locale)
	{
	    if ($package === '__core__') {
	        $path = realpath(SQ_SYSTEM_ROOT).'/core/locale';
	    } else {
	        $path = realpath(SQ_SYSTEM_ROOT).'/packages/'.$package.'/locale';
	    }

	    $ok = 0;

        $source_filename = $path.'/'.$locale.'.po';
        if (file_exists($source_filename) === FALSE) {
        	return Array();
        }

        $lines = file($source_filename);
		
        $strings = Array();
        $current    = NULL;
        $line_index = NULL;
        $flags      = NULL;
        $msgid      = NULL;
        $msgstr     = NULL;

        // Create closure to build the strings file.
        $add_to_strings = function($msgid, $msgstr) use (&$strings) {
        	if (($msgstr !== '') && ($msgid !== '')) {
            	$strings[$msgid] = $msgstr;
        	}
        };

        foreach ($lines as $i => $line) {
            $line = trim($line);
            if (strpos($line, 'msgid "') === 0) {
                // Message ID.
                $current = 'msgid';
                $add_to_strings($msgid, $msgstr);
                if (empty($msgid) === FALSE) {
                    $flags = '';
                }
                $msgid  = '';
                $msgstr = '';
                $this_str = substr($line, strlen('msgid "'), -1);
            } else if (strpos($line, 'msgstr[') === 0) {
                // Plural option.
                list($line_index, $this_str) = sscanf($line, 'msgstr[%d] %s');
                $current = 'msgstr_plural';
                $msgstr  = Array();
                $this_str = substr(trim($this_str), -1, 1);
            } else if (strpos($line, 'msgstr "') === 0) {
                // Single translation.
                $current  = 'msgstr';
                $msgstr   = '';
                $this_str = substr($line, strlen('msgstr "'), -1);
            } else if (strpos($line, '"') === 0) {
                // Continuation of previous string.
                $this_str = substr($line, -1, 1);
            } else {
                $this_str = '';
            }

            // Normalise the string: handle any embedded newlines (remember
            // actual LFs don't count), unescape double quotes and backslashes.
            $this_str = str_replace('\\n', "\n", $this_str);
            $this_str = str_replace('\\"', '"', $this_str);
            $this_str = str_replace('\\\\', '\\', $this_str);

            switch ($current) {
                case 'msgid':
                    $msgid .= $this_str;
                break;

                case 'msgstr':
                    $msgstr .= $this_str;
                break;

                case 'msgstr_plural':
				    if (array_key_exists($line_index, $msgstr) === FALSE) {
					$msgstr[$line_index] = '';
				    }
                    $msgstr[$line_index] .= $this_str;
                break;
            }//end switch
        }//end foreach

        $add_to_strings($msgid, $msgstr);

	    return $strings;

	}//end extractPoFile()


	/**
	 * Takes a PO string file from the specified package (or core) and turns it
	 * into a Matrix serialised string
	 *
	 * Package name of "__core__" will build the strings of the Core package.
	 * Returns 0 on success, 1 if failed, -1 if nothing was found.
	 *
	 * @param string $package   The name of the package.
	 * @param string $locale    The specific locale to build (NULL = all of them).
	 *
	 * @return integer
	 */
	public function buildSerializedStringsFile($package, $locale=NULL)
	{
	    $ok = 0;
	    $build_locales = Array();

	    if ($locale === NULL) {
	        $files = glob($path.'/*.po');
	        foreach ($files as $file) {
	            $build_locales[] = basename($file, '.po');
	        }

	        if (count($build_locales) === 0) {
	            $ok = -1;
	        }
	    } else {
	        if (is_file($path.'/'.$locale.'.po') === FALSE) {
	            $ok = -1;
	        } else {
	            $build_locales = Array($locale);
	        }
	    }

	    foreach ($build_locales as $locale) {
	        $target_filename = SQ_DATA_PATH.'/private/packages/'.$package.'/strings.'.$locale;
	        $lines = file($source_filename);

			if (!file_exists(SQ_DATA_PATH.'/private/packages/'.$package)) {
				mkdir(SQ_DATA_PATH.'/private/packages/'.$package);
			}

			$strings = $this->extractPoFile($package, $locale);
	        file_put_contents($target_filename, serialize($strings));
	    }//end foreach

	    return $ok;
	}//end build_string_file()


	/**
	 * Takes a PO string file from all packages and turns it into
	 * a JavaScript locale file stored in data/public.
	 *
	 * This scans the PO files for any strings marked with the flag "js-translate"
	 * and adds them to the translated strings JS file.
	 * Flags are lines before msgid lines starting with "#,".
	 * This is to be done at the end of step 3, after all packages are installed.
	 *
	 * @return integer
	 */
	public function buildJavascriptStringFile()
	{
	    $packages = $GLOBALS['SQ_SYSTEM']->getInstalledPackages();
	    $locales  = Array();

	    $build_files = Array();
	    foreach ($packages as $package) {
	        if ($package['code_name'] === '__core__') {
	            $path = realpath(SQ_SYSTEM_ROOT).'/core/locale';
	        } else {
	            $path = realpath(SQ_SYSTEM_ROOT).'/packages/'.$package['code_name'].'/locale';
	        }

	        $files = glob($path.'/*.po');
	        foreach ($files as $file) {
	            $build_files[] = Array(
	                'filename' => $file,
	                'locale'   => basename($file, '.po'),
	            );
	        }
	    }//end foreach

	    $strings = Array();
	    $ok      = 0;

	    foreach ($build_files as $source) {
	        $source_filename = $source['filename'];
	        $locale          = $source['locale'];
	        $lines = file($source_filename);

	        if (array_key_exists($locale, $strings) === FALSE) {
	            $strings[$locale] = Array();
	        }

	        $current    = NULL;
	        $flags      = NULL;
	        $line_index = NULL;
	        $msgid      = NULL;
	        $msgstr     = NULL;

	        // Create closure to build the strings array.
	        $add_to_strings = function($msgid, $msgstr, $flags) use (&$strings, $locale) {
	            if ((empty($msgid) === FALSE) && (empty($flags) === FALSE) && (strpos($flags, 'js-translate') !== FALSE)) {
	                $strings[$locale][$msgid] = $msgstr;
	            }
	        };

	        foreach ($lines as $line) {
	            $line = trim($line);
	            if (strpos($line, '#, ') === 0) {
	                $add_to_strings($msgid, $msgstr, $flags);
	                $msgid  = '';
	                $msgstr = '';
	                $flags = substr($line, 3);
	            } else if (strpos($line, 'msgid "') === 0) {
	                // Message ID.
	                $current = 'msgid';
	                if (empty($msgid) == FALSE) {
	                    $add_to_strings($msgid, $msgstr, $flags);
	                    $flags = '';
	                }
	                $msgid  = '';
	                $msgstr = '';
	                $this_str = substr($line, strlen('msgid "'), -1);
	            } else if (strpos($line, 'msgstr[') === 0) {
	                // Plural option.
	                list($line_index, $this_str) = sscanf($line, 'msgstr[%d] %s');
	                $current = 'msgstr_plural';
	                $msgstr  = Array();
	                $this_str = substr(trim($this_str), -1, 1);
	            } else if (strpos($line, 'msgstr "') === 0) {
	                // Single translation.
	                $current  = 'msgstr';
	                $msgstr   = '';
	                $this_str = substr($line, strlen('msgstr "'), -1);
	            } else if (strpos($line, '"') === 0) {
	                // Continuation of previous string.
	                $this_str = substr($line, -1, 1);
	            } else {
	                $this_str = '';
	            }

	            // Normalise the string: handle any embedded newlines (remember
	            // actual LFs don't count), unescape double quotes and backslashes.
	            $this_str = str_replace('\\n', "\n", $this_str);
	            $this_str = str_replace('\\"', '"', $this_str);
	            $this_str = str_replace('\\\\', '\\', $this_str);

	            switch ($current) {
	                case 'msgid':
	                    $msgid .= $this_str;
	                break;

	                case 'msgstr':
	                    $msgstr .= $this_str;
	                break;

	                case 'msgstr_plural':
					    if (array_key_exists($line_index, $msgstr) === FALSE) {
							$msgstr[$line_index] = '';
					    }
	                    $msgstr[$line_index] .= $this_str;
	                break;
	            }//end switch
	        }//end foreach

	        $add_to_strings($msgid, $msgstr, $flags);
	    }//end foreach

	    foreach ($strings as $locale => $locale_strings) {
	        if (count($locale_strings) > 0) {
	            $js_file  = 'SQ_CURRENT_LOCALE = \''.$locale.'\';'."\n";
	            $js_file .= 'translated_strings[\''.$locale.'\'] = {'."\n";
	            foreach ($locale_strings as $msgid => $msgstr) {
	                $js_file .= '\''.addslashes($msgid).'\': \''.addslashes($msgstr).'\','."\n";
	            }

	            // Pull out the last comma from the file.
	            $js_file .= '};'."\n";
	            $js_file = str_replace(','."\n".'}', "\n".'}', $js_file);

	            $target_filename = SQ_DATA_PATH.'/public/system/core/js_strings.'.$locale.'.js';
	            create_directory(dirname($target_filename));
	            file_put_contents($target_filename, $js_file);
	        }
	    }

	    return $ok;

	}//end build_string_file()


}//end class
?>
