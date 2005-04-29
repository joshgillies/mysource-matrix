<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: indent_list_to_xml.php,v 1.2 2005/04/29 05:39:49 gsherwood Exp $
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
* @version $Revision: 1.2 $
* @package MySource_Matrix
* @subpackage Lexicon
*/

/**
* Convert the specified indented list to a multi-dimensional array
* 
* @param array	&$lines	The lines of the text file to process
*
* @return array	The indented list represented as a multi-dimensional array
*/
function indent_list_to_array(&$lines)
{
	$res = Array();
	$indent_size = get_indent_size(current($lines));
	while (true) {
		$current_line = current($lines);
		//echo "$current_line \n";
		$line_content = trim($current_line);
		if (get_indent_size($current_line) < $indent_size) break;
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


/*
* Get the indent size (number of tabs) for the specified string
* 
* @param string	$s	The string to analyse
*
* @return int	The number of tabs before the first non-tab character
*/
function get_indent_size($s)
{
	$res = 0;
	while ($s{$res} == "\t") $res++;
	return $res;

}//end get_indent_size()


/*
* Print the supplied multi-dimensional array as XML (recursive call)
* 
* @param array	$res	The array to print
* @param string	$indent	The indent characters to put before each line of output
*
* @return void
*/
function print_array_xml_r($res, $indent='')
{
	if (empty($res)) {
		echo '(empty)';
	} else {
		foreach ($res as $item => $kids) {
			echo $indent.'<entity name="'.trim($item).'">';
			if (!empty($kids)) {
				echo "\n";
				echo $indent."\t".'<relation name="Category">'."\n";
				print_array_xml_r($kids, $indent."\t\t");
				echo $indent."\t".'</relation>'."\n";
				echo $indent;
			}
			echo '</entity>'."\n";
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
	<relations>
		<relation name="Category"/>
	</relations>
	<entities>
<?php
	$lines = file($argv[1]);
	reset($lines);
	$res = indent_list_to_array($lines, $res);
	print_array_xml_r($res, "\t\t");
?>
	</entities>
</thesaurus>
