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
* $Id: translation.js,v 1.4 2012/08/30 01:09:21 ewang Exp $
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
	if (!locale) {
		return [];
	}

	var locale_parts = new Array();

	// split language from the rest
	var lang_pos = locale.indexOf('_');

	if (lang_pos == -1) {
		locale_parts[0] = locale;
		return locale_parts;
	}
	locale_parts[0] = locale.substr(0, lang_pos);

	locale = locale.substr(lang_pos + 1, locale.length);

	// split variant away from the rest
	var variant_pos = locale.indexOf('@');
	if (variant_pos == -1) {
		locale_parts[1] = locale;
	} else {
		locale_parts[1] = locale.substr(0, variant_pos);
		locale_parts[2] = locale.substr(variant_pos + 1, locale.length);
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

	if (locale_parts.length >= 1) {
		cum_locale_parts.push(locale_parts[0]);
	}

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
	return vsprintf(arguments[0], replacements);

}//end js_translate();

/**
* Translates a localisable string found in JavaScript code
*
* @param string	string_code	the string code to translate
*
* @return string
*/
function js_translate_plural()
{
	var locale_parts = get_cumulative_locale_parts(SQ_CURRENT_LOCALE);
	var found_string = null;

	var replacements = [];
	
	// TODO: Replace this with handling of plural forms as built in step 3.
	var str_form = 0;
	if (arguments[2] !== 1) {
		str_form = 1;
	}

	for(i = 3; i < arguments.length; i++) {
		replacements.push(arguments[i]);
	}

	while(locale_parts.length > 0) {
		locale = locale_parts.pop();
		if(!translated_strings[locale][arguments[0]]) continue;
		return vsprintf(translated_strings[locale][arguments[0]][str_form], replacements);
	}

	// no string code to be found anywhere
	if (arguments[2] === 1) {
		return vsprintf(arguments[0], replacements);
	} else {
		return vsprintf(arguments[1], replacements);
	}

}//end js_translate_plural()
