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
* $Id: indent_list_to_xml.php,v 1.1.4.1 2006/11/29 05:54:20 arailean Exp $
*
* Script to form valid thesaurus XML from a tab-indented text file like this:
*
* Software
*	CMSes
*		MySource Classic
*		MySource Matrix
*	OSes
*		Windows XP
*		Red Hat Linux
* Hardware
*	CPUs
*		2.4GHz Celeron
*		400MHz Pentiun 2
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.1.4.1 $
* @package MySource_Matrix
* @subpackage __core__
*/


/**
* Convert the specified indented list to a multi-dimensional array
*
* Returns the indented list represented as a multi-dimensional array
*
* @param array	&$lines	The lines of the text file to process
*
* @return array
* @access public
*/
function indent_list_to_array(&$lines)
{
	$res = Array();
	$indent_size = get_indent_size(current($lines));
	while (true) {
		$current_line = current($lines);
		//echo "$current_line \n";
		$line_content = trim($current_line);
		if (get_indent_size($current_line) < $indent_size) {
			break;
		}
		if (false === next($lines)) break;
		if (empty($line_content)) continue;
		if (get_indent_size(current($lines)) > $indent_size) {
			$res[$current_line] = indent_list_to_array($lines);
		} else {
			$res[$current_line] = Array();
		}
	}
	return $res;

}//end indent_list_to_array()


/**
* Get the indent size (number of tabs) for the specified string
*
* Returns the number of tabs before the first non-tab character
*
* @param string	$s	The string to analyse
*
* @return int
* @access public
*/
function get_indent_size($s)
{
	$res = 0;
	while ($s{$res} == "\t") {
		$res++;
	}
	return $res;

}//end get_indent_size()


/**
* Print the supplied multi-dimensional array as XML (recursive call)
*
* @param array	$res	The array to print
* @param string	$indent	The indent characters to put before each line of output
*
* @return void
* @access public
*/
function print_array_xml_r($res, $indent='')
{
	if (empty($res)) {
		echo '(empty)';
	} else {
		foreach ($res as $item => $kids) {
			echo $indent.'<term name="'.trim($item).'">';
			if (!empty($kids)) {
				echo "\n";
				echo $indent."\t".'<relation name="Category">'."\n";
				print_array_xml_r($kids, $indent."\t\t");
				echo $indent."\t".'</relation>'."\n";
				echo $indent;
			}
			echo '</term>'."\n";
		}
	}

}//end print_array_xml_r()


// MAIN //
if ($argc != 2) {
	echo "Usage: php indent_list_to_xml inputfile.txt > outputfile.xml \n";
	exit();
}
?>
<thesaurus>
<?php
	$lines = file($argv[1]);
	reset($lines);
	$res = indent_list_to_array($lines, $res);
	print_array_xml_r($res, "\t\t");
?>
</thesaurus>
