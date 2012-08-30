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
* $Id: character_sets.inc,v 1.9 2012/08/30 00:56:51 ewang Exp $
*
*/


/**
* Standard Character Set List
*
* Purpose
*     This list of apparently standard character sets was taken from this web site:
*     http://msdn.microsoft.com/library/default.asp?url =>/workshop/author/dhtml/reference/charsets/charset4.asp
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.9 $
* @package Fudge
* @subpackage Standards_Lists
*/

$standards_lists_character_sets = Array('big5'								=> 'Chinese Traditional (Big5)',
										'cp866'								=> 'Cyrillic (DOS)',
										'koi8-r'							=> 'Cyrillic (KOI8-R)',
										'windows-1251'						=> 'Cyrillic (Windows)',
										'euc-jp'							=> 'Japanese (EUC)',
										'shift_jis'							=> 'Japanese (Shift-JIS)',
										'iso-8859-15'						=> 'Latin 9 (ISO)',
										'utf-8'								=> 'Unicode (UTF-8)',										
										'iso-8859-1'						=> 'Western European (ISO)',
										'Windows-1252'						=> 'Western European (Windows)',
										);

$standards_lists_tidy_char_sets = Array('ASMO-708'							=> '',
										'DOS-720'							=> '',
										'iso-8859-6'						=> '',
										'x-mac-arabic'						=> '',
										'windows-1256'						=> '',
										'ibm775'							=> '',
										'iso-8859-4'						=> '',
										'windows-1257'						=> '',
										'ibm852'							=> '',
										'iso-8859-2'						=> '',
										'x-mac-ce'							=> '',
										'windows-1250'						=> '',
										'EUC-CN'							=> '',
										'gb2312'							=> '',
										'hz-gb-2312'						=> '',
										'x-mac-chinesesimp'					=> '',
										'big5'								=> 'big5',
										'x-Chinese-CNS'						=> '',
										'x-Chinese-Eten'					=> '',
										'x-mac-chinesetrad'					=> '',
										'cp866'								=> '',
										'iso-8859-5'						=> '',
										'koi8-r'							=> '',
										'koi8-u'							=> '',
										'x-mac-cyrillic'					=> '',
										'windows-1251'						=> '',
										'x-IA5-German'						=> '',
										'ibm737'							=> '',
										'iso-8859-7'						=> '',
										'x-mac-greek'						=> '',
										'windows-1253'						=> '',
										'ibm869'							=> '',
										'DOS-862'							=> '',
										'iso-8859-8-i'						=> '',
										'iso-8859-8'						=> '',
										'x-mac-hebrew'						=> '',
										'windows-1255'						=> '',
										'x-EBCDIC-Arabic'					=> '',
										'x-EBCDIC-CyrillicRussian'			=> '',
										'x-EBCDIC-CyrillicSerbianBulgarian'	=> '',
										'x-EBCDIC-DenmarkNorway'			=> '',
										'x-ebcdic-denmarknorway-euro'		=> '',
										'x-EBCDIC-FinlandSweden'			=> '',
										'x-ebcdic-finlandsweden-euro'		=> '',
										'x-ebcdic-france-euro'				=> '',
										'x-EBCDIC-Germany'					=> '',
										'x-ebcdic-germany-euro'				=> '',
										'x-EBCDIC-GreekModern'				=> '',
										'x-EBCDIC-Greek'					=> '',
										'x-EBCDIC-Hebrew'					=> '',
										'x-EBCDIC-Icelandic'				=> '',
										'x-ebcdic-icelandic-euro'			=> '',
										'x-ebcdic-international-euro'		=> '',
										'x-EBCDIC-Italy'					=> '',
										'x-ebcdic-italy-euro'				=> '',
										'x-EBCDIC-JapaneseAndKana'			=> '',
										'x-EBCDIC-JapaneseAndJapaneseLatin'	=> '',
										'x-EBCDIC-JapaneseAndUSCanada'		=> '',
										'x-EBCDIC-JapaneseKatakana'			=> '',
										'x-EBCDIC-KoreanAndKoreanExtended'	=> '',
										'x-EBCDIC-KoreanExtended'			=> '',
										'CP870'								=> '',
										'x-EBCDIC-SimplifiedChinese'		=> '',
										'X-EBCDIC-Spain'					=> '',
										'x-ebcdic-spain-euro'				=> '',
										'x-EBCDIC-Thai'						=> '',
										'x-EBCDIC-TraditionalChinese'		=> '',
										'CP1026'							=> '',
										'x-EBCDIC-Turkish'					=> '',
										'x-EBCDIC-UK'						=> '',
										'x-ebcdic-uk-euro'					=> '',
										'ebcdic-cp-us'						=> '',
										'x-ebcdic-cp-us-euro'				=> '',
										'ibm861'							=> '',
										'x-mac-icelandic'					=> '',
										'x-iscii-as'						=> '',
										'x-iscii-be'						=> '',
										'x-iscii-de'						=> '',
										'x-iscii-gu'						=> '',
										'x-iscii-ka'						=> '',
										'x-iscii-ma'						=> '',
										'x-iscii-or'						=> '',
										'x-iscii-pa'						=> '',
										'x-iscii-ta'						=> '',
										'x-iscii-te'						=> '',
										'euc-jp'							=> '',
										'iso-2022-jp'						=> 'iso2022',
										'iso-2022-jp'						=> 'iso2022',
										'csISO2022JP'						=> 'iso2022',
										'x-mac-japanese'					=> '',
										'shift_jis'							=> '',
										'ks_c_5601-1987'					=> '',
										'euc-kr'							=> '',
										'iso-2022-kr'						=> 'iso2022',
										'Johab'								=> '',
										'x-mac-korean'						=> '',
										'iso-8859-3'						=> '',
										'iso-8859-15'						=> '',
										'x-IA5-Norwegian'					=> '',
										'IBM437'							=> '',
										'x-IA5-Swedish'						=> '',
										'windows-874'						=> '',
										'ibm857'							=> '',
										'iso-8859-9'						=> '',
										'x-mac-turkish'						=> '',
										'windows-1254'						=> '',
										'unicode'							=> '',
										'unicodeFFFE'						=> '',
										'utf-7'								=> '',
										'utf-8'								=> 'utf8',
										'us-ascii'							=> 'ascii',
										'windows-1258'						=> '',
										'ibm850'							=> '',
										'x-IA5'								=> '',
										'iso-8859-1'						=> 'latin1',
										'macintosh'							=> 'mac',
										'Windows-1252'						=> 'win1252',
										);
?>