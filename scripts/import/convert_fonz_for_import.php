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
* $Id: convert_fonz_for_import.php,v 1.3 2012/08/30 01:04:53 ewang Exp $
*
* Script to form valid thesaurus XML from a comma separated file in the form
* "parent_term","relation","child_term"
*
* @author  Elden McDonald <emcdonald@squiz.net
* @version $Revision: 1.3 $
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
