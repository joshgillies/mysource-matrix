/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: translation.js,v 1.3 2006/12/05 05:10:21 bcaldwell Exp $
*
*/


var SQ_CURRENT_LOCALE = null;

var translated_strings = new Array();


/**
* Splits up the locale passed into language, country, and variant and return
* each part individually
*
* This will return an array of between one and three elements, depending on
* what parts of the locale are in there. For example, if locale = 'fr_FR@euro',
* this will return ['fr', 'FR', 'euro']
*
* @param string	locale	the locale that we are going to split
*
* @return Array
*/
function get_locale_parts(locale)
{
	var locale_parts = new Array();

	// split language from the rest
	var lang_pos = locale.indexOf('_');

	if (lang_pos == -1) {
		locale_parts[0] = locale;
		return locale_parts;
	}
	locale_parts[0] = locale.substring(0, lang_pos - 1);

	locale = locale.substring(lang_pos + 1, locale.length);

	// split variant away from the rest
	var variant_pos = locale.indexOf('@');
	if (variant_pos == -1) {
		locale_parts[1] = locale;
	} else {
		locale_parts[1] = locale.substring(0, variant_pos - 1);
		locale_parts[2] = locale.substring(variant_pos + 1, locale.length);
	}
	return locale_parts;

}//end get_locale_parts()


/**
* Splits up the locale passed into language, country, and variant and return
* an array consisting of all possible partial locales
*
* This will return an array of between one and three elements, depending on
* what parts of the locale are in there. For example, if locale = 'fr_FR@euro',
* this will return ['fr', 'fr_FR', 'fr_FR@euro']
*
* @param string	locale	the locale that we are going to split
*
* @return Array
*/
function get_cumulative_locale_parts(locale)
{
	var locale_parts = get_locale_parts(locale);
	var cum_locale_parts = Array();
	cum_locale_parts.push(locale_parts[0]);

	if (locale_parts.length >= 2) {
		cum_locale_parts.push(cum_locale_parts[0] + '_' + locale_parts[1]);

		if (locale_parts.length >= 3) {
			cum_locale_parts.push(cum_locale_parts[1] + '@' + locale_parts[2]);
		}
	}

	return cum_locale_parts;

}//end get_cumulative_locale_parts()


/**
* Translates a localisable string found in JavaScript code
*
* @param string	string_code	the string code to translate
*
* @return string
*/
function js_translate()
{
	var locale_parts = get_cumulative_locale_parts(SQ_CURRENT_LOCALE);
	var found_string = null;

	var replacements = [];

	for( i = 1; i < arguments.length; i++) {
		replacements.push(arguments[i]);
	}

	while(locale_parts.length > 0) {
		locale = locale_parts.pop();
		if(!translated_strings[locale][arguments[0]]) continue;
		return vsprintf(translated_strings[locale][arguments[0]], replacements);
	}

	// no string code to be found anywhere
	return 'string code '+arguments[0]+' not found';

}//end js_translate();
