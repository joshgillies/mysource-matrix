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
* $Id: get_design_area_setable_attrs.php,v 1.1 2003/12/12 14:54:39 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Small script to return the design areas setable attributes and their descriptions
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
error_reporting(E_ALL);
$SYSTEM_ROOT = '';
// from cmd line
if ((php_sapi_name() == 'cli')) {
	if (isset($_SERVER['argv'][1])) $SYSTEM_ROOT = $_SERVER['argv'][1];
	$err_msg = "You need to supply the path to the System Root as the first argument\n";

} else {
	if (isset($_GET['SYSTEM_ROOT'])) $SYSTEM_ROOT = $_GET['SYSTEM_ROOT'];
	$err_msg = '
	<div style="background-color: red; color: white; font-weight: bold;">
		You need to supply the path to the System Root as a query string variable called SYSTEM_ROOT
	</div>
	';
}

if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error($err_msg, E_USER_ERROR);
}

// Dont set SQ_INSTALL flag before this include because we want
// a complete load now that the database has been created
require_once $SYSTEM_ROOT.'/core/include/init.inc';


$am = &$GLOBALS['SQ_SYSTEM']->am;

$design_area_types = $am->getTypeDescendants('design_area');
$design_types = $am->getTypeDescendants('design');
$design_types[] = 'design';

//pre_echo($design_area_types);
//pre_echo($design_types);

$design_areas = array_diff($design_area_types, $design_types);
sort($design_areas);

//pre_echo($design_areas);

if (!SQ_PHP_CLI) {
?>
<html>
<style type="text/css">
	body, table, td, th { 
		font-family: verdana, arial, sans-serif;
		font-size: 9pt;
		background-color: #ffffff;
	}

	table {
		width: 100%;
		border: 1px solid;
	}
	th, td {
		vertical-align: top;
		text-align: left;
	}
	th.type-code {
		color: #ffffff;
		background-color: #000000;
		padding: 10px;
	}

</style>
<body>
<?php
}// endif

foreach($design_areas as $type_code) {

	$am->includeAsset($type_code);
	$da = new $type_code();
	$edit_fns = $da->getEditFns();

	$setable_vars = $edit_fns->_getSetableVars($da);

	#pre_echo($type_code.' : '.print_r($setable_vars, 1));

	if (SQ_PHP_CLI) {
		echo $type_code, " ", str_repeat('-', 80 - (strlen($type_code) + 1)), "\n";
	
	} else {
	?>
		<table>
			<tr>
				<th class="type-code" colspan="3"><?php echo $type_code; ?></th>
			</tr>
			<tr>
				<th width="20%">Var Name</th>
				<th width="10%">Type</th>
				<th width="70%">Description</th>
			</tr>
	<?php
	}// endif

	ksort($setable_vars);

	foreach($setable_vars as $var_name => $info) {
		$attr = $da->getAttribute($var_name);
		$desc = $attr->description;

		if ($info['type'] == 'selection') {
			if (!SQ_PHP_CLI) $desc .= '<pre>';
			else $desc .= "\n";
			$desc .= "Options : \n";
			foreach($attr->_params['options'] as $value => $text) { 
				$desc .= $value.' => '.$text."\n";
			}
			if (!SQ_PHP_CLI) $desc .= '</pre>';
		}

		if (SQ_PHP_CLI) {
			echo "\t", $var_name, "\t" , $info['type'], "\t", $desc, "\n";
		
		} else {
		?>
			<tr>
				<td><?php echo $var_name; ?></th>
				<td><?php echo $info['type']; ?></th>
				<td><?php echo $desc; ?></th>
			</tr>
		<?php
		}// endif

	}// end foreach

	if (SQ_PHP_CLI) {
		echo str_repeat('-', 80), "\n";
	
	} else {
	?>
		</table>
		<br>
	<?php
	}// endif

	unset($edit_fns);
	unset($da);

}// end foreach

if (!SQ_PHP_CLI) {
?>
</body>
</html>
<?php
}// endif


?>