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
* $Id: get_design_area_setable_attrs.php,v 1.8 2006/12/06 05:39:51 bcaldwell Exp $
*
*/

/**
* Small script to return the design areas setable attributes and their descriptions
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.8 $
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


require_once $SYSTEM_ROOT.'/core/include/init.inc';

$am = &$GLOBALS['SQ_SYSTEM']->am;

$design_area_types = $am->getTypeDescendants('design_area');
$design_types = $am->getTypeDescendants('design');
$design_types[] = 'design';

// remove the 'design' asset type and any decendants
$all_design_area_types = array_diff($design_area_types, $design_types);

$design_areas = Array();
// now remove all design areas that are not instantiable
foreach ($all_design_area_types as $type_code) {
	#pre_echo($type_code.' : '.$am->getTypeInfo($type_code, 'instantiable'));
	if ($am->getTypeInfo($type_code, 'instantiable')) $design_areas[] = $type_code;
}

sort($design_areas);


if (!SQ_PHP_CLI) {
	?>
	<html>
	<title>Settable Attributes for Instantiable Design Areas</title>
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
}

foreach ($design_areas as $type_code) {

	$am->includeAsset($type_code);
	$da = new $type_code();

	$setable_vars = $da->vars;
	$protected_vars = $da->getProtectedAttrs();

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
	}

	ksort($setable_vars);

	foreach ($setable_vars as $var_name => $info) {
		if (in_array($var_name, $protected_vars) || $info['type']=='serialise') continue;
		$attr = $da->getAttribute($var_name);
		$desc = $attr->description;

		if ($info['type'] == 'selection') {
			if (!SQ_PHP_CLI) $desc .= '<pre>';
			else $desc .= "\n";
			$desc .= "\t\tOptions : \n";
			foreach ($attr->_params['options'] as $value => $text) {
				$desc .= "\t\t\t".$value.' => '.$text."\n";
			}
			if (!SQ_PHP_CLI) $desc .= '</pre>';
		}

		if (SQ_PHP_CLI) {
			echo "\t-> ", $var_name, "\t" , $info['type'], "\t", $desc, "\n";
		} else {
			?>
				<tr>
					<td><?php echo $var_name; ?></th>
					<td><?php echo $info['type']; ?></th>
					<td><?php echo $desc; ?></th>
				</tr>
			<?php
		}

	}//end foreach

	if (SQ_PHP_CLI) {
		echo str_repeat('-', 80), "\n\n";
	} else {
		?>
			</table>
			<br>
		<?php
	}

	unset($edit_fns);
	unset($da);

}//end foreach

if (!SQ_PHP_CLI) {
	?>
	</body>
	</html>
	<?php
}

?>
