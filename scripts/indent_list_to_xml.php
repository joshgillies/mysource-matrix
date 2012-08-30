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
* $Id: indent_list_to_xml.php,v 1.5 2012/08/30 01:04:53 ewang Exp $
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
* @version $Revision: 1.5 $
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
			echo $indent.'<term name="'.htmlspecialchars(trim($item)).'">';
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
