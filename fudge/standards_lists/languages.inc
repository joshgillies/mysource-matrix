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
* $Id: languages.inc,v 1.4 2003/11/26 00:51:19 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


/**
* Standard Language List
*
* Purpose
*     An alphabetical list of each language and its official
*     two-character, taken from ISO 639. (Yes, its in English)
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage Standards_Lists
*/

$standards_lists_languages = Array( 'ab' => 'Abkhazian',
									'aa' => 'Afar',
									'af' => 'Afrikaans',
									'sq' => 'Albanian',
									'am' => 'Amharic',
									'ar' => 'Arabic',
									'hy' => 'Armenian',
									'as' => 'Assamese',
									'ay' => 'Aymara',
									'az' => 'Azerbaijani',
									'ba' => 'Bashkir',
									'eu' => 'Basque',
									'bn' => 'Bengali; Bangla',
									'dz' => 'Bhutani',
									'bh' => 'Bihari',
									'bi' => 'Bislama',
									'br' => 'Breton',
									'bg' => 'Bulgarian',
									'my' => 'Burmese',
									'be' => 'Byelorussian',
									'km' => 'Cambodian',
									'ca' => 'Catalan',
									'zh' => 'Chinese',
									'co' => 'Corsican',
									'hr' => 'Croatian',
									'cs' => 'Czech',
									'da' => 'Danish',
									'nl' => 'Dutch',
									'en' => 'English',
									'eo' => 'Esperanto',
									'et' => 'Estonian',
									'fo' => 'Faroese',
									'fj' => 'Fiji',
									'fi' => 'Finnish',
									'fr' => 'French',
									'fy' => 'Frisian',
									'gl' => 'Galician',
									'ka' => 'Georgian',
									'de' => 'German',
									'el' => 'Greek',
									'kl' => 'Greenlandic',
									'gn' => 'Guarani',
									'gu' => 'Gujarati',
									'ha' => 'Hausa',
									'he' => 'Hebrew',
									'hi' => 'Hindi',
									'hu' => 'Hungarian',
									'is' => 'Icelandic',
									'id' => 'Indonesian',
									'ia' => 'Interlingua',
									'ie' => 'Interlingue',
									'iu' => 'Inuktitut',
									'ik' => 'Inupiak',
									'ga' => 'Irish',
									'it' => 'Italian',
									'ja' => 'Japanese',
									'jw' => 'Javanese',
									'kn' => 'Kannada',
									'ks' => 'Kashmiri',
									'kk' => 'Kazakh',
									'rw' => 'Kinyarwanda',
									'ky' => 'Kirghiz',
									'rn' => 'Kirundi',
									'ko' => 'Korean',
									'ku' => 'Kurdish',
									'lo' => 'Laothian',
									'la' => 'Latin',
									'lv' => 'Latvian, Lettish',
									'ln' => 'Lingala',
									'lt' => 'Lithuanian',
									'mk' => 'Macedonian',
									'mg' => 'Malagasy',
									'ms' => 'Malay',
									'ml' => 'Malayalam',
									'mt' => 'Maltese',
									'mi' => 'Maori',
									'mr' => 'Marathi',
									'mo' => 'Moldavian',
									'mn' => 'Mongolian',
									'na' => 'Nauru',
									'ne' => 'Nepali',
									'no' => 'Norwegian',
									'oc' => 'Occitan',
									'or' => 'Oriya',
									'om' => '(Afan) Oromo',
									'ps' => 'Pashto, Pushto',
									'fa' => 'Persian',
									'pl' => 'Polish',
									'pt' => 'Portuguese',
									'pa' => 'Punjabi',
									'qu' => 'Quechua',
									'rm' => 'Rhaeto-Romance',
									'ro' => 'Romanian',
									'ru' => 'Russian',
									'sm' => 'Samoan',
									'sg' => 'Sangho',
									'sa' => 'Sanskrit',
									'gd' => 'Scots Gaelic',
									'sr' => 'Serbian',
									'sh' => 'Serbo-Croatian',
									'st' => 'Sesotho',
									'tn' => 'Setswana',
									'sn' => 'Shona',
									'sd' => 'Sindhi',
									'si' => 'Sinhalese',
									'ss' => 'Siswati',
									'sk' => 'Slovak',
									'sl' => 'Slovenian',
									'so' => 'Somali',
									'es' => 'Spanish',
									'su' => 'Sundanese',
									'sw' => 'Swahili',
									'sv' => 'Swedish',
									'tl' => 'Tagalog',
									'tg' => 'Tajik',
									'ta' => 'Tamil',
									'tt' => 'Tatar',
									'te' => 'Telugu',
									'th' => 'Thai',
									'bo' => 'Tibetan',
									'ti' => 'Tigrinya',
									'to' => 'Tonga',
									'ts' => 'Tsonga',
									'tr' => 'Turkish',
									'tk' => 'Turkmen',
									'tw' => 'Twi',
									'ug' => 'Uighur',
									'uk' => 'Ukrainian',
									'ur' => 'Urdu',
									'uz' => 'Uzbek',
									'vi' => 'Vietnamese',
									'vo' => 'Volapuk',
									'cy' => 'Welsh',
									'wo' => 'Wolof',
									'xh' => 'Xhosa',
									'yi' => 'Yiddish',
									'yo' => 'Yoruba',
									'za' => 'Zhuang',
									'zu' => 'Zulu'
									);
?>