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
* $Id: header.php,v 1.13 2013/03/08 02:19:51 ewang Exp $
*
*/

/**
* Header Pop-Up
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
?>
<!DOCTYPE html>
<!--[if lt IE 8 ]> 	<html class="ie">   				<![endif]-->
<!--[if IE 8 ]>     <html class="ie ie8"> 		        <![endif]-->
<!--[if IE 9 ]>     <html class="ie ie9 gtie8"> 		<![endif]-->
<!--[if IE 10 ]>    <html class="ie ie-10 gtie8 gtie9"> <![endif]-->
<!--[if !IE]><!--> 	<html class="">                     <!--<![endif]-->
<head>
	<meta charset="utf-8"/>
	<meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1"/>
	<title>Squiz Matrix - Edit Bodycopy</title>
	<link rel="stylesheet" type="text/css" href="<?php echo sq_web_path('lib')?>/web/css/edit.css" />

	<?php

	if ($_GET['browser'] != 'ns') {
		?><script language="JavaScript" src="<?php echo sq_web_path('lib').'/js/detect.js';?>"></script><?php
	}
	?>

	<script>
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
	if ($_GET['page_width']) {
		$div_width  = 'width: '.  preg_replace('/[^0-9]+/', '', $_GET['page_width']).'px; ';
	}

	if ($_GET['page_height']) {
		$div_height = 'height: '.preg_replace('/[^0-9]+/', '', $_GET['page_height']).'px; ';
	}
?>

<body onload="javascript: if(typeof popup_init == 'function') popup_init();" >

<div class="sq-bodycopy-popup-wrapper">
