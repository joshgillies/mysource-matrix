<?php
/**
* Header Pop-Up
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
?>
<html>
<head>
<style type="text/css">
	body { 
		background-color: #212E61;
	}
	td { 
		font-family: Arial, Verdana, Sans-Serif; 
		font-size: 12px; 
	}
	.bodycopy-popup-heading {
		font-family: Arial, Verdana, Sans-Serif; 
		font-size: 12px; 
		font-weight: bold;
	}
	.bodycopy-popup-table { 
		background-color: #C0C0C0;
	}
	.smallprint {
		font-family: Arial, Verdana, Sans-Serif; 
		font-size: 10px; 
	}
	.warning {
		font-family: Arial, Verdana, Sans-Serif; 
		font-size: 11px; 
		color: #ff0000;
	}
</style>
<?php

$bgcolor = "#212E61";

if ($_GET['browser'] != "ns") {
	?><script language="JavaScript" src="<?php echo sq_web_path('lib').'/js/detect.js';?>"></script><?php
}
?>

<script language="JavaScript">
	if (is_ie4up || is_dom) {
		var owner = parent;
	} else {
		var owner = window;
	}// end if

	function popup_close() {
		popup_init = null;
		owner.bodycopy_hide_popup();
	}
</script>
</head>

<?php
	if ($_GET['page_width'])  $table_width  = "width='".$_GET['page_width']."'";
	if ($_GET['page_height']) $table_height = "height='".$_GET['page_height']."'";
?>

<body topmargin="0" leftmargin="0" marginheight="0" marginwidth="0" onload="javascript: if(typeof popup_init == 'function') popup_init();" <?php echo $_GET['body_extra']?>>
<table <?php echo $table_width?> <?php echo $table_height?> border="1">
	<tr>
		<td valign="top" align="center" class="bodycopy-popup-table">