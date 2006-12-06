<?php
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
* $Id: csv_to_html_tree.php,v 1.4 2006/12/06 05:42:21 bcaldwell Exp $
*
*/

/**
* CSV site structure to HTML tree script
* Command-line only
*
* @author  Mark Brydon <mbrydon@squiz.net>
* 21 Nov 2006
*
*
* Purpose:
* 		Conversion of a CSV file into a tree structure where each level is represented
* 		by an HTML heading (eg; <h1>First level</h1><h2>Second level</h2>)
*
* 		eg; CSV file input:
*				Fred,John
*				Fred,John,Mary
*				Fred,Jack
*				Fred,Jack,Peter,Jill
*				Arnie
*				Arnie,Connor
*
*      	Output from this script:
*	      		<h1>Fred</h1>
*        			<h2>John</h2>
*          			<h3>Mary</h3>
*      			<h2>Jack</h2>
*						<h3>Peter</h3>
*							<h4>Jill</h4>
*				<h1>Arnie</h1>
*					<h2>Connor</h2>
*
*/


/**
* Prints out some basic help info detailing how to use this script
*
* @return void
* @access public
*/
function printUsage()
{
	echo "CSV to HTML structure tree converter\n\n";
	echo "Usage: csv_to_html_tree [csv file]\n";
	echo "csv file: A comma separated values file that represents the site structure\n\n";

}//end printUsage()


/************************** MAIN PROGRAM ****************************/

if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

// Has a filename been supplied?
$csv_filename	= $argv[1];
if (empty($csv_filename)) {
	printUsage();
	echo "* A CSV filename must be specified as the first parameter\n\n";
	return -1;
}

// Does the supplied file exist?
$fd = fopen($csv_filename, 'r');
if (!$fd) {
	printUsage();
	echo "* The supplied CSV file was not found\n\n";
	return -2;
}

// Yippee, we have a file. Let's process it now
$last_html_tree	= Array();

while (($data = fgetcsv($fd, 1024, ',')) !== FALSE) {
	// The level we have stored already (eg; heading 1 = 1)
	$current_level = 0;
	$num_fields = count($data);

	// Grab a line and throw it into an HTML array
	$html_tree = Array();
	foreach ($data as $key => $val) {
		$current_level++;
		$html_tree[$current_level] = $val;
	}

	// Cycle through the line, omitting any levels we have already covered
	foreach ($html_tree as $level => $page_name) {
		$current_page = $html_tree[$level];

		if (!empty($page_name)) {
			if ($page_name != $last_html_tree[$level]) {
				// Write out the heading tag, trimming any spaces surrounding the page name
				echo '<h'.$level.'>'.trim($page_name).'</h'.$level.">\n";
			}
		}

	}

	// Remember what we saw for the next line
	$last_html_tree = $html_tree;
}
fclose($fd);


// That's all folks :-D

?>
