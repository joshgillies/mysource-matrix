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
* $Id: convert_fonz_for_import.php,v 1.1 2006/10/16 04:28:26 emcdonald Exp $
*
* Script to form valid thesaurus XML from a comma separated file in the form
* "parent_term","relation","child_term"
*
* @author  Elden McDonald <emcdonald@squiz.net
* @version $Revision: 1.1 $
* @package MySource_Matrix
* @subpackage __core__
*/



if ($argc != 2) {
	echo "Usage: php convert_fonz_for_import inputfile.txt > outputfile.xml \n";
	exit();
}
?>
<thesaurus>
<?php
$handle = fopen($argv[1], 'r');
while ((list($parent_term, $relation_name, $child_term) = fgetcsv($handle, 1000, ',', '"')) !== FALSE) {
	// do not include definitions or scope notes because they are paragraphs
	if (($relation_name != 'Definition') && ($relation_name != 'Scope Note')) {
		echo '<term name="'.$parent_term.'">'."\n";
		echo '<relation name="'.$relation_name.'">'."\n";
		echo '<term name="'.$child_term.'"/>'."\n";
		echo '</relation>'."\n";
		echo '</term>'."\n";
	}
}
fclose($handle);
?>
</terms>
</thesaurus>
