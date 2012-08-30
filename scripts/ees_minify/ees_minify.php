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
*/

/**
* Minify some of Matrix js files to core/lib/matrix.min.js. This is mainly for EES performance improvement.
 * 
* 
* @author  Edison Wang <ewang@squiz.com.au>
* @version $Revision: 1.3 $
* @package MySource_Matrix
*/

error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once 'jsmin.php';

$file = $SYSTEM_ROOT.'/core/lib/js/matrix.min.js';

$source_files = Array (
    $SYSTEM_ROOT.'/fudge/var_serialise/var_serialise.js',
    $SYSTEM_ROOT.'/core/lib/js/general.js',
    $SYSTEM_ROOT.'/core/lib/js/debug.js',
    $SYSTEM_ROOT.'/core/lib/js/layer_handler.js',
    $SYSTEM_ROOT.'/core/lib/html_form/html_form.js',
    $SYSTEM_ROOT.'/core/lib/js/detect.js',
    $SYSTEM_ROOT.'/core/assets/bodycopy/bodycopy/js/bodycopy_edit_divs.js',
    $SYSTEM_ROOT.'/core/assets/metadata/metadata_fields/metadata_field_select/js/metadata_field_select.js',
    $SYSTEM_ROOT.'/core/assets/metadata/metadata_fields/metadata_field_multiple_text/js/metadata_field_multiple_text.js',
    $SYSTEM_ROOT.'/core/assets/metadata/metadata_fields/metadata_field_hierarchy/js/metadata_field_hierarchy.js',
);

// Output a minified version
$string = '';
foreach ($source_files as $source) {
    echo 'processing '.$source."\n";
    $string .= JSMin::minify(file_get_contents($source));
}
$result = file_put_contents($file, $string);

if($result === FALSE) {
    trigger_error ("FAILED TO GENREATE MINIFIED JS FILES FOR EES");
}

echo "Minified js file is completed.\n";
echo $file. "\n";
?>
